/*
 * Copyright 2010 Big Switch Networks
 *
 * This code is released under the OpenFlow license.
 *
 * We are making this software available for public use and benefit
 * with the expectation that others will use, modify and enhance the 
 * Software and contribute those enhancements back to the community. 
 * However, since we would like to make the Software available for 
 * broadest use, with as few restrictions as possible permission is 
 * hereby granted, free of charge, to any person obtaining a copy of 
 * this Software to deal in the Software under the copyrights without 
 * restriction, including without limitation the rights to use, copy, 
 * modify, merge, publish, distribute, sublicense, and/or sell copies 
 * of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED -Y´AS IS¡, WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS 
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The name and trademarks of copyright holder(s) may NOT be used in 
 * advertising or publicity pertaining to the Software or any 
 * derivatives without specific, written prior permission.
 *
 */

/*
 * OpenFlow flow object to/from JSON object parsing
 *
 * These routines encode/decode OpenFlow flow C structures to/from
 * JSON representations.
 *
 * Limitations:  Vendor actions are limited to 64 bytes of data
 * beyond the vendor header.  They are passed as raw data (encoded as
 * a hex string) and hence any data contained therein is in the host
 * order of the sending machine.
 *
 */


#include <json/json.h>
#include <json/json_object_private.h> /* Needed for iter_object */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <openflow/openflow.h>
#include "cmdsrv.h"

#define _SIMPLE_STR_LEN 32

/* Check a variable against a literal string */
#define _CHECK_KEY(key, str) (!(strncmp((key), (str), sizeof(str))))

/****************************************************************
 *
 * Pack/unpack utility functions
 *
 ****************************************************************/

static void
mac_to_str(const uint8_t *mac, char *str)
{
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void
str_to_mac(const char *str, uint8_t *mac)
{
    unsigned int lmac[6];
    int i;

    /* Scan into long ints and then copy to chars. */
    sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x", 
           &lmac[0], &lmac[1], &lmac[2], &lmac[3], &lmac[4], &lmac[5]);

    for (i = 0; i < 6; i++) {
        mac[i] = lmac[i];
    }
}

/* High and low ascii characters for a byte */
static char
hex_hi(uint8_t byte)
{
    byte = byte >> 8;
    if (byte < 10) {
        return '0' + byte;
    }
    return 'a' + (byte - 10);
}

static char
hex_lo(uint8_t byte)
{
    byte = byte & 0xf;
    if (byte < 10) {
        return '0' + byte;
    }
    return 'a' + (byte - 10);
}

/* Convert two hex characters to an octet */
static uint8_t
chars_to_byte(const char *string)
{
    uint8_t rv = 0;

    if (*string >= '0' && *string <= '9') {
        rv = *string - '0';
    } else { /* TODO verify *string in 'a' to 'f' */
        rv = *string - 'a';
    }
    rv = rv << 8;
    string++;
    if (*string >= '0' && *string <= '9') {
        rv += (*string - '0');
    } else { /* TODO verify *string in 'a' to 'f' */
        rv += (*string - 'a');
    }

    return rv;
}

/* Convert raw data to hex encoded string */
static void
octets_to_hexstr(uint8_t *data, char *string, int bytes)
{
    int idx;

    for (idx = 0; idx < bytes; idx++) {
        string[2 * idx] = hex_hi(*data);
        string[2 * idx + 1] = hex_lo(*data);
    }
    string[2 * bytes] = '\0';
}

/* Convert hex encoded string to raw data */
static void
hexstr_to_octets(const char *string, uint8_t *data, int len)
{
    int idx = 0;

    while (*string != '\0' && idx < len) {
        data[idx] = chars_to_byte(string);
        string += 2;
        idx++;
    }
}


/****************************************************************
 *
 * ACTION PACK FUNCTIONS
 *
 ****************************************************************/

/* Pack the action type and length into the given object */
static void
action_header_pack(struct ofp_action_header *hdr, json_object *jobj, char *nm)
{
    json_object *obj;

    obj = json_object_new_string(nm);
    json_object_object_add(jobj, "action_name", obj);
    obj = json_object_new_int(hdr->type);
    json_object_object_add(jobj, "type", obj);
    obj = json_object_new_int(hdr->len);
    json_object_object_add(jobj, "len", obj);
}

/****************************************************************
 * Per-action pack functions
 *
 * action_<type>_pack
 *
 * Parameters
 *     act: Pointer to an action structure of the appropriate type
 * Returns
 *     Pointer to a new json object representing the action
 *
 ****************************************************************/

