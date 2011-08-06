/*
 * TBD:  Need standard header
 * Copyright 2010 Big Switch Networks
 *
 * rest.h
 * Header file for REST encap related functionality
 */

#if !defined(_CS_REST_H_)
#define _CS_REST_H_

#include <stdint.h>

/* FIXME:  This should be structured better */

/* These lists should have the same operation and URI entries, 
 * although the order is not important. */

/* The REST version needs the names, not the functions */
#define REST_REQUEST_LIST                           \
    {"GET", "/cs/1.0/echo", NULL},                  \
    {"GET", "/cs/1.0/info", NULL},                  \
    {"PUT", "/cs/1.0/port", NULL},                  \
    {"GET", "/cs/1.0/port", NULL},                  \
    {"GET", "/cs/1.0/portstats", NULL},             \
    {"GET", "/cs/1.0/flowtableentry", NULL},        \
    {"GET", "/cs/1.0/flowtable", NULL},             \
    {"PUT", "/cs/1.0/dpexpire", NULL},              \
    {"GET", "/cs/1.0/mgmtstats", NULL},             \
    {"GET", "/cs/1.0/controllerstatus", NULL},      \
    {"PUT", "/cs/1.0/loglevel", NULL},              \
    {"GET", "/cs/1.0/loglevel", NULL},              \
    {"PUT", "/cs/1.0/failresponse/open", NULL},     \
    {"PUT", "/cs/1.0/failresponse/close", NULL},    \
    {"GET", "/cs/1.0/failresponse", NULL},          \
    {"GET", "/cs/1.0/drvcmd", NULL}

/* The command server needs to know the handlers to use */
#define CMDSRV_REQUEST_LIST                                         \
    {"GET", "/cs/1.0/echo", handle_echo_request},                   \
    {"GET", "/cs/1.0/info", handle_info_request},                   \
    {"PUT", "/cs/1.0/port", handle_port_config_set},                \
    {"GET", "/cs/1.0/port", handle_port_config_get},                \
    {"GET", "/cs/1.0/portstats", handle_port_statistics_get},       \
    {"GET", "/cs/1.0/flowtableentry", handle_flow_table_entry_get}, \
    {"GET", "/cs/1.0/flowtable", handle_flow_table_get},            \
    {"PUT", "/cs/1.0/dpexpire", handle_datapath_expire_set},        \
    {"GET", "/cs/1.0/mgmtstats", handle_mgmt_stats_get},            \
    {"GET", "/cs/1.0/controllerstatus", handle_unsupported_op},     \
    {"PUT", "/cs/1.0/loglevel", handle_log_level_put},              \
    {"GET", "/cs/1.0/loglevel", handle_unsupported_op},             \
    {"PUT", "/cs/1.0/failresponse/open", handle_unsupported_op},    \
    {"PUT", "/cs/1.0/failresponse/close", handle_unsupported_op},   \
    {"GET", "/cs/1.0/failresponse", handle_unsupported_op},         \
    {"GET", "/cs/1.0/drvcmd", handle_drvcmd_request}

#if defined(CS_TEST_ENV)
#define true 1
#define false 0
#endif

#define REST_OP_LEN 4         /* Length of operation string PUT... */
#define REST_URI_MAX 64       /* Max supported URI length after prefix */
#define REST_HEADER_MAX 128   /* Check this */

#define CS_URI_PREFIX "/cs/1.0/"
#define CS_URI_PREFIX_LEN (strlen(CS_URI_PREFIX))

typedef enum cs_rest_status_e {
    CS_REST_MORE_DATA = -1,            /* Internal error, need more data */
    CS_REST_OKAY = 200,
    CS_REST_BAD_REQUEST = 400,
    CS_REST_NOT_FOUND = 404,
    CS_REST_NOT_ALLOWED = 405,
    CS_REST_SERVER_ERROR = 500,
    CS_REST_NOT_IMPLEMENTED = 502
} cs_rest_status_t;

/* Definition of cs_rest_header_t is forwarded in cmdsrv.h */
typedef struct cs_rest_header_s {
    int sd;                            /* File descriptor of socket */
    char op[REST_OP_LEN];              /* GET/PUT/POST(PST)/DELETE(DEL) */
    char uri[REST_URI_MAX];            /* URI for operation */
    uint32_t transaction_id;             /* 0 is not valid */
    cs_rest_status_t status_code;      /* For responses */
    uint32_t json_length;                /* Length of JSON data */
    int header_length;                 /* When packed into packet */

    /* Wire and parse representations */
    uint8_t packed_header[REST_HEADER_MAX];   /* Packed header */
} cs_rest_header_t;

/* The request handle map is an array mapping operation and URI strings
 * to function handlers.  The map can be used to iterate over all known
 * op/uri requests.  The array of handlers must be declared with
 * external visibility in some file including the rest module.
 */
typedef int (*request_handler_t)(cs_rest_header_t *hdr, uint8_t *rd_buf);
typedef struct request_map_s {
    char *op;
    char *uri;
    request_handler_t handler;
} request_map_t;

extern int cs_rest_header_create(cs_rest_header_t *hdr, char *op, char *uri,
                                 uint32_t trx_id, uint32_t status_code, 
                                 int json_length);

extern int cs_rest_header_pack(cs_rest_header_t *hdr, 
                               uint32_t status_code, int json_length);

extern cs_rest_status_t cs_rest_header_parse(uint8_t *buffer, int len,
                                             cs_rest_header_t *hdr,
                                             char **error_str);

extern int cs_packet_create(uint8_t *buf, int len, cs_rest_header_t *hdr,
                            uint8_t *json_data, int json_len);

#endif
