-- Copyright (c) 2010 Big Switch Networks
-- ui_rest.lua
-- REST functions for CLI and Web UI

require "ui_utils"

end_of_string = ' '

transaction_id = 1
function setup_rest_header(op, uri, jlen)
   hdr = {}
   hdr.op = op
   hdr.uri = uri
   hdr.transaction_id = transaction_id
   transaction_id = transaction_id + 1
   hdr.json_length = tostring(jlen)
   hdr.status_code = CS_REST_OKAY
   hdr = cs_rest_header_pack(hdr)

   return hdr
end

-- String terminator gets tricky in Lua; #str reports proper value of
-- entire string.  Our split currently has problems with using 0

function cs_rest_header_pack(hdr)
   hdr.packed_header = tostring(hdr.op)
   hdr.packed_header = hdr.packed_header .. end_of_string .. hdr.uri
   hdr.packed_header = hdr.packed_header .. end_of_string .. 
      tostring(hdr.transaction_id)
   hdr.packed_header = hdr.packed_header .. end_of_string .. 
      tostring(hdr.status_code)
   hdr.packed_header = hdr.packed_header .. end_of_string .. 
      tostring(hdr.json_length) .. end_of_string
   hdr.header_length = #hdr.packed_header

   return hdr
end

CS_URI_PREFIX = "/cs/1.0/"
CS_URI_PREFIX_LEN = #CS_URI_PREFIX

-- Return pair cs_rest_status and error_string (if error)
function cs_rest_header_parse(buffer, len, hdr)
   if hdr == nil then hdr = {} end
   hdr.header_length = 0

   -- Will not work with end_of_string == '\0'
   t = split(buffer, end_of_string)

   if #t < 5 then
      return -1 -- fixme error code
   end

   hdr.header_length = #t[1] + #t[2] + #t[3] + #t[4] + #t[5] + 5

   hdr.op = t[1]
   hdr.uri = t[2]
   hdr.transaction_id = t[3]
   hdr.status_code = t[4]
   hdr.json_length = t[5]

   if len < hdr.header_length + hdr.json_length then
      return nil -- fixme error code
   end

   return hdr
end

-- Report rest status
function report_rest_status(status, err_str, outf)
   outf = outf or printf
   if status ~= CS_REST_OKAY then
      if not status then
         outf("ERROR: Status nil, operation did not complete.\n")
      else
         if cs_rest_status_strings[tonumber(status)] then
            outf("Warning: Received error status %d: %s\n", 
                   tonumber(status), cs_rest_status_strings[tonumber(status)])
         else
            outf("Warning: Unknown error status %d\n", tonumber(status))
         end
      end
      if err_str then
         outf("Error string given: %s\n", err_str)
      end
   end
end
