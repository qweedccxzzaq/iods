/*
 * TBD:  Need standard header
 * Copyright 2010 Big Switch Networks
 */

#if !defined(_CS_CMDSRV_H_)
#define _CS_CMDSRV_H_

#include <openflow/openflow.h>
#include <json/json.h>

#include <udatapath/datapath.h>
#include <udatapath/switch-flow.h>
#include <udatapath/table.h>

#include "rest.h"
#include "cs_int.h"

/* Command server administrative overhead */
typedef struct cs_config_s {
    int listen_port;  /* On which port to listen for connections */
    uint32_t net_addr;  /* On which IP addr to listen for connections */
    int serial_connections; /* Only allow one connection at a time */
} cs_config_t;

#define CS_LOG printf

/****************************************************************
 * Error codes
 ****************************************************************/

#define CS_OKAY 0
#define CS_ERROR_NONE 0
#define CS_ERROR_BAD_PARAM     -1
#define CS_ERROR_RESOURCE      -2
#define CS_ERROR_DRIVER        -3
#define CS_ERROR_REQUEST       -4
#define CS_ERROR_UNKNOWN       -10

/*
 * Port configuration get/set
 */

typedef struct cs_port_config_s {
    int port;
    int speed;
    int mtu;
    int autoneg;
    int enable;
    int rx_pause;
    int tx_pause;
    int link;       /* Ignored on set */
    int duplex;
} cs_port_config_t;

/*
 * Internal representation of which fields in the port config structure 
 * are to be accessed.  These values do not go on the wire
 */

typedef enum cs_port_config_masks_e {
    CS_PORT_CONFIG_PORT      = (1 << 0),
    CS_PORT_CONFIG_SPEED     = (1 << 1),
    CS_PORT_CONFIG_MTU       = (1 << 2),
    CS_PORT_CONFIG_AUTONEG   = (1 << 3),
    CS_PORT_CONFIG_ENABLE    = (1 << 4),
    CS_PORT_CONFIG_RX_PAUSE  = (1 << 5),
    CS_PORT_CONFIG_TX_PAUSE  = (1 << 6),
    CS_PORT_CONFIG_LINK      = (1 << 7),
    CS_PORT_CONFIG_DUPLEX    = (1 << 8),
    CS_PORT_CONFIG_ALL       = ((1 << 9) - 1)
} cs_port_config_masks_t;

/*
 * Port statistics
 */
typedef struct cs_port_statistics_s {
    uint64_t tx_bytes;
    uint64_t tx_uc_pkts;
    uint64_t tx_bcmc_pkts;
    uint64_t tx_errors;
    uint64_t rx_bytes;
    uint64_t rx_uc_pkts;
    uint64_t rx_bcmc_pkts;
    uint64_t rx_errors;
} cs_port_statistics_t;

/****************************************************************/

extern void cmdsrv_thread(int argc, char **argv);


/****************************************************************/



/* Per message pack/unpack functions */

/* Port configuration */
extern int cs_json_port_config_pack(cs_port_config_t *config, uint8_t *buffer,
                                    int len, uint32_t to_pack);
extern int cs_json_port_config_unpack(cs_port_config_t *config, uint8_t *buffer,
                                      uint32_t *unpacked, int ignore_error);
extern int cs_port_config_fill(int unit, int port, int of_port,
                               cs_port_config_t *config);
extern int cs_port_config_apply(int unit, int port, cs_port_config_t *config,
                                uint32_t get_mask);

/* Port statistics */
extern int cs_json_port_statistics_pack(int of_port, 
                                        cs_port_statistics_t *stats,
                                        uint8_t *buffer, int len);
extern int cs_json_port_statistics_unpack(int *of_port, 
                                          cs_port_statistics_t *stats,
                                          uint8_t *buffer, int ignore_error);
extern int cs_port_statistics_fill(int of_port, cs_port_statistics_t *stats);

extern int cs_json_port_request_pack(int of_port, uint8_t *buffer, int len);
extern int cs_json_port_request_unpack(int *of_port, uint8_t *buffer, 
                                       int ignore_error);

/* Flow entry dump */

/* Vendor action max bytes */
#define VENDOR_BYTES_MAX 64
#define VENDOR_STRING_LEN ((VENDOR_BYTES_MAX * 2) + 1)

extern int of_json_action_list_unpack(struct ofp_action_header *actions, 
                                      int bytes, json_object *obj);
extern json_object *of_json_action_list_pack(struct ofp_action_header *actions,
                                             int bytes);

extern json_object *of_json_sw_flow_pack(struct sw_flow *flow, 
                                         uint16_t out_port);
extern int of_json_sw_flow_unpack(struct sw_flow *flow, int flow_bytes,
                                  uint16_t *out_port, json_object *obj);

/* Management statistics */
/* TBD */

/* Controller connection status */
/* TBD */

/* Logging level */
/* TBD */

/* Fail mechanism */
/* TBD */

#if 0
/****************************************************************/
/****************************************************************/

/*
 * Transmit:
 */

/* cs_message_init
 * Initialize a message (TCP content) with proper header
 *
 * params_start: Output parameter the points to beginning of where JSON 
 * parameters should be placed. 
 */
extern int cs_message_init(uint8_t *buffer, int len, int request,
                           uint8_t **params_start);

/*
 * Once the buffer is initialized to the proper type, pack the JSON parameters
 * into the buffer.  Then call the message_close function to complete the header
 * setup.
 */
extern int cs_message_close(uint8_t *buffer, int len, int params_len);

/****************************************************************/
#endif

#endif
