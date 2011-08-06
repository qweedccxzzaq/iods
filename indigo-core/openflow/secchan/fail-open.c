/* Copyright (c) 2008, 2009 The Board of Trustees of The Leland Stanford
 * Junior University
 *
 * We are making the OpenFlow specification and associated documentation
 * (Software) available for public use and benefit with the expectation
 * that others will use, modify and enhance the Software and contribute
 * those enhancements back to the community. However, since we would
 * like to make the Software available for broadest use, with as few
 * restrictions as possible permission is hereby granted, free of
 * charge, to any person obtaining a copy of this Software to deal in
 * the Software under the copyrights without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The name and trademarks of copyright holder(s) may NOT be used in
 * advertising or publicity pertaining to the Software or any
 * derivatives without specific, written prior permission.
 *
 * Copyright (c) 2011 Big Switch Networks
 * Code has been added to support two controller connection failure
 * modes.
 *
 * Fail-host:  On connection down, explicitly delete all flow
 * table entries.  There after, no new flows are inserted until a
 * controller connection is re-established.  So this is a form of
 * fail-closed.
 *
 * Fail-static:  Leave the flow table "as is" and disable flow
 * expirations.  No new flows are added to the table.  So the switch
 * continues to forward according to the state when the controller
 * connection failed.
 *
 */

#include <config.h>
#include <openflow/openflow.h>
#include "fail-open.h"
#include <arpa/inet.h>
#include <stddef.h>
#include <string.h>
#include "learning-switch.h"
#include "netdev.h"
#include "packets.h"
#include "port-watcher.h"
#include "rconn.h"
#include "secchan.h"
#include "status.h"
#include "stp-secchan.h"
#include "timeval.h"

#define THIS_MODULE VLM_fail_open
#include "vlog.h"

#if defined(OF_HW_PLAT)
#include <cmdsrv.h>
#include <cmdsrv_client.h>
#include <vconn.h>
#endif

struct fail_open_data {
    const struct settings *s;
    struct rconn *local_rconn;
    struct rconn *remote_rconn;
    struct lswitch *lswitch;
    int last_disconn_secs;
    time_t boot_deadline;
    bool in_fail_state;
};

/* Fail-host: start.  Connection to controller is down
 * Send message to local switch to delete all flow table entries
 */
static void
fail_host_start(struct fail_open_data *fail_open)
{
    struct ofpbuf *bufferp;
    struct ofp_flow_mod *ofm;
    int rv;

    ofm = make_openflow(sizeof *ofm, OFPT_FLOW_MOD, &bufferp);    
    memset(&ofm->match, 0, sizeof(ofm->match));
    ofm->match.wildcards = OFPFW_ALL;
    ofm->command = htons(OFPFC_DELETE);

    rv = rconn_send(fail_open->local_rconn, bufferp, NULL);
    if (rv != 0) {
        VLOG_WARN("fail-open: Failed to delete all flows");
    }

    return;
}

static void
fail_host_stop(struct fail_open_data *fail_open UNUSED)
{
    /* Nothing to do here */
    return;
}

/* Send message to disable expirations */
static void
fail_static_start(struct fail_open_data *fail_open UNUSED)
{
    int rv;

    rv = cmdsrv_client_datapath_expire_set(0, 0, 0);
    if (rv != 0) {
        VLOG_WARN("fail-open: dp expire disable failed");
    }
    
    return;
}


/* Send message to restart expirations */
static void
fail_static_stop(struct fail_open_data *fail_open UNUSED)
{
    int rv;

    rv = cmdsrv_client_datapath_expire_set(0, 0, 1);
    if (rv != 0) {
        VLOG_WARN("fail-open: dp expire enable failed");
    }

    return;
}

/* Handle non-open cases; host and static */
static void
handle_host_static(struct fail_open_data *fail_open, bool cxn_now_failed)
{
    if (cxn_now_failed != fail_open->in_fail_state) { /* Process state change */
        if (cxn_now_failed) {
            if (fail_open->s->fail_mode == FAIL_HOST) {
                fail_host_start(fail_open);
                VLOG_WARN("Now in fail_host mode");
            } else {
                fail_static_start(fail_open);
                VLOG_WARN("Now in fail_static mode");
            }
        } else {
            if (fail_open->s->fail_mode == FAIL_HOST) {
                fail_host_stop(fail_open);
                VLOG_WARN("No longer in fail_host mode");
            } else {
                fail_static_stop(fail_open);
                VLOG_WARN("No longer in fail_static mode");
            }
        }
        fail_open->in_fail_state = cxn_now_failed;
    }

    return;
}

