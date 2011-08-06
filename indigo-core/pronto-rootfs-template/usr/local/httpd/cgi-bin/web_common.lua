-- Common UI information
-- Copyright 2011 Big Switch Networks

-- This defines the set of links at the top of the page

require "ui_utils"
require "ui_cs_op"
require "platform"

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
   if var == val then print(str) end
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
   print("<thead>")
   print(row_dec)
   local hdrs = split(hdr_string, '|')

   for i,v in ipairs(hdrs) do
      --if i == #hdrs then
      --   printf("<th %s>", last_col_elt)
      --else
      --   printf("<th>")
      --end
      print("<th>" .. tostring(v) .. "</th>")
   end
   print("</tr>")
   print("</thead>")
   print("<tbody>")
end


--
-- Common element definitions
--

common_includes = [[
    <script type="text/javascript" src="/static/jq-ui-ind/js/jquery-1.4.4.min.js"></script>
    <script type="text/javascript" src="/static/jq-ui-ind/js/jquery-ui-1.8.9.custom.min.js"></script>
    <script type="text/javascript" src="/static/jq-plugins/jquery.dropshadow.js"></script>
    <script type="text/javascript" src="/static/jq-plugins/jquery.dataTables.min.js"></script>
    <script type="text/javascript" src="/static/jq-ui-ind/js/jquery-form.js"></script>
]]

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


-- FIXME:  Where does this go
-- ver = get_build_version()
-- top_matter = top_matter .. "<b>Hardware:        </b> " .. Platform.hw_name .. "<br>"
-- top_matter = top_matter .. "<b>Firmware Version:</b> " .. ver .. "<br><br>"

