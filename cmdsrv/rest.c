/*
 * TBD:  Need standard header
 * Copyright 2010 Big Switch Networks
 *
 * rest.c
 * Implementation of REST related encode/parse routines
 */

#include "rest.h"
#include "cs_int.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* The request list is just used for request validation */
static request_map_t rest_request_list[] = {
    REST_REQUEST_LIST /* See rest.h */
};

static int rest_request_list_length =
    sizeof(rest_request_list)/sizeof(rest_request_list[0]);

#define MAX_JSON_LEN (CS_BUF_SIZE - REST_HEADER_MAX)

/* Pack the current header into its own buffer */
int
cs_rest_header_pack(cs_rest_header_t *hdr, uint32_t status_code,
                    int json_length)
{
    char *ptr;

    /* Generate the string representation of the header */
    hdr->status_code = status_code;
    hdr->json_length = json_length;
    ptr = (char *)hdr->packed_header;
    ptr += sprintf(ptr, "%s ", hdr->op);
    ptr += sprintf(ptr, "%s ", hdr->uri);
    ptr += sprintf(ptr, "%u ", hdr->transaction_id);
    ptr += sprintf(ptr, "%d ", status_code);
    ptr += sprintf(ptr, "%d ", json_length);
    hdr->header_length = (uint8_t *)ptr - hdr->packed_header;

    return 0;
}

/*
 * cs_rest_header_create
 *
 * Returns the number of bytes needed in the final packet for the header 
 * or -1 on error
 *
 * JSON length must be accurate when this is called.
 */

int
cs_rest_header_create(cs_rest_header_t *hdr, char *op, char *uri,
                      uint32_t trx_id, uint32_t status_code, int json_length)
{
    if (json_length > MAX_JSON_LEN) {
        DBG_ERROR("JSON length is too big: %d\n", json_length);
        return -1;
    }
    memset(hdr, 0, sizeof(*hdr));
    hdr->transaction_id = trx_id;
    hdr->status_code = status_code;
    strcpy(hdr->op, op);
    strcpy(hdr->uri, uri);
    hdr->json_length = json_length;

    (void)cs_rest_header_pack(hdr, status_code, json_length);

    return hdr->header_length;
}

/*
 * cs_rest_header_parse
 * Parse the header pointed to by buffer into the structure pointed to by hdr
 */
static char *error_strs[] = {
    "No error",
    "Can not PUT",
    "Can not GET",
    "Unknown URI prefix",
    "Unknown URI"
};

/* Increment pointer and check if beyond end of data */
/* Accumulates length of header including following space */
#define _CHECK_TOKEN(_t, _s, _h) do {                    \
        _t = strtok((_s), " ");                          \
        if ((_t) == NULL) return CS_REST_MORE_DATA;      \
        (_h)->header_length += strlen(_t) + 1;           \
    } while (0)

/*
 * Parse the REST header and verify that the entire message
 * has been received (based on the length in the header).
 *
 * Corner case:  short data terminates in middle of string
 * leaving what looks to be a valid identifier.
 *
 * Currently assumes a single space separates fields.
 */
cs_rest_status_t
cs_rest_header_parse(uint8_t *buffer, int len, cs_rest_header_t *hdr, 
                     char **error_str)
{
    char *dummy;
    char *token;
    int i;
    char parse_hdr[REST_HEADER_MAX];
    int found_type;

    memset(hdr, 0, sizeof(*hdr));

    /* In case error_str pointer isn't passed, give it a dummy value */
    if (error_str == NULL) {
        error_str = &dummy;
    }

    *error_str = error_strs[0];

    if (len < REST_OP_LEN + CS_URI_PREFIX_LEN) {
        return CS_REST_MORE_DATA;
    }

    strncpy(parse_hdr, (char *)buffer, REST_HEADER_MAX - 1);
    strncpy((char *)(hdr->packed_header), parse_hdr, REST_HEADER_MAX - 1);

    _CHECK_TOKEN(token, parse_hdr, hdr);    /* OP */
    strcpy(hdr->op, token);

    _CHECK_TOKEN(token, NULL, hdr);    /* URI */
    strncpy(hdr->uri, token, REST_URI_MAX);

    _CHECK_TOKEN(token, NULL, hdr);    /* Transaction ID */
    hdr->transaction_id = strtoul(token, NULL, 10);

    _CHECK_TOKEN(token, NULL, hdr);
    hdr->status_code = strtoul(token, NULL, 0);

    _CHECK_TOKEN(token, NULL, hdr);    /* JSON Length */
    hdr->json_length = strtoul(token, NULL, 10);

    found_type = false;
    DBG_VERBOSE("Looking for %s %s\n", hdr->op, hdr->uri);
    for (i = 0; i < rest_request_list_length; i++) {
        if (!strcmp(hdr->op, rest_request_list[i].op)) {
            if (!strcmp(hdr->uri, rest_request_list[i].uri)) {
                found_type = true;
                break;
            }
        }
    }

    if (!found_type) {
        *error_str = error_strs[4];
        return CS_REST_NOT_ALLOWED;
    }

    /* Make sure entire json part of packet is present */
    if (len < hdr->header_length + hdr->json_length) {
        return CS_REST_MORE_DATA;
    }

    return CS_REST_OKAY;
}

/* 
 * Create a packet based on the rest header and json data params 
 *
 * Returns number of bytes sent if successful
 */
int
cs_packet_create(uint8_t *buf, int len, cs_rest_header_t *hdr,
                 uint8_t *json_data, int json_len)
{
    int pkt_len;

    pkt_len = hdr->header_length;
    if (json_data != NULL) {
        pkt_len += json_len;
    }
    if (len < pkt_len) {
        return -1;
    }

    memcpy(buf, hdr->packed_header, hdr->header_length);
    if ((json_data != NULL) && (json_len != 0)) {
        memcpy(buf + hdr->header_length, json_data, json_len);
    }

    return pkt_len;
}
