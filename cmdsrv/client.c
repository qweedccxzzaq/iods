/*
 * TBD:  Need standard header
 * Copyright 2011 Big Switch Networks
 *
 * Command server client library
 *
 * This library provides a set of C routines for interacting with the
 * command server as a client.
 */

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <json/json.h>
#include <netinet/in.h>
#include <stdio.h>

#include "cmdsrv_client.h"
#include "cs_int.h"
#include "rest.h"

#define SMALL_BUF 512
#define MEDIUM_BUF 1024
#define BIG_BUF 2048

FILE *dbg_fd = NULL;
int cs_debug_level = DBG_LVL_WARN;

/*
 * cmdsrv_client_connect
 *
 * Connect to a command server and return the socket descriptor
 * This should be used for one transaction and then 'close' called on the 
 * returned descriptor.
 *
 * If cs_addr is 0, then "localhost" (127.0.0.1) is used.
 * If cs_port is 0, then default port (CMDSRV_PORT) is used 
 *
 * Returns socket descriptor or -1 on error.
 */

int
cmdsrv_client_connect(unsigned int cs_addr, unsigned short int cs_port)
{
    int sd;
    struct sockaddr_in serv_addr;

    if (cs_port == 0) {
        cs_port = CMDSRV_PORT;
    }
    if (cs_addr == 0) {
        cs_addr = (127 << 24) | 1;
    }

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error creating socket for cmdsrv cxn\n");
        return -1;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    cs_addr = htonl(cs_addr);
    memcpy(&serv_addr.sin_addr.s_addr, &cs_addr, sizeof(cs_addr));
    serv_addr.sin_port = htons(cs_port);

    if (connect(sd, (struct sockaddr *)&serv_addr, 
                sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Error connecting socket for cmdsrv cxn\n");
        return -1;
    }

    return sd;
}

/*
 * Eventually, we should have an API for each operation with params
 */

/*
 * General model:
 *    Create connection
 *    Pack params (if any)
 *    Send request
 *    Get response
 *    Process return code
 *    Unpack params (if any)
 *    Return status
 */

int
cmdsrv_client_datapath_expire_set(unsigned int cs_addr, 
                                  unsigned short int cs_port,
                                  int enabled)
{
    int sd;
    json_object *jobj;
    json_object *intobj;
    uint8_t trx_buf[MEDIUM_BUF];
    uint8_t json_buf[SMALL_BUF];
    cs_rest_header_t hdr;
    int json_bytes, send_bytes, in_bytes, out_bytes;
    char *err_str;

    if (dbg_fd == NULL) {
        dbg_fd = stderr;
    }
    if ((sd = cmdsrv_client_connect(cs_addr, cs_port)) < 0) {
        fprintf(stderr, "Connection error in expire_set\n");
        return -1;
    }

    jobj = json_object_new_object();
    intobj = json_object_new_int(enabled);
    json_object_object_add(jobj, "enabled", intobj);
    json_bytes = snprintf((char *)json_buf, SMALL_BUF, "%s",
                          json_object_to_json_string(jobj)) + 1;

    if (json_bytes >= SMALL_BUF) {
        fprintf(stderr, "Internal cs_cli error in expire_set: buf size\n");
        return -1;
    }

    if (cs_rest_header_create(&hdr, "PUT", "/cs/1.0/dpexpire", 
                              1, 0, json_bytes) < 0) {
        fprintf(stderr, "Error packing rest hdr in expire_set\n");
        return -1;
    }

    send_bytes = 
        cs_packet_create(trx_buf, MEDIUM_BUF, &hdr, json_buf, json_bytes);
    out_bytes = send(sd, trx_buf, send_bytes, 0);

    if (out_bytes != send_bytes) {
        fprintf(stderr, "Error in expire_set: mismatch send bytes\n");
        fprintf(stderr, "  Intended %d. Actual %d\n", send_bytes, out_bytes);
        return -1;
    }

    /* FIXME:  Should get bytes until none returned */
    in_bytes = recv(sd, trx_buf, MEDIUM_BUF, 0);
    if (in_bytes <= 0) {
        fprintf(stderr, "Error in expire_set: Received %d bytes\n", in_bytes);
        return -1;
    }
    if (cs_rest_header_parse(trx_buf, in_bytes, &hdr, &err_str) < 0) {
        fprintf(stderr, "Error in expire_set: Could not parse response\n");
        return -1;
    }

    if (hdr.status_code != CS_REST_OKAY) {
        fprintf(stderr, "Error in expire_set: status code %d\n",
                hdr.status_code);
        return -1;
    }

    return 0;
}