static json_object *
action_output_pack(struct ofp_action_output *act, char *nm)
{
    json_object *int_obj;
    json_object *jobj;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);
    /* Add action_name here */
    int_obj = json_object_new_int(act->port);
    json_object_object_add(jobj, "port", int_obj);
    int_obj = json_object_new_int(act->max_len);
    json_object_object_add(jobj, "max_len", int_obj);
    DBG_VERBOSE("Pack output action port %d\n", act->port);

    return jobj;
}

static json_object *
action_set_vlan_vid_pack(struct ofp_action_vlan_vid *act, char *nm)
{
    json_object *int_obj;
    json_object *jobj;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);
    int_obj = json_object_new_int(act->vlan_vid);
    json_object_object_add(jobj, "vlan_vid", int_obj);

    return jobj;
}

static json_object *
action_set_vlan_pcp_pack(struct ofp_action_vlan_pcp *act, char *nm)
{
    json_object *int_obj;
    json_object *jobj;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);
    int_obj = json_object_new_int(act->vlan_pcp);
    json_object_object_add(jobj, "vlan_pcp", int_obj);

    return jobj;
}

static json_object *
action_strip_vlan_pack(struct ofp_action_header *act, char *nm)
{
    json_object *jobj;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);

    return jobj;
}

static json_object *
action_set_dl_addr_pack(struct ofp_action_dl_addr *act, char *nm)
{
    json_object *obj;
    json_object *jobj;
    char obj_string[_SIMPLE_STR_LEN];

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);

    mac_to_str(act->dl_addr, obj_string);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "dl_addr", obj);

    return jobj;
}

static json_object *
action_set_nw_addr_pack(struct ofp_action_nw_addr *act, char *nm)
{
    json_object *int_obj;
    json_object *jobj;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);
    int_obj = json_object_new_int(act->nw_addr);
    json_object_object_add(jobj, "nw_addr", int_obj);

    return jobj;
}

static json_object *
action_set_nw_tos_pack(struct ofp_action_nw_tos *act, char *nm)
{
    json_object *int_obj;
    json_object *jobj;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);
    int_obj = json_object_new_int(act->nw_tos);
    json_object_object_add(jobj, "nw_tos", int_obj);

    return jobj;
}

static json_object *
action_set_tp_port_pack(struct ofp_action_tp_port *act, char *nm)
{
    json_object *int_obj;
    json_object *jobj;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);
    int_obj = json_object_new_int(act->tp_port);
    json_object_object_add(jobj, "tp_port", int_obj);

    return jobj;
}

static json_object *
action_enqueue_pack(struct ofp_action_enqueue *act, char *nm)
{
    json_object *int_obj;
    json_object *jobj;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);
    int_obj = json_object_new_int(act->port);
    json_object_object_add(jobj, "port", int_obj);
    int_obj = json_object_new_int(act->queue_id);
    json_object_object_add(jobj, "queue_id", int_obj);

    return jobj;
}

static json_object *
action_vendor_pack(struct ofp_action_vendor_header *act, char *nm)
{
    json_object *int_obj;
    json_object *str_obj;
    json_object *jobj;
    char vendor_str[VENDOR_STRING_LEN];
    int bytes;

    jobj = json_object_new_object();
    action_header_pack((struct ofp_action_header *)act, jobj, nm);
    int_obj = json_object_new_int(act->vendor);
    json_object_object_add(jobj, "vendor", int_obj);

    bytes = act->len - sizeof(struct ofp_action_vendor_header);
    /* Pack the rest as a raw string */
    octets_to_hexstr(((uint8_t *)act) + sizeof(struct ofp_action_vendor_header),
                     vendor_str, bytes);
    str_obj = json_object_new_string(vendor_str);
    json_object_object_add(jobj, "data", str_obj);
    /* TODO Warn if length is too insufficient */
    
    return jobj;
}

