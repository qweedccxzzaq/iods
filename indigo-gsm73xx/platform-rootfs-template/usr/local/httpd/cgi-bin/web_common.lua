-- Common UI information
-- Copyright 2011 Big Switch Networks

-- This defines the set of links at the top of the page

require "ui_utils"
require "ui_cs_op"

ver = get_build_version()

-- Print enabled/disabled based on boolean val
function two_choice(val, s_0, s_1)
   if not val then return s_0 end
   if tonumber(val) == 0 then return s_0 end
   return s_1
end

-- Print enabled/disabled based on boolean val
function en_dis(val)
   if val == 1 then return "enabled" else return "disabled" end
end

-- For radio buttons etc, indicate if entry should be checked/selected
function get_chk_val(var, val, str)
   str = str or "CHECKED" -- default is "checked"
   if tostring(var) == tostring(val) then print(str) end
end

function get_chk_non_nil(var, str)
   str = str or "CHECKED" -- default is "checked"
   if var ~= nil then print(str) end
end

function get_chk_is_nil(var, str)
   str = str or "CHECKED" -- default is "checked"
   if var == nil then print(str) end
end

-- Get port count, generate error info and return -1 on error
function get_port_count()
   -- First get mgmt stats and determine port count
   status, results, err_str = cs_mgmt_stats_get()
   if status ~= CS_REST_OKAY or results == nil then
      printf("<font color=red><b>"..
             "Error %s retrieving mgmt stats for port count: %s.\n",
             tostring(status), tostring(err_str))
      printf("</b></font>\n")
      return -1
   end
   return results[1].port_count
end

-- Print the HTML for table headings
-- hdr_string is a string of | separated column headings
function table_start(table_dec, row_dec, hdr_string)
   print(table_dec)
   print(row_dec)
   local hdrs = split(hdr_string, '|')

   for i,v in ipairs(hdrs) do
      if i == #hdrs then
         printf("<th %s>", last_col_elt)
      else
         printf("<th>")
      end
      print(tostring(v) .. "</th>")
   end
   print("</tr>")
end

--
-- Common element definitions
--

even_row_elt = 'class="even"'
odd_row_elt = 'class="odd"'
last_col_elt = 'class="last"'

-- Print the heading for a row <tr class=...> based on index and total count
function start_row(i)
   if (i % 2 == 0) then
      print("<tr " .. even_row_elt .. ">")
   else
      print("<tr " .. odd_row_elt .. ">")
   end
end

if get_port_count() > 40 then
   top_matter = "<h1>GSM7352S"
else
   top_matter = "<h1>GSM7328S"
end

top_matter = top_matter .. "<span><strong>Firmware version: </strong>" .. ver .. "</span></h1>"

top_matter = top_matter .. [[

<p id="logo"><img src="../i/logo.png" alt="" /></p>

<!--<table cellpadding="5" border="1" align="center">
<tr>
<td align=center><a href="settings">OpenFlow System<br>Configuration</a></td>
<td align=center><a href="flowtable">OpenFlow<br>Flow Table</a></td>
<td align=center><a href="portstats">Port<br>Statistics</a></td>
<td align=center><a href="port">Port Control<br>and Status</a></td>
<td align=center><a href="control">OpenFlow<br>System Control</a></td>
<td align=center><a href="logs">OpenFlow<br>Logs</a></td>
</tr>
</table>-->
]]
