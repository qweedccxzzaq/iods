/*
 * Simple profiling
 *
 * Very simple profiling code that works by marking a section
 * of code in the following manner:
 *
 * SPROF_IN(SPROF_FOO_PROCESSING)
 * code_running_foo()
 * SPROF_OUT(SPROF_FOO_PROCESSING)
 *
 * Then SPROF_DUMP() will show the results in a table giving total
 * time in the code, number of times the code is timed, average and
 * max times.
 */


#ifndef _SPORF_H
#define _SPROF_H 1

#if defined(SPROF_SUPPORT)
#include <sys/time.h>

typedef struct timeval sprof_time_t;  /* A point in time */
typedef uint32_t sprof_acc_time_t; /* Accumulated time */
typedef uint32_t sprof_delta_t; /* Difference in time */

typedef struct sprof_block_s {
    struct timeval last_time_in;
    uint32_t tot_time;
    uint32_t max_dur;  /* Max seen time for this event */
    uint32_t count;
} sprof_block_t;

/* Add your own SPROF identifiers here and add descriptive strings below */
typedef enum sprof_points_e {
    SPROF_DP_RUN,
    SPROF_DP_WAIT,
    SPROF_PBLOCK,
    SPROF_FLOW_EXP,
    SPROF_FLOW_REMOVING,
    SPROF_CHAIN_TIMEOUT,
    SPROF_HW_PKT_DEQUEUE,
    SPROF_SW_PKT_RECV,
    SPROF_LIN_TAB_TO,
    SPROF_HASH_TAB_TO,
    SPROF_HW_TAB_TO,
    SPROF_HW_STAT_UPD,
    SPROF_HW_FLOW_RM,
    SPROF_HW_ENT_RM,
    SPROF_HW_FLOW_INST,
    SPROF_HW_ENTRY_INST,
    SPROF_REMOTE_RUN,
    SPROF_RCONN_RUN,
    SPROF_RCONN_PROC,
    SPROF_POINTS_MAX
} sprof_points_t;

/* Order of these strings MUST match the enum above */
#define SPROF_NAMES_INIT {                                     \
        "dp_run",               /* SPROF_DP_RUN          */    \
        "dp_wait",              /* SPROF_DP_WAIT         */    \
        "poll_block",           /* SPROF_PBLOCK          */    \
        "flow_exp",             /* SPROF_FLOW_EXP        */    \
        "flow_rmving",          /* SPROF_FLOW_REMOVING   */    \
        "chain_timeout",        /* SPROF_CHAIN_TIMEOUT   */    \
        "hw_pkt_dequeue",       /* SPROF_HW_PKT_DEQUEUE  */    \
        "sw_pkt_recv",          /* SPROF_SW_PKT_RECV     */    \
        "lin_tab_to",           /* SPROF_LIN_TAB_TO      */    \
        "hash_tab_to",          /* SPROF_HASH_TAB_TO     */    \
        "hw_tab_to",            /* SPROF_HW_TAB_TO       */    \
        "hw_stat_upd",          /* SPROF_HW_STAT_UPD     */    \
        "hw_flow_rm",           /* SPROF_HW_FLOW_RM      */    \
        "hw_ent_rm",            /* SPROF_HW_ENT_RM       */    \
        "hw_flow_inst",         /* SPROF_HW_FLOW_INST    */    \
        "hw_entry_inst",        /* SPROF_HW_ENTRY_INST   */    \
        "remote_run",           /* SPROF_REMOTE_RUN      */    \
        "rconn_run",            /* SPROF_RCONN_RUN       */    \
        "rconn_proc"            /* SPROF_RCONN_PROC      */    \
        ""                      /* SPROF_POINTS_MAX      */    \
}

/* The max number of profile points to monitor */
extern sprof_block_t sprof_blocks[SPROF_POINTS_MAX];
extern char *sprof_names[SPROF_POINTS_MAX];

/* If hook is set and diff is big, hook is called */
extern void (*sprof_hook_f)(void);

/* Bracket in/out profile markers */
#define SPROF_IN(idx) gettimeofday(&sprof_blocks[idx].last_time_in, NULL)
#define SPROF_OUT(idx) {                                                \
    struct timeval cur;                                                 \
    uint32_t diff;                                                      \
    gettimeofday(&cur, NULL);                                           \
    diff = (cur.tv_usec - sprof_blocks[idx].last_time_in.tv_usec) +     \
        (cur.tv_sec - sprof_blocks[idx].last_time_in.tv_sec) * 1000000; \
    if (diff > sprof_blocks[idx].max_dur) sprof_blocks[idx].max_dur = diff; \
    sprof_blocks[idx].tot_time += diff;                                 \
    sprof_blocks[idx].count++;                                          \
    if ((sprof_hook_f != NULL) && (diff > 5000000)) sprof_hook_f();     \
}

/* Dump the table entries between start and end; -1 for no limit */
static inline void
SPROF_DUMP(void) {
    int idx;
    int seen = 0;

    for (idx = 0; idx < SPROF_POINTS_MAX; idx++) {
        if (sprof_blocks[idx].count > 0) {
            if (!seen) {
                printf("Idx: %-20s %8s %10s %10s %10s\n", "Name",
                       "Tot ms", "Count", "Ave us", "Max us");
                seen = 1;
            }
            printf("%3d: %-20s %10d %8d  %10d %10d\n",
                   idx,
                   sprof_names[idx],
                   sprof_blocks[idx].tot_time / 1000,
                   sprof_blocks[idx].count,
                   sprof_blocks[idx].tot_time / sprof_blocks[idx].count,
                   sprof_blocks[idx].max_dur);
        }
    }
    if (!seen) {
        printf("No profiling data\n");
    }
}
#else /* No SPROF support */

#define SPROF_IN(idx)
#define SPROF_OUT(idx)
#define SPROF_DUMP()

#endif

#endif /* _SPROF_H_ */
