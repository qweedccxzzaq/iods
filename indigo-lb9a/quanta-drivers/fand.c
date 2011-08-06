/*
 * Copyright 2011 Big Switch Networks
 *
 * Fan Daemon for LB9A
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "i2cLib/i2cApi.h"

/* Comment or uncomment to set debug level */

#define DBG_NONE(fmt, args...)
#define DBG_ALWAYS(fmt, args...)  fprintf(stderr, fmt, ##args)

#define DBG_ERROR(fmt, args...)   DBG_ALWAYS(fmt, ##args)
#define DBG_WARN(fmt, args...)    DBG_ALWAYS(fmt, ##args)
#define DBG_INFO(fmt, args...)    DBG_ALWAYS(fmt, ##args)
#define DBG_VERBOSE(fmt, args...) DBG_ALWAYS(fmt, ##args)

#define DEFAULT_SECS 60
#define MIN_SECS 1
#define MAX_SECS 300

#define SETFANCMD "setfan"

static int highest_max = -1;
static int last_setting = -1;

/*
 * Set all fans to given percent value
 */
static void
set_fans(int fan_setting)
{
    int i;

    for (i = 1; i < 4; i++) {
        DBG_INFO("fand: Fan %d to %d percent.\n", i, fan_setting);
        if (adt7470_set_fan(i, adt_7470_sensor_1, fan_setting) != 0) {
            DBG_WARN("fand: Cannot set Fan %d to %d\n", i, fan_setting);
        }
    }
}

/*
 * Check temps and set fan speeds if changed
 */

static int
fan_speed_check(void)
{
    int max_temp = 0;
    int i;
    unsigned int temp = 0;
    int fan_setting = 100;

    if (adt7470_data_refresh() != 0) {
        DBG_WARN("fand: ADT7470 Refresh FAIL\n");
        return -1;
    }

    /* From Quanta diag code; Get temperature */
    for (i = 1; i < 5; i++) {
        if (0 != adt7470_get_temp(i, adt_7470_sensor_1, &temp)) {
            DBG_WARN("fand: Cannot get temp %d\n", i);
            continue;
        }
        if (temp > max_temp) {
            max_temp = temp;
        }
    }

    DBG_VERBOSE("fand: temp: %d\n", max_temp);
    if (max_temp > highest_max) {
        DBG_INFO("fand: New max temp: %d\n", max_temp);
        highest_max = max_temp;
    }

    /* Map to fan percent setting */
    if (max_temp <= 40) fan_setting = 40;
    else if (max_temp > 40 && max_temp <= 45) fan_setting = 50;
    else if (max_temp >= 46 && max_temp <= 50) fan_setting = 60;
    else if (max_temp >= 51 && max_temp <= 53) fan_setting = 80;
    else fan_setting = 100;

    if (fan_setting != last_setting) {
        set_fans(fan_setting);
        last_setting = fan_setting;
    }
    return 0;
}

void
usage(void)
{
    printf("%s <n>   - Set all fans to n percent, 10 <= n <= 100\n",
           SETFANCMD);
    printf("fand [<n>]   - Run fan daemon checking every n seconds\n");
    printf("     %d <= n <= %d\n", MIN_SECS, MAX_SECS);
    printf("     Runs in foreground unless backgrounded\n");
    printf("     Normal use is 'fand 20 > /local/logs/fand.log 2>&1\n");
}

/*
 * Simple fan control daemon
 *
 * fand <n>
 *   Check the temperature every n seconds
 */

int
main(int argc, char **argv)
{
    int secs = DEFAULT_SECS;
    int percent = 100;

    if (!strncmp(argv[0], SETFANCMD, sizeof(SETFANCMD))) {
        percent = strtoul(argv[1], NULL, 10);
        if ((percent < 10) || (percent > 100)) {
            usage();
            return 1;
        }
        DBG_INFO("Setting fans to %d percent\n", percent);
        set_fans(percent);
        return 0;
    }

    if (argc > 1) {
        if (!strncmp(argv[1], "help", sizeof("help"))
            || (!strncmp(argv[1], "-h", sizeof("-h")))) {
            usage();
            return 0;
        }
        secs = strtoul(argv[1], NULL, 10);
        if ((secs < MIN_SECS) || (secs > MAX_SECS)) {
            secs = DEFAULT_SECS;
            DBG_WARN("fand: Error in parameter %s. Using default\n", argv[1]);
        }
    }
    DBG_INFO("fand: Running fand. Checking temp every %d seconds\n", secs);

    while (1) {
        fan_speed_check();
        sleep(secs);
    }

    /* Won't reach */
    set_fans(100);
}
