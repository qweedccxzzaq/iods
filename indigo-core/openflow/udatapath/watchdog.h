/*
 * watchdog.h
 *
 * Watchdog header file
 *
 * Based on Linux watchdog samples
 *
 * This code is typically only included in one file
 * and the WATCHDOG_STROKE is called in the main event
 * loop.
 */

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_ 1

#if defined(WATCHDOG_SUPPORT)

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define WATCHDOG_LONG_OPTIONS  {"watchdog", no_argument, 0, 'w'},
#define WATCHDOG_START if (watchdog_active) watchdog_open()
#define WATCHDOG_STOP if (watchdog_active) watchdog_close()
#define WATCHDOG_STROKE if (watchdog_active) watchdog_stroke()
#define WATCHDOG_USAGE "  -w, --watchdog          enable SW watchdog\n"
#define WATCHDOG_STATE  printf("Watchdog is %sactive (fd %d)\n", \
                               watchdog_active ? "" : "not ", wd_fd)

extern int watchdog_active;
extern int wd_fd;

static inline void
watchdog_open(void)
{
    printf("Starting software watchdog\n");
    if ((wd_fd = open("/dev/watchdog", O_WRONLY)) == -1) {
        printf("Error opening /dev/watchdog\n");
        wd_fd = -1;
    }
}

static inline void
watchdog_stroke(void)
{
#if 0 /* Code to monitor max delay between strokes */
    static uint32_t max_diff;
    static uint32_t last_time;
    uint32_t diff;

    if (last_time != 0) {
        uint32_t cur_time;

        cur_time = time_msec();
        diff = cur_time - last_time;
        last_time = cur_time;
        if (diff > max_diff) {
            max_diff = diff;
            printf("New watchdog max diff %d\n", max_diff);
        }
    }
#endif

    if (wd_fd != -1) {
        write(wd_fd, "\0", 1);
        fsync(wd_fd);
    }
}

static inline void
watchdog_close(void)
{
    if (wd_fd != -1) {
        printf("Stopping software watchdog\n");
        close(wd_fd);
    }
}

#else /* No watchdog support */

#define WATCHDOG_LONG_OPTIONS
#define WATCHDOG_STROKE
#define WATCHDOG_USAGE
#define WATCHDOG_START
#define WATCHDOG_STOP
#define WATCHDOG_STATE printf("Watchdog not enabled\n")

#endif

#endif /* _WATCHDOG_H_ */
