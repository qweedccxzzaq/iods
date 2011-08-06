-- Copyright (c) 2010 Big Switch Networks
-- ui_cs_op.lua
-- Low level cmdsrv operations

require "ui_rest"
require "ui_cs_cxn"
require "json"

-- Returns the REST header status and a table of entries, each of which 
-- is the processed JSON data for that segment
-- Returns status == nil if unable to communicate with server
function parse_reply(reply, bytes)
   local result = {}
   local status = nil
   local offset = 1
   local hdr = {}
   local packed_hdr = ""
   while bytes > 0 do
      ui_dbg_verb("Parse reply, %d (aka %d) remaining\n", bytes, #reply)
      packed_hdr = string.sub(reply, offset, offset + REST_HEADER_MAX)
      hdr = cs_rest_header_parse(packed_hdr, bytes)
      if hdr == nil then break end
      status = tonumber(hdr.status_code)
      offset = offset + hdr.header_length
      if tonumber(hdr.json_length) > 0 then
         entry = Json.Decode(string.sub(reply, offset, 
                                        offset + hdr.json_length))
         if entry == nil then
            ui_dbg_error("Error parsing reply for transaction %d\n", 
                         hdr.transaction_id)
            break
         end
         table.insert(result, entry)
      end
      bytes = bytes - (hdr.header_length + hdr.json_length)
      offset = offset + hdr.json_length
   end
   return status, result
end

-- Run the transaction to get the flow table.  
-- return -1 for status if error occurs on transaction
function cs_flowtable_get()
   local hdr = setup_rest_header("GET", CS_URI_PREFIX .. "flowtable", 0)

   reply, bytes, err_str = transact(hdr.packed_header)
   if bytes < 0 then
      return CS_REST_INTERNAL, nil, err_str
   end
   return parse_reply(reply, bytes)
end

-- Get driver information
function cs_info_get()
   -- Only need the header
   local hdr = setup_rest_header("GET", CS_URI_PREFIX .. "info", 0)
   local buf = hdr.packed_header

   reply, bytes, err_str = transact(buf)
   if bytes < 0 then
      return CS_REST_INTERNAL, nil, err_str
   end
   return parse_reply(reply, bytes)
end

-- Get driver information
function cs_mgmt_stats_get()
   -- Only need the header
   local hdr = setup_rest_header("GET", CS_URI_PREFIX .. "mgmtstats", 0)
   local buf = hdr.packed_header

   reply, bytes, err_str = transact(buf)
   if bytes < 0 then
      return CS_REST_INTERNAL, nil, err_str
   end
   return parse_reply(reply, bytes)
end


-- Get port statistics
function cs_portstats_get(port)
   local json_table = { ["port"] = tostring(port) }
   local json_str = Json.Encode(json_table)
   hdr = setup_rest_header("GET", CS_URI_PREFIX .. "portstats", #json_str)

   reply, bytes, err_str = transact(hdr.packed_header .. json_str)
   if bytes < 0 then
      return CS_REST_INTERNAL, nil, err_str
   end
   return parse_reply(reply, bytes)
end

-- Get port status information
function cs_port_get(port)
   local json_table = { ["port"] = tostring(port) }
   local json_str = Json.Encode(json_table)
   hdr = setup_rest_header("GET", CS_URI_PREFIX .. "port", #json_str)

   reply, bytes, err_str = transact(hdr.packed_header .. json_str)
   if bytes < 0 then
      return CS_REST_INTERNAL, nil, err_str
   end
   return parse_reply(reply, bytes)
end

-- Set port parameters
function cs_port_set(params)
   -- Do these by hand since there's only a few
   local json_table = {}

   json_table["port"] = params.port
   json_table["speed"] = params.speed
   json_table["enable"] = params.enable
   json_table["mtu"] = params.mtu
   json_table["autoneg"] = params.autoneg
   json_table["rx_pause"] = params.rx_pause
   json_table["tx_pause"] = params.tx_pause
   json_table["duplex"] = params.duplex
   local json_str = Json.Encode(json_table)
   ui_dbg_verb("js " .. json_str)
   local hdr = setup_rest_header("PUT", CS_URI_PREFIX .. "port", #json_str)
   local reply, bytes = transact(hdr.packed_header .. json_str)
   if bytes < 0 then
      return CS_REST_INTERNAL, nil, err_str
   end
   return parse_reply(reply, bytes)
end

-- Set loglevel
function cs_loglevel_set(level)
   local json_table = {}

   json_table["level"] = level
   local json_str = Json.Encode(json_table)
   ui_dbg_verb("js " .. json_str)
   local hdr = setup_rest_header("PUT", CS_URI_PREFIX .. "loglevel", #json_str)
   local reply, bytes = transact(hdr.packed_header .. json_str)
   if bytes < 0 then
      return CS_REST_INTERNAL, nil, err_str
   end
   return parse_reply(reply, bytes)
end

-- Get driver information
function cs_drvcmd(unit, command)
   -- TODO:  Allow unit to be specified
   local json_table = {}
   json_table.command = command
   json_table.unit = unit
   local json_str = Json.Encode(json_table)
   local hdr = setup_rest_header("GET", CS_URI_PREFIX .. "drvcmd", #json_str)

   -- todo:  Record number of bytes in ofswd.log; transact; dump end of log

   local reply, bytes = transact(hdr.packed_header .. json_str)
   if bytes < 0 then
      return CS_REST_INTERNAL, nil, err_str
   end
   return parse_reply(reply, bytes)
end

-- This should probably just be hardcoded in platform.lua
platform_port_count = -1
function port_count_get()
   -- port count is cached in platform_port_count
   if platform_port_count < 0 then
      status, results, err_str = cs_mgmt_stats_get()
      report_rest_status(status, err_str)

      if results == nil or #results == 0 or results[1]["port_count"] == nil then
         printf("Could not get port count from management stats\n")
      else
         platform_port_count = results[1]["port_count"]
      end
   end

   return platform_port_count
end
