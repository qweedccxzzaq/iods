/* Copyright (c) 2008 The Board of Trustees of The Leland Stanford
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
 */

/* Interface exported by OpenFlow module. */

#ifndef DATAPATH_H
#define DATAPATH_H 1

#include <stdbool.h>
#include <stdint.h>
#include "openflow/nicira-ext.h"
#include "ofpbuf.h"
#include "timeval.h"
#include "list.h"
#include "netdev.h"
#include "packets.h"

#if defined(OF_HW_PLAT)
#include <of_hw_api.h>  // FIXME:  Can declare struct of_hw_driver instead
extern int max_rx_queue;
#endif

struct rconn;
struct pvconn;
struct sw_flow;
struct sender;

struct sw_queue {
    struct list node; /* element in port.queues */
    unsigned long long int tx_packets;
    unsigned long long int tx_bytes;
    unsigned long long int tx_errors;
    uint32_t queue_id;
    uint16_t class_id; /* internal mapping from OF queue_id to tc class_id */
    struct sw_port *port; /* reference to the parent port */
    /* keep it simple for now, only one property (assuming min_rate) */
    uint16_t property; /* one from OFPQT_ */
    uint16_t min_rate;
};

#define MAX_HW_NAME_LEN 32
enum sw_port_flags {
    SWP_USED             = 1 << 0,    /* Is port being used */
    SWP_HW_DRV_PORT      = 1 << 1,    /* Port controlled by HW driver */
};
#if defined(OF_HW_PLAT)
#define IS_HW_PORT(p) ((p)->flags & SWP_HW_DRV_PORT)
#define REMOTE_RUN_LIMIT 25
#define TIMEOUT_PROC_MAX 3
#define DP_EXPIRE_ENABLED(dp) ((dp)->expire_enabled)
#else
#define IS_HW_PORT(p) 0
#define REMOTE_RUN_LIMIT 50
#define TIMEOUT_PROC_MAX 0
#define DP_EXPIRE_ENABLED(dp) (true)
#endif

#define PORT_IN_USE(p) (((p) != NULL) && (p)->flags & SWP_USED)

/* Enable/disable state definitions for port_add */
#define PORT_ADD_STATE_UP 1
#define PORT_ADD_STATE_DOWN 0

struct sw_port {
    uint32_t config;            /* Some subset of OFPPC_* flags. */
    uint32_t state;             /* Some subset of OFPPS_* flags. */
    uint32_t flags;             /* SWP_* flags above */
    struct datapath *dp;
    struct netdev *netdev;
    char hw_name[OFP_MAX_PORT_NAME_LEN];
    uint8_t eth_addr[ETH_ADDR_LEN];
    struct list node; /* Element in datapath.ports. */
    unsigned long long int rx_packets, tx_packets;
    unsigned long long int rx_bytes, tx_bytes;
    unsigned long long int tx_dropped;
    uint16_t port_no;
    /* port queues */
    uint16_t num_queues;
    struct sw_queue queues[NETDEV_MAX_QUEUES];
    struct list queue_list; /* list of all queues for this port */

    /* Currently hardware specific info, but useful in general */
    uint32_t state_changes;
    int last_state;
};

#if defined(OF_HW_PLAT)
struct hw_pkt_q_entry {
    struct ofpbuf *buffer;
    struct hw_pkt_q_entry *next;
    of_port_t port_no;
    int reason;
};
extern int dp_mgmt_set(struct datapath *dp);
extern int dp_port_add_state_set(struct datapath *dp, int state);
extern uint32_t mgmt_pkt_out, mgmt_pkt_in, non_mgmt_pkt_drop;
extern void datapath_expire_set(struct datapath *dp, int enabled);
#endif

#define DP_MAX_PORTS 255
BUILD_ASSERT_DECL(DP_MAX_PORTS <= OFPP_MAX);

extern char * dp_mgmt_filter_mode_strings[];

struct datapath {
    /* Remote connections. */
    struct list remotes;        /* All connections (including controller). */

    /* Listeners. */
    struct pvconn **listeners;
    size_t n_listeners;

    time_t last_timeout;

    /* Unique identifier for this datapath */
    uint64_t  id;
    char dp_desc[DESC_STR_LEN];	/* human readible comment to ID this DP */

    struct sw_chain *chain;  /* Forwarding rules. */

    /* Configuration set from controller. */
    uint16_t flags;
    uint16_t miss_send_len;

    /* Switch ports. */
    struct sw_port ports[DP_MAX_PORTS];
    struct sw_port *local_port;  /* OFPP_LOCAL port, if any. */
    struct list port_list; /* All ports, including local_port. */

#if defined(OF_HW_PLAT)
    of_dp_mgmt_t dp_mgmt; /* controlling structure for dp_mgmt */

    /* Although the chain maintains the pointer to the HW driver
     * for flow operations, the datapath needs the port functions
     * in the driver structure
     */
    of_hw_driver_t *hw_drv;
    struct hw_pkt_q_entry *hw_pkt_list_head, *hw_pkt_list_tail;

    /* Set ports to this state when added to datapath */
    int add_port_state;

    /* Delay RX process until things are really 'up' */
    int rx_enabled; 

    /* Allow table expirations to be disabed */
    int expire_enabled; /* FIXME:  Shouldn't depend on OF_HW_PLAT */
#endif
};

int dp_new(struct datapath **, uint64_t dpid);
int dp_add_port(struct datapath *, const char *netdev, uint16_t);
int dp_add_local_port(struct datapath *, const char *netdev, uint16_t);
void dp_add_pvconn(struct datapath *, struct pvconn *);
void dp_run(struct datapath *);
void dp_wait(struct datapath *);
void dp_send_error_msg(struct datapath *, const struct sender *,
                  uint16_t, uint16_t, const void *, size_t);
void dp_send_flow_end(struct datapath *, struct sw_flow *,
                      enum ofp_flow_removed_reason);
void dp_output_port(struct datapath *, struct ofpbuf *, int in_port, 
                    int out_port, uint32_t queue_id, bool ignore_no_fwd);
void dp_output_control(struct datapath *, struct ofpbuf *, int in_port,
        size_t max_len, int reason);
struct sw_port * dp_lookup_port(struct datapath *, uint16_t);
struct sw_queue * dp_lookup_queue(struct sw_port *, uint32_t);

extern void datapath_status(struct datapath *dp);
extern int datapath_status_string(struct datapath *dp, char *buf, int bytes);

extern int udatapath_cmd(int argc, char *argv[]);
extern void udatapath_status(void);
extern int udatapath_status_string(char *buf, int bytes);

extern struct datapath *of_datapath_get(void);

#endif /* datapath.h */
