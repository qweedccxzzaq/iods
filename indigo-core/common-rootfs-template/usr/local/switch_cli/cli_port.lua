-- (c) 2010 Big Switch Networks
--
-- cli_port.lua
--

require "ui_utils"
require "ui_cs_op"
require "parselib"

port_help = [[Show or set port attributes and statistics.
port <subcmd> <p-spec> [<set keyword options>]
Examples:
    port show *                     Show port attributes
    port set 2-4,8,12 speed=1000    Set port attributes
    port stats 8,9,15               Show port statistics
]]

port_subcmds = { "set", "show", "stats"}

port_parse_tab = {
   {
      key="subcommand", 
      parser=parse_string_list, 
      pargs=port_subcmds, 
      help="Subcommand: show, set or stats"
   },
   {
      key="portspec", 
      parser=parse_port_spec,
      help="Port list like * or 1,2,4-8"
   },

   -- Flag to indicate link only
   ["-l"] = {
      key="-l", 
      help="Show only ports with link"
   },
   -- Keyword parameters for set only
   speed = {
      key="speed", 
      parser=parse_int,
      help="Port speed in Mb"
   },
   mtu = {
      key="mtu", 
      parser=parse_int, 
      help="MTU of port in bytes"
   },
   autoneg = {
      key="autoneg", 
      parser=parse_yes_no,
      help = "Enable autoneg, yes or no",
   },
   enable = {
      key="enable", 
      parser=parse_yes_no,
      pargs=true,
      help = "Port enable state"
   },
   duplex = {
      key="duplex", 
      parser=parse_int,
      pargs=true,
      help = "full duplex enabled"
   },
   tx_pause = {
      key="tx_pause", 
      parser=parse_int,
      pargs=true,
      help = "tx_pause enabled"
   },
   rx_pause = {
      key="rx_pause", 
      parser=parse_int,
      pargs=true,
      help = "rx_pause enabled"
   },
}

local attr_keys = get_keys_as_set(port_parse_tab)

function port_handler(command, line, parsed)
   if command == "ps" then
      parsed.subcommand = "show"
      if not parsed.portspec then
         parsed.portspec = parse_port_spec("*")
      end
   end

   if parsed.subcommand == "set" then
      return port_set_handler(command, line, parsed)
   end
   if parsed.subcommand == "stats" then
      return port_stats_handler(command, line, parsed)
   end

   -- Assume show
   return port_show_handler(command, line, parsed)

end

-- Also handles ps
function port_parser(command, orig_line, line, parsed)
   local completions = nil

   if not parsed.portspec then
      return 1, nil, nil, "Port specification"
   end

   if not parsed.subcommand then
      return 1, nil, nil, "Subcommand set/show/stats"
   end

   if parsed.subcommand == "set" then
      if set_card(parsed) == 2 then
         if line:find("%s$") then
            completions = attr_keys
         end
         return 1, nil, completions, "Port attribute"
      else
         completions = attr_keys
         for k,v in pairs(parsed) do
            completions[k] = nil
         end
      end
   end

   return 0, parsed, completions, ""
end

function port_show_handler(command, line, parsed)
   local port, i, status, results, err_str

   printf("Port Show.  * = Link up; D = Disabled\n")
   printf("%4s %4s %6s  %6s  %7s %7s %4s\n",
          "Port", "Link", "Speed", "MTU", "TXPause", "RXPause", "Dplx")

   for i,port in ipairs(parsed.portspec) do
      status, results, err_str = cs_port_get(port)
      report_rest_status(status, err_str)

      if results == nil or #results == 0 then
         printf("No port entries returned for %d\n", port)
      else
         entry = results[1]
         if not (parsed["-l"] and entry.link ~= 1) then
            if entry.enable ~= 1 then
               link = "D"
            else
               link = (entry.link == 1 and "*") or " "
            end
            printf("%4d %2s %8d  %6d  %4d    %4d  %4d\n",
                   port, link, entry.speed,
                   entry.mtu, entry.tx_pause,
                   entry.rx_pause, entry.duplex)
         end
      end
   end
   return 0
end

function port_set_handler(command, line, parsed)
   local port, i, status, results, err_str

   for i,port in ipairs(parsed.portspec) do
      parsed.port = port
      status, results, err_str = cs_port_set(parsed)
      report_rest_status(status, err_str)
   end

   return 0
end

function port_stats_handler(command, line, parsed)
   local port, i, status, results, err_str
   local skip_port

   printf("%5s %12s %12s %12s %12s %12s %12s %12s %12s\n",
          " ", "Unicast", "BC/MC ", " ", " ",
          "Unicast", "BC/MC "," ", " ")
   printf("%5s %12s %12s %12s %12s %12s %12s %12s %12s\n",
          "Port", "Rx-Pkts", "Rx-Pkts", "Rx-Bytes", "Rx-Errs",
          "Tx-Pkts", "Tx-Pkts", "Tx-Bytes", "Tx-Errs")

   for i,port in ipairs(parsed.portspec) do
      skip_port = false

      if parsed["-l"] then
         status, results, err_str = cs_port_get(port)
         report_rest_status(status, err_str)
         if entry.link ~= 1 then skip_port = true end
      end

      if not skip_port then
         status, results, err_str = cs_portstats_get(port)
         report_rest_status(status, err_str)

         if results == nil or #results == 0 then
            printf("No port stats returned for %d\n", port)
         end

         entry=results[1]
         printf("%5d %12d %12d %12d %12d %12d %12d %12d %12d\n",
                port, entry.rx_uc_pkts, entry.rx_bcmc_pkts, 
                entry.rx_bytes, entry.rx_errors, entry.tx_uc_pkts, 
                entry.tx_bcmc_pkts, entry.tx_bytes, entry.tx_errors)
      end
   end

   return 0
end