/* Pack a single action pointed to by action; return number bytes consumed */ 
/* jobj is a JSON array object */
static int
action_pack(struct ofp_action_header *action, json_object *jobj)
{
    json_object *act_obj;
    char *add_str;

    act_obj = NULL;
    add_str = "";

    switch (action->type) {
    case OFPAT_OUTPUT:
        add_str = "output";
        act_obj = action_output_pack((struct ofp_action_output *)action, add_str);
        break;

    case OFPAT_SET_VLAN_VID:
        add_str = "set_vlan_vid";
        act_obj = action_set_vlan_vid_pack((struct ofp_action_vlan_vid *)action,
                                           add_str);
        break;

    case OFPAT_SET_VLAN_PCP:
        add_str = "set_vlan_pcp";
        act_obj = action_set_vlan_pcp_pack((struct ofp_action_vlan_pcp *)action,
                                           add_str);
        break;

    case OFPAT_STRIP_VLAN:
        add_str = "strip_vlan";
        act_obj = action_strip_vlan_pack((struct ofp_action_header *)action,
                                         add_str);
        break;

    case OFPAT_SET_DL_SRC:
        add_str = "set_dl_src";
        act_obj = action_set_dl_addr_pack((struct ofp_action_dl_addr *)action,
                                          add_str);
        break;

    case OFPAT_SET_DL_DST:
        add_str = "set_dl_dst";
        act_obj = action_set_dl_addr_pack((struct ofp_action_dl_addr *)action,
                                          add_str);
        break;

    case OFPAT_SET_NW_SRC:
        add_str = "set_nw_src";
        act_obj = action_set_nw_addr_pack((struct ofp_action_nw_addr *)action,
                                          add_str);
        break;

    case OFPAT_SET_NW_DST:
        add_str = "set_nw_dst";
        act_obj = action_set_nw_addr_pack((struct ofp_action_nw_addr *)action,
                                          add_str);
        break;

    case OFPAT_SET_NW_TOS:
        add_str = "set_nw_tos";
        act_obj = action_set_nw_tos_pack((struct ofp_action_nw_tos *)action,
                                         add_str);
        break;

    case OFPAT_SET_TP_SRC:
        add_str = "set_tp_src";
        act_obj = action_set_tp_port_pack((struct ofp_action_tp_port *)action,
                                          add_str);
        break;

    case OFPAT_SET_TP_DST:
        add_str = "set_tp_dst";
        act_obj = action_set_tp_port_pack((struct ofp_action_tp_port *)action,
                                          add_str);
        break;

    case OFPAT_ENQUEUE:
        add_str = "enqueue";
        act_obj = action_enqueue_pack((struct ofp_action_enqueue *)action,
                                      add_str);
        break;

    case OFPAT_VENDOR:
        add_str = "vendor";
        act_obj = action_vendor_pack((struct ofp_action_vendor_header *)action,
                                     add_str);
        break;

    default:
        return 0;
        break;
    }

    json_object_array_add(jobj, act_obj);
    DBG_VVERB("Packed action %s len %d\n", add_str, action->len);

    return action->len;
}

/*
 * of_json_action_list_pack
 *
 * Create a JSON object for an array of actions of variable type
 *
 * @param actions Pointer to the action list to pack
 * @param bytes The number of bytes in the action list
 * @return Pointer to the resulting JSON object or NULL if error
 */

json_object *
of_json_action_list_pack(struct ofp_action_header *actions, int bytes)
{
    json_object *jobj;
    int packed_bytes;
    int tot_bytes = 0;

    jobj = json_object_new_array();
    while (bytes > 0) {
        packed_bytes = action_pack(actions, jobj);
        if (packed_bytes == 0) {
            /* TODO free jobj */
            return NULL;
        }        

        bytes -= packed_bytes;
        tot_bytes += packed_bytes;
        actions = (struct ofp_action_header *)
            (((uint8_t *)actions) + packed_bytes);
    }

    DBG_VERBOSE("Packed action list with %d bytes\n", tot_bytes);
    return jobj;
}


/****************************************************************
 *
 * ACTION UNPACK FUNCTIONS
 *
 ****************************************************************/

/****************************************************************
 * Per-action unpack functions
 *
 * action_<type>_unpack
 *
 * Parameters
 *     jobj: The JSON object representing the action
 *     act:  Pointer to an action of the right type to be populated
 *     bytes:  Number of bytes available in act
 * Returns
 *     Number of bytes unpacked, or -1 on error
 *
 ****************************************************************/

/****************************************************************
 * Per-action pack functions
 ****************************************************************/

static int
action_output_unpack(json_object *jobj, struct ofp_action_output *act,
                     int bytes)
{
    json_object_iter iter;

    if (bytes < sizeof(struct ofp_action_output)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "port")) {
            act->port = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "max_len")) {
            act->max_len = json_object_get_int(iter.val);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_output);
}

static int
action_set_vlan_vid_unpack(json_object *jobj, struct ofp_action_vlan_vid *act,
                           int bytes)
{
    json_object_iter iter;

    if (bytes < sizeof(struct ofp_action_vlan_vid)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "vlan_vid")) {
            act->vlan_vid = json_object_get_int(iter.val);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_vlan_vid);
}