/* Causes 'r' to enter or leave fail-open mode, if appropriate. */
static void
fail_open_periodic_cb(void *fail_open_)
{
    struct fail_open_data *fail_open = fail_open_;
    int disconn_secs;
    bool open;

    if (time_now() < fail_open->boot_deadline) {
        return;
    }
    disconn_secs = rconn_failure_duration(fail_open->remote_rconn);
    open = disconn_secs >= fail_open->s->probe_interval * 3;

    /* fail_host and fail_static mode handling */
    if (fail_open->s->fail_mode != FAIL_OPEN) {
        handle_host_static(fail_open, open);
        return;
    }

    /* fail_open mode handling */
    if (open != (fail_open->lswitch != NULL)) {
        if (!open) {
            VLOG_WARN("No longer in fail-open mode");
            lswitch_destroy(fail_open->lswitch);
            fail_open->lswitch = NULL;
        } else {
            VLOG_WARN("Could not connect to controller for %d seconds, "
                      "failing open", disconn_secs);
            fail_open->lswitch = lswitch_create(fail_open->local_rconn, true,
                                                fail_open->s->max_idle);
            fail_open->last_disconn_secs = disconn_secs;
        }
    } else if (open && disconn_secs > fail_open->last_disconn_secs + 60) {
        VLOG_INFO("Still in fail-open mode after %d seconds disconnected "
                  "from controller", disconn_secs);
        fail_open->last_disconn_secs = disconn_secs;
    }
    if (fail_open->lswitch) {
        lswitch_run(fail_open->lswitch, fail_open->local_rconn);
    }
}

static void
fail_open_wait_cb(void *fail_open_)
{
    struct fail_open_data *fail_open = fail_open_;
    if (fail_open->lswitch) {
        lswitch_wait(fail_open->lswitch);
    }
}

static bool
fail_open_local_packet_cb(struct relay *r, void *fail_open_)
{
    struct fail_open_data *fail_open = fail_open_;
    /* Static and host take first path as lswitch always NULL */
    if (rconn_is_connected(fail_open->remote_rconn) || !fail_open->lswitch) {
        return false;
    } else {
        lswitch_process_packet(fail_open->lswitch, fail_open->local_rconn,
                               r->halves[HALF_LOCAL].rxbuf);
        rconn_run(fail_open->local_rconn);
        return true;
    }
}

static void
fail_open_status_cb(struct status_reply *sr, void *fail_open_)
{
    struct fail_open_data *fail_open = fail_open_;
    const struct settings *s = fail_open->s;
    int trigger_duration = s->probe_interval * 3;
    int cur_duration = rconn_failure_duration(fail_open->remote_rconn);

    status_reply_put(sr, "trigger-duration=%d", trigger_duration);
    status_reply_put(sr, "current-duration=%d", cur_duration);
    status_reply_put(sr, "triggered=%s",
                     cur_duration >= trigger_duration ? "true" : "false");
    status_reply_put(sr, "max-idle=%d", s->max_idle);
}

static struct hook_class fail_open_hook_class = {
    fail_open_local_packet_cb,  /* local_packet_cb */
    NULL,                       /* remote_packet_cb */
    fail_open_periodic_cb,      /* periodic_cb */
    fail_open_wait_cb,          /* wait_cb */
    NULL,                       /* closing_cb */
};

void
fail_open_start(struct secchan *secchan, const struct settings *s,
                struct switch_status *ss,
                struct rconn *local_rconn, struct rconn *remote_rconn)
{
    struct fail_open_data *fail_open = xmalloc(sizeof *fail_open);
    fail_open->s = s;
    fail_open->local_rconn = local_rconn;
    fail_open->remote_rconn = remote_rconn;
    fail_open->lswitch = NULL;
    fail_open->boot_deadline = time_now() + s->probe_interval * 3;
    if (s->enable_stp) {
        fail_open->boot_deadline += STP_EXTRA_BOOT_TIME;
    }
    switch_status_register_category(ss, "fail-open",
                                    fail_open_status_cb, fail_open);
    add_hook(secchan, &fail_open_hook_class, fail_open);
}
