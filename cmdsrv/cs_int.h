/*
 * TBD:  Need standard header
 * Copyright 2010 Big Switch Networks
 */

#if !defined(_CS_INT_H_)
#define _CS_INT_H_

#include <stdio.h>
#include <netinet/in.h>

#include "rest.h"

#if defined(CS_TEST_ENV)
#include <udatapath/switch-flow.h>
#endif

#define CMDSRV_PORT 8088

#if !defined(false)
#define true 1
#define false 0
#endif

extern int cs_debug_level;
extern FILE *dbg_fd;
extern struct datapath *dp_hook;

#define dbg_send(mod, lvl, fmt, args...) do {                   \
        if ((dbg_fd != NULL) && (cs_debug_level >= (lvl))) {    \
            fprintf(dbg_fd, fmt, ##args);                       \
            fflush(dbg_fd);                                     \
        }                                                       \
    } while (0)
#if defined(CS_TEST_ENV)

#define IS_7352 true
#define _IS_ODD(_p) ((_p) & 1)

static inline int
of_port_to_bcm_unit(int of_port)
{
    if (IS_7352) {
        if (of_port <= 24) {
            return 0;
        }
        if (of_port <= 48) {
            return 1;
        }
        /* FIXME:  Need to verify port connectivity to front panel */
        if (of_port <= 52) {
            return 0;
        }
    } else {
        if (of_port <= 26) {
            return 0;
        }
    }

    return -1;
}

static inline int
of_port_to_bcm_port(int of_port)
{
    if (of_port <= 24) {
        if (_IS_ODD(of_port)) {
            return of_port;
        } else {
            return of_port - 2;
        }
    }

    if (IS_7352) {
        if (of_port <= 48) {
            of_port -= 24;
            if (_IS_ODD(of_port)) {
                return of_port;
            } else {
                return of_port - 2;
            }
        } else if (of_port <= 50) {
            /* FIXME:  Need to verify port connectivity to front panel */
            /* This is a guess */
            return of_port - 25;
        }
    } else {
        /* FIXME:  Need to verify port connectivity to front panel */
        /* This is a guess */
        if (of_port <= 26) {
            return of_port - 1;
        }
    }

    return -1;
}
#define MAP_TO_UNIT(drv, of_port) of_port_to_bcm_unit(of_port)
#define MAP_TO_PORT(drv, of_port) of_port_to_bcm_port(of_port)
#else
#define MAP_TO_UNIT(drv, of_port) OF_PORT_TO_BCM_UNIT(drv, of_port)
#define MAP_TO_PORT(drv, of_port) OF_PORT_TO_BCM_PORT(drv, of_port)
#endif        

#define DBG_LVL_NONE      -1 /* All output off */
#define DBG_LVL_ERROR     0  /* Default value */
#define DBG_LVL_ALWAYS    0  /* For requested dump output */
#define DBG_LVL_WARN      1
#define DBG_LVL_INFO      2
#define DBG_LVL_VERBOSE   3
#define DBG_LVL_VVERB     4  /* Include success indications */

#define DBG_ERROR(fmt, args...)   dbg_send(0, DBG_LVL_ERROR, fmt, ##args)
#define DBG_ALWAYS(fmt, args...)  dbg_send(0, DBG_LVL_ALWAYS, fmt, ##args)
#define DBG_WARN(fmt, args...)    dbg_send(0, DBG_LVL_WARN, fmt, ##args)
#define DBG_INFO(fmt, args...)    dbg_send(0, DBG_LVL_INFO, fmt, ##args)
#define DBG_VERBOSE(fmt, args...) dbg_send(0, DBG_LVL_VERBOSE, fmt, ##args)
#define DBG_VVERB(fmt, args...)   dbg_send(0, DBG_LVL_VVERB, fmt, ##args)
#define DBG_NONE(fmt, args...)
/* Same as DEBUG_ALWAYS */
#define DEBUGK(fmt, args...)      dbg_send(0, DBG_LVL_ALWAYS, fmt, ##args)

#define TRY(op, str) do {                                       \
        int rv;                                                 \
        if (((rv = (op)) < 0)) {                                \
            DBG_ERROR("ERROR %d: %s\n", rv, str);               \
            return rv;                                          \
        } else {                                                \
            DBG_VVERB("%s: success\n", str);                    \
        }                                                       \
    } while (0)

/* Specify error code for return if not success */
#define TRY_RV(op, str, err_ret) do {                            \
        int _rv;                                                 \
        if (((_rv = (op)) < 0)) {                                \
            DBG_ERROR("ERROR %d: %s\n", _rv, str);               \
            return err_ret;                                      \
        } else {                                                 \
            DBG_VVERB("%s: success\n", str);                     \
        }                                                        \
    } while (0)

#define CS_BUF_SIZE 1024 * 32 /* Max size transaction request/response */


#if defined(CS_TEST_ENV)

static struct ofp_action_output act_out = {OFPAT_OUTPUT, 8, 17, 0};
static struct ofp_action_vlan_vid act_vid = {OFPAT_SET_VLAN_VID, 8, 99};
static unsigned char act_bytes[128];

static inline void
populate_flows(struct sw_flow *flows, int count)
{
    int i;

    memcpy(act_bytes, &act_vid, sizeof(act_vid));
    memcpy(&act_bytes[8], &act_out, sizeof(act_out));
    for (i = 0; i < count; i++) {
        flows[i].key.flow.nw_src = i;
        flows[i].key.flow.in_port = i;
        flows[i].key.flow.nw_dst = i * 16;
        flows[i].key.flow.dl_vlan = i + 1024;
        flows[i].key.wildcards = i;
        flows[i].cookie = i;
        flows[i].priority = i + 16;
        flows[i].idle_timeout = i + 32;
        flows[i].hard_timeout = i + 48;
        flows[i].used = i + 64;
        flows[i].created = i + 80;
        flows[i].packet_count = i + 96;
        flows[i].byte_count = i + 102;
        /* To do:  Populate some actions */
        if (flows[i].sf_acts != NULL) {
            flows[i].sf_acts->actions_len = 16;
            memcpy(&flows[i].sf_acts->actions, act_bytes, 16);
        }
    }
}
#endif

#endif




/* ? */
#if 0
typedef enum cs_json_logging_level_e {
    CS_JSON_LOGGING_LEVEL_VERBOSE,
    CS_JSON_LOGGING_LEVEL_INFO,
    CS_JSON_LOGGING_LEVEL_WARN,
    CS_JSON_LOGGING_LEVEL_ERROR,
    CS_JSON_LOGGING_LEVEL_CRITICAL,
    CS_JSON_LOGGING_LEVEL_NONE
} cs_json_logging_level_t;

#define cs_json_logging_level_str_init {      \
        "VERBOSE",                            \
        "INFO",                               \
        "WARN",                               \
        "ERROR",                              \
        "CRITICAL",                           \
        "NONE"                                \
}
extern char *json_logging_level_str[];
#endif