static int
action_set_vlan_pcp_unpack(json_object *jobj, struct ofp_action_vlan_pcp *act,
                           int bytes)
{
    json_object_iter iter;

    if (bytes < sizeof(struct ofp_action_vlan_pcp)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "vlan_pcp")) {
            act->vlan_pcp = json_object_get_int(iter.val);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_vlan_pcp);
}

static int
action_strip_vlan_unpack(json_object *jobj, struct ofp_action_header *act,
                         int bytes)
{
    json_object_iter iter;

    if (bytes < sizeof(struct ofp_action_header)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_header);
}

static int
action_set_dl_addr_unpack(json_object *jobj, struct ofp_action_dl_addr *act,
                          int bytes)
{
    json_object_iter iter;
    const char *obj_string;

    if (bytes < sizeof(struct ofp_action_dl_addr)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "dl_addr")) {
            obj_string = json_object_get_string(iter.val);
            str_to_mac(obj_string, act->dl_addr);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_dl_addr);
}

static int
action_set_nw_addr_unpack(json_object *jobj, struct ofp_action_nw_addr *act,
                          int bytes)
{
    json_object_iter iter;

    if (bytes < sizeof(struct ofp_action_nw_addr)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_addr")) {
            act->nw_addr = json_object_get_int(iter.val);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_nw_addr);
}

static int
action_set_nw_tos_unpack(json_object *jobj, struct ofp_action_nw_tos *act,
                         int bytes)
{
    json_object_iter iter;

    if (bytes < sizeof(struct ofp_action_nw_tos)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_tos")) {
            act->nw_tos = json_object_get_int(iter.val);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_nw_tos);
}

static int
action_set_tp_port_unpack(json_object *jobj, struct ofp_action_tp_port *act,
                          int bytes)
{
    json_object_iter iter;

    if (bytes < sizeof(struct ofp_action_tp_port)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "tp_port")) {
            act->tp_port = json_object_get_int(iter.val);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_tp_port);
}

static int
action_enqueue_unpack(json_object *jobj, struct ofp_action_enqueue *act,
                      int bytes)
{
    json_object_iter iter;

    if (bytes < sizeof(struct ofp_action_enqueue)) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "port")) {
            act->port = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "queue_id")) {
            act->queue_id = json_object_get_int(iter.val);
        } else {
            return -1;
        }
    }

    return sizeof(struct ofp_action_enqueue);
}

static int
action_vendor_unpack(json_object *jobj, struct ofp_action_vendor_header *act,
                     int bytes)
{
    json_object_iter iter;
    int data_bytes;
    const char *str_ptr;

    bytes -= sizeof(struct ofp_action_vendor_header);
    if (bytes < 0) {
        return -1;
    }

    json_object_object_foreachC(jobj, iter) {
        if (_CHECK_KEY(iter.key, "type")) {
            act->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "len")) {
            act->len = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "vendor")) {
            act->vendor = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "data")) {
            str_ptr = json_object_get_string(iter.val);
            data_bytes = strlen(str_ptr)/2;
            if (data_bytes > bytes) {
                return -1;
            }
            hexstr_to_octets(str_ptr, ((uint8_t *)act) +
                             sizeof(struct ofp_action_vendor_header),
                             data_bytes);
        } else {
            return -1;
        }
        
    }

    return sizeof(struct ofp_action_vendor_header) + data_bytes;
}

/*
 * of_json_action_list_unpack
 *
 * Unpack an action list from a JSON object.
 *
 * @param actions Pointer to the space allocated for the action list
 * @param bytes The number of bytes allocated for the action list.
 * @param obj The object to be unpacked
 * @return Number of bytes unpacked if successful; -1 otherwise
 */

int
of_json_action_list_unpack(struct ofp_action_header *actions, int bytes, 
                           json_object *obj)
{
    int act_bytes;
    int unpacked = 0;
    json_object_iter iter;

    json_object_object_foreachC(obj, iter) {
        if (_CHECK_KEY(iter.key, "output")) {
            act_bytes = action_output_unpack(iter.val, 
                (struct ofp_action_output *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_vlan_vid")) {
            act_bytes = action_set_vlan_vid_unpack(iter.val, 
                (struct ofp_action_vlan_vid *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_vlan_pcp")) {
            act_bytes = action_set_vlan_pcp_unpack(iter.val, 
                (struct ofp_action_vlan_pcp *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "strip_vlan")) {
            act_bytes = action_strip_vlan_unpack(iter.val, 
                (struct ofp_action_header *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_dl_src")) {
            act_bytes = action_set_dl_addr_unpack(iter.val, 
                (struct ofp_action_dl_addr *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_dl_dst")) {
            act_bytes = action_set_dl_addr_unpack(iter.val, 
                (struct ofp_action_dl_addr *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_nw_src")) {
            act_bytes = action_set_nw_addr_unpack(iter.val, 
                (struct ofp_action_nw_addr *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_nw_dst")) {
            act_bytes = action_set_nw_addr_unpack(iter.val, 
                (struct ofp_action_nw_addr *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_nw_tos")) {
            act_bytes = action_set_nw_tos_unpack(iter.val, 
                (struct ofp_action_nw_tos *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_tp_src")) {
            act_bytes = action_set_tp_port_unpack(iter.val, 
                (struct ofp_action_tp_port *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "set_tp_dst")) {
            act_bytes = action_set_tp_port_unpack(iter.val, 
                (struct ofp_action_tp_port *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "enqueue")) {
            act_bytes = action_enqueue_unpack(iter.val, 
                (struct ofp_action_enqueue *)actions, bytes);
        } else if (_CHECK_KEY(iter.key, "vendor")) {
            act_bytes = action_vendor_unpack(iter.val, 
                (struct ofp_action_vendor_header *)actions, bytes);
        } else {
            DBG_ERROR("Unknown key in action list: %s\n", iter.key);
            return -1;
        }

        DBG_VERBOSE("Unpacked action %s with %d bytes\n", 
                      iter.key, act_bytes);

        if (act_bytes <= 0) {
            return CS_ERROR_BAD_PARAM;
        }
        unpacked += act_bytes;
        bytes -= act_bytes;
        actions = (struct ofp_action_header *)((uint8_t *)actions + act_bytes);
    }


    DBG_VERBOSE("Unpacked action list with %d bytes\n", unpacked);

    return unpacked;
}

/*
 * of_json_sw_flow_pack
 *
 * Pack the sw_flow object into a JSON object
 *
 * @param flow The sw_flow object to be packed
 * @param out_port The out_port (for query only)
 * @return The JSON object created
 *
 * Assumes data structure entries are in host byte order
 */

json_object *
of_json_sw_flow_pack(struct sw_flow *flow, uint16_t out_port)
{
    json_object *jobj;
    json_object *obj;
    char obj_string[_SIMPLE_STR_LEN];
    struct sw_flow_key *key;
    struct flow *match;

    key = &flow->key;
    match = &key->flow;

    DBG_VERBOSE("Packing SW flow object\n");
    jobj = json_object_new_object();
    obj = json_object_new_int(key->wildcards);
    json_object_object_add(jobj, "wildcards", obj);

    /* Should these only be packed based on wildcard settings? */
    obj = json_object_new_int(match->in_port);
    json_object_object_add(jobj, "in_port", obj);

    mac_to_str(match->dl_src, obj_string);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "dl_src", obj);
    mac_to_str(match->dl_dst, obj_string);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "dl_dst", obj);
    obj = json_object_new_int(match->dl_vlan);
    json_object_object_add(jobj, "dl_vlan", obj);
    obj = json_object_new_int(match->dl_vlan_pcp);
    json_object_object_add(jobj, "dl_vlan_pcp", obj);
    obj = json_object_new_int(match->dl_type);
    json_object_object_add(jobj, "dl_type", obj);

    obj = json_object_new_int(match->nw_tos);
    json_object_object_add(jobj, "nw_tos", obj);
    obj = json_object_new_int(match->nw_proto);
    json_object_object_add(jobj, "nw_proto", obj);
    obj = json_object_new_int(match->nw_src);
    json_object_object_add(jobj, "nw_src", obj);
    obj = json_object_new_int(key->nw_src_mask);
    json_object_object_add(jobj, "nw_src_mask", obj);
    obj = json_object_new_int(match->nw_dst);
    json_object_object_add(jobj, "nw_dst", obj);
    obj = json_object_new_int(key->nw_dst_mask);
    json_object_object_add(jobj, "nw_dst_mask", obj);

    obj = json_object_new_int(match->tp_src);
    json_object_object_add(jobj, "tp_src", obj);
    obj = json_object_new_int(match->tp_dst);
    json_object_object_add(jobj, "tp_dst", obj);

    snprintf(obj_string, _SIMPLE_STR_LEN, "%"PRIu64, flow->cookie);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "cookie", obj);

    obj = json_object_new_int(flow->idle_timeout);
    json_object_object_add(jobj, "idle_timeout", obj);
    obj = json_object_new_int(flow->hard_timeout);
    json_object_object_add(jobj, "hard_timeout", obj);

    obj = json_object_new_int(flow->priority);
    json_object_object_add(jobj, "priority", obj);

    snprintf(obj_string, _SIMPLE_STR_LEN, "%"PRIu64, flow->packet_count);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "packet_count", obj);
    snprintf(obj_string, _SIMPLE_STR_LEN, "%"PRIu64, flow->byte_count);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "byte_count", obj);
    snprintf(obj_string, _SIMPLE_STR_LEN, "%"PRIu64, flow->created);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "created", obj);
    snprintf(obj_string, _SIMPLE_STR_LEN, "%"PRIu64, flow->used);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "used", obj);

    obj = json_object_new_int(out_port);
    json_object_object_add(jobj, "out_port", obj);

    if (flow->sf_acts != NULL && flow->sf_acts->actions_len > 0) {
        obj = of_json_action_list_pack(flow->sf_acts->actions,
                                       flow->sf_acts->actions_len);
        DBG_VVERB("act string: %s\n", json_object_to_json_string(obj));
        json_object_object_add(jobj, "actions", obj);
    }

    return jobj;
}

/*
 * of_json_sw_flow_unpack
 *
 * Unpack a JSON object that holds flow match criteria
 * Set up the wildcards bitmap to indicate which values found
 *
 * @param flow The sw_flow object to be filled in
 * @param out_port Pointer to int holding output port
 * @param obj Pointer to the JSON object containing the flow mod
 *
 * This routine does not allocate additional space for the flow_mod.  If
 * there are insufficient bytes to unpack the action list, an error is 
 * returned.
 */

int
of_json_sw_flow_unpack(struct sw_flow *flow, int flow_bytes, 
                       uint16_t *out_port, json_object *obj)

{
    json_object_iter iter;
    struct sw_flow_key *key;
    const char *obj_string;
    uint32_t wildcards;
    int wc_seen = false; /* Were wildcards seen in object? */
    int bytes;

    key = &flow->key;
    wildcards = OFPFW_ALL;
    if (out_port != NULL) {
        *out_port = OFPP_NONE;
    }

    bytes = sizeof(*flow);
    if (flow_bytes < bytes) {
        return -1;
    }

    /* TODO:  What if nw_src_mask/nw_dst_mask are not specified? */

    json_object_object_foreachC(obj, iter) {
        if (_CHECK_KEY(iter.key, "wildcards")) {
            key->wildcards = json_object_get_int(iter.val);
            wc_seen = true;
        } else if (_CHECK_KEY(iter.key, "in_port")) {
            key->flow.in_port = json_object_get_int(iter.val);
            wildcards &= ~OFPFW_IN_PORT;

        } else if (_CHECK_KEY(iter.key, "dl_src")) {
            obj_string = json_object_get_string(iter.val);
            str_to_mac(obj_string, key->flow.dl_src);
            wildcards &= ~OFPFW_DL_SRC;
        } else if (_CHECK_KEY(iter.key, "dl_dst")) {
            obj_string = json_object_get_string(iter.val);
            str_to_mac(obj_string, key->flow.dl_dst);
            wildcards &= ~OFPFW_DL_DST;
        } else if (_CHECK_KEY(iter.key, "dl_vlan")) {
            key->flow.dl_vlan = json_object_get_int(iter.val);
            wildcards &= ~OFPFW_DL_VLAN;
        } else if (_CHECK_KEY(iter.key, "dl_vlan_pcp")) {
            key->flow.dl_vlan_pcp = json_object_get_int(iter.val);
            wildcards &= ~OFPFW_DL_VLAN_PCP;
        } else if (_CHECK_KEY(iter.key, "dl_type")) {
            key->flow.dl_type = json_object_get_int(iter.val);
            wildcards &= ~OFPFW_DL_TYPE;

        } else if (_CHECK_KEY(iter.key, "nw_tos")) {
            key->flow.nw_tos = json_object_get_int(iter.val);
            wildcards &= ~OFPFW_NW_TOS;
        } else if (_CHECK_KEY(iter.key, "nw_proto")) {
            key->flow.nw_proto = json_object_get_int(iter.val);
            wildcards &= ~OFPFW_NW_PROTO;
        } else if (_CHECK_KEY(iter.key, "nw_src")) {
            key->flow.nw_src = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_dst")) {
            key->flow.nw_dst = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_src_mask")) {
            key->nw_src_mask = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_dst_mask")) {
            key->nw_dst_mask = json_object_get_int(iter.val);

        } else if (_CHECK_KEY(iter.key, "tp_src")) {
            key->flow.tp_src = json_object_get_int(iter.val);
            wildcards &= ~OFPFW_TP_SRC;
        } else if (_CHECK_KEY(iter.key, "tp_dst")) {
            key->flow.tp_dst = json_object_get_int(iter.val);
            wildcards &= ~OFPFW_TP_DST;

        } else if (_CHECK_KEY(iter.key, "cookie")) {
            obj_string = json_object_get_string(iter.val);
            sscanf(obj_string, "%"PRIu64, &flow->cookie);

        } else if (_CHECK_KEY(iter.key, "idle_timeout")) {
            flow->idle_timeout = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "hard_timeout")) {
            flow->hard_timeout = json_object_get_int(iter.val);

        } else if (_CHECK_KEY(iter.key, "priority")) {
            flow->priority = json_object_get_int(iter.val);

        } else if (_CHECK_KEY(iter.key, "packet_count")) {
            obj_string = json_object_get_string(iter.val);
            sscanf(obj_string, "%"PRIu64, &flow->packet_count);
        } else if (_CHECK_KEY(iter.key, "byte_count")) {
            obj_string = json_object_get_string(iter.val);
            sscanf(obj_string, "%"PRIu64, &flow->byte_count);
        } else if (_CHECK_KEY(iter.key, "created")) {
            obj_string = json_object_get_string(iter.val);
            sscanf(obj_string, "%"PRIu64, &flow->created);
        } else if (_CHECK_KEY(iter.key, "used")) {
            obj_string = json_object_get_string(iter.val);
            sscanf(obj_string, "%"PRIu64, &flow->used);

        } else if (_CHECK_KEY(iter.key, "out_port")) {
            if (out_port != NULL) {
                *out_port = json_object_get_int(iter.val);
            }
        } else {
            return CS_ERROR_BAD_PARAM;
        }
    }
    if (!wc_seen) { /* Use wildcards based on fields seen */
        key->wildcards = wildcards;
    }

    /* TODO Unpack action lists; update bytes with amt unpacked */
    return bytes;
}

#if 0
/****************************************************************
 * 
 * DEAD CODE
 *
 * Was for packing/unpacking flow mods; may be resurrected some day
 *
 ****************************************************************/

/*
 * of_json_flow_mod_pack
 *
 * Pack the ofp_flow_mod object into a JSON object
 *
 * @param flow_mod The object to be packed
 * @return The JSON object created or NULL on error
 *
 * Assumes data structure entries are in host byte order
 */

json_object *
of_json_flow_mod_pack(struct ofp_flow_mod *flow_mod)
{
    json_object *jobj;
    json_object *obj;
    struct ofp_match *match;
    struct ofp_header *header;
    char obj_string[_SIMPLE_STR_LEN];

    match = &flow_mod->match;
    header= &flow_mod->header;

    jobj = json_object_new_object();

    obj = json_object_new_int(header->version);
    json_object_object_add(jobj, "version", obj);

    obj = json_object_new_int(header->type);
    json_object_object_add(jobj, "type", obj);

    obj = json_object_new_int(header->length);
    json_object_object_add(jobj, "length", obj);

    obj = json_object_new_int(header->xid);
    json_object_object_add(jobj, "xid", obj);

    /****************************************************************/

    obj = json_object_new_int(match->wildcards);
    json_object_object_add(jobj, "wildcards", obj);

    obj = json_object_new_int(match->in_port);
    json_object_object_add(jobj, "in_port", obj);

    mac_to_str(match->dl_src, obj_string);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "dl_src", obj);

    mac_to_str(match->dl_dst, obj_string);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "dl_dst", obj);

    obj = json_object_new_int(match->dl_vlan);
    json_object_object_add(jobj, "dl_vlan", obj);

    obj = json_object_new_int(match->dl_vlan_pcp);
    json_object_object_add(jobj, "dl_vlan_pcp", obj);

    obj = json_object_new_int(match->dl_type);
    json_object_object_add(jobj, "dl_type", obj);

    obj = json_object_new_int(match->nw_tos);
    json_object_object_add(jobj, "nw_tos", obj);

    obj = json_object_new_int(match->nw_proto);
    json_object_object_add(jobj, "nw_proto", obj);

    obj = json_object_new_int(match->nw_src);
    json_object_object_add(jobj, "nw_src", obj);

    obj = json_object_new_int(match->nw_dst);
    json_object_object_add(jobj, "nw_dst", obj);

    obj = json_object_new_int(match->tp_src);
    json_object_object_add(jobj, "tp_src", obj);

    obj = json_object_new_int(match->tp_dst);
    json_object_object_add(jobj, "tp_dst", obj);

    snprintf(obj_string, _SIMPLE_STR_LEN, "%"PRIu64, flow_mod->cookie);
    obj = json_object_new_string(obj_string);
    json_object_object_add(jobj, "cookie", obj);

    /****************************************************************/

    obj = json_object_new_int(flow_mod->command);
    json_object_object_add(jobj, "command", obj);

    obj = json_object_new_int(flow_mod->idle_timeout);
    json_object_object_add(jobj, "idle_timeout", obj);

    obj = json_object_new_int(flow_mod->hard_timeout);
    json_object_object_add(jobj, "hard_timeout", obj);

    obj = json_object_new_int(flow_mod->priority);
    json_object_object_add(jobj, "priority", obj);

    obj = json_object_new_int(flow_mod->out_port);
    json_object_object_add(jobj, "out_port", obj);

    obj = json_object_new_int(flow_mod->flags);
    json_object_object_add(jobj, "flags", obj);

    obj = of_json_action_list_pack(flow_mod->actions, flow_mod->header.length -
                           sizeof(struct ofp_flow_mod));
    json_object_object_add(jobj, "actions", obj);

    /* DEBUG */
    printf("packed object %s\n", json_object_to_json_string(jobj));

    return jobj;
}


/*
 * of_json_flow_mod_unpack
 *
 * Unpack a JSON object that holds a flow_mod.
 *
 * @param flow_mod The object to be filled up
 * @param flow_mod_bytes The number of bytes allocated for flow_mod
 * @param obj Pointer to the JSON object containing the flow mod
 * @return The number of bytes unpacked if successful.  <0 error code otherwise
 *
 * This routine does not allocate additional space for the flow_mod.  If
 * there are insufficient bytes to unpack the action list, an error is 
 * returned.
 */

int
of_json_flow_mod_unpack(struct ofp_flow_mod *flow_mod,
                        int flow_mod_bytes,
                        json_object *obj)
{
    
    json_object_iter iter;
    const char *obj_string;
    int unpacked;
    struct ofp_match *match;
    struct ofp_header *header;
    int act_bytes;

    match = &flow_mod->match;
    header= &flow_mod->header;

    /* Cheat, assuming entire header is properly unpacked */
    unpacked = sizeof(struct ofp_flow_mod);

    json_object_object_foreachC(obj, iter) {
        if (_CHECK_KEY(iter.key, "version")) {
            header->version = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "type")) {
            header->type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "length")) {
            header->length = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "xid")) {
            header->xid = json_object_get_int(iter.val);

        } else if (_CHECK_KEY(iter.key, "wildcards")) {
            match->wildcards = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "in_port")) {
            match->in_port = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "dl_src")) {
            obj_string = json_object_get_string(iter.val);
            str_to_mac(obj_string, match->dl_src);
        } else if (_CHECK_KEY(iter.key, "dl_dst")) {
            obj_string = json_object_get_string(iter.val);
            str_to_mac(obj_string, match->dl_dst);
        } else if (_CHECK_KEY(iter.key, "dl_vlan")) {
            match->dl_vlan = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "dl_vlan_pcp")) {
            match->dl_vlan_pcp = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "dl_type")) {
            match->dl_type = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_tos")) {
            match->nw_tos = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_proto")) {
            match->nw_proto = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_src")) {
            match->nw_src = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "nw_dst")) {
            match->nw_dst = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "tp_src")) {
            match->tp_src = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "tp_dst")) {
            match->tp_dst = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "cookie")) {
            obj_string = json_object_get_string(iter.val);
            sscanf(obj_string, "%"PRIu64, &flow_mod->cookie);

        } else if (_CHECK_KEY(iter.key, "command")) {
            flow_mod->command = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "idle_timeout")) {
            flow_mod->idle_timeout = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "hard_timeout")) {
            flow_mod->hard_timeout = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "priority")) {
            flow_mod->priority = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "out_port")) {
            flow_mod->out_port = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "flags")) {
            flow_mod->flags = json_object_get_int(iter.val);
        } else if (_CHECK_KEY(iter.key, "actions")) {
            act_bytes = of_json_action_list_unpack(flow_mod->actions,
                flow_mod_bytes - sizeof(struct ofp_flow_mod), iter.val);
            if (act_bytes < 0) {
                return OF_JSON_ERROR_BAD_PARAM;
            }
            unpacked += act_bytes;
        } else {
            return OF_JSON_ERROR_BAD_PARAM;
        }
    }

    if (header->length != unpacked) {
        DBG_ERROR("Error: Flow mod header length %d but unpacked %d\n",
                   header->length, unpacked);
    }
    return unpacked;
}

#endif 
