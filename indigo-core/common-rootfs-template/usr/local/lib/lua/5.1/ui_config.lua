-- Copyright (c) Big Switch Networks
-- ui_config.lua
-- Configuration routines for CLI and Web UI

-- General ui_ Lua conventions
--     ui_dbg_verb, error, warn, info are vectors with printf semantics;
--     See dbg_lvl_set in ui_utils.lua
--

-- Currently all config vars are written to sysenv; this could
-- be broken out to sysenv and other config vars

-- Configurations are stamped with a date stamp and stored in history

-- The following are used in ifcfg scripts:
-- switch_ip
-- gateway
-- netmask
-- dhcp_config

require "ui_utils"
require "platform"
require "parselib"

Config = Config or {}

Config.validator_to_help = {
   [parse_ip]       = "a dotted IP address",
   [parse_int]      = "an integer",
   [parse_mac]      = "a colon separated MAC address",
   [parse_yes_no]   = "yes or no",
   [parse_hex]      = "a hex integer",
   [parse_range]    = "a range low-high",
   [parse_dpid]     = "a datapath id up to 16 hex-digits",
   [parse_vid]      = "a VLAN id, -1 to 4095",
}

-- Config vars and validators
Config.known_config_vars = {
   controller_ip = {
      validator = parse_ip,
      help = "the IP address of the controller",
   },
   controller_port = {
      validator = parse_int,
      help = "the TCP port for the controller connection",
   },
   ofp_options = {
      help = "the options passed to ofprotocol",
   },
   tap0_mac = {
      validator = parse_mac,
      help = "the switch dataplane interface",
   },
   dp_mgmt = {
      validator = parse_yes_no,
      help = "to enable or disable dataplane mgmt",
   },
   dp_mgmt_oob = {
      validator = parse_yes_no,
      help = "set out-of-band dataplane mgmt mode",
   },
   dp_mgmt_port = {
      validator = parse_int,
      help = "the OpenFlow port number of the fixed port for dp mgmt",
   },
   dp_mgmt_port_fixed = {
      validator = parse_yes_no,
      help = "enable fixed management port for dp_mgmt"
   },
   dp_mgmt_vid = {
      validator = parse_vid,
      help = "the VLAN id for dp mgmt or -1 for untagged"
   },
   dp_mgmt_vid_fixed = {
      validator = parse_yes_no,
      help = "enable fixed management VLAN for dp_mgmt"
   },
   system_ref = {
      help = "of switch for some display points",
   },
   hostname = {
      help = "the hostname of the switch"
   },
   disable_sysconf = {
      validator = parse_yes_no,
      help = "disable the system config script on startup",
   },
   disable_telnet = {
      validator = parse_yes_no,
      help = "disable telnet if set to yes"
   },
   datapath_id = {
      validator = parse_dpid,
      help = "the datapath ID for the OpenFlow instance",
   },
   fail_mode = {
      validator = parse_string_set,
      validator_arg = {"open", "closed", "host", "static"},
      help = "the fail behavior for the OpenFlow protocol",
   },
   log_level = {
      validator = parse_string_set,
      validator_arg = {"debug", "info", "warn", "error", "none"},
      help = "the logging level for the system",
   },
   disable_httpd = {
      validator = parse_yes_no,
      help = "disable httpd if set to yes"
   },
   use_factory_mac = {
      validator = parse_yes_no,
      help = "use the factory MAC address if yes"
   },
}

-- Simple parsing of lines 'export key = value'
function parse_config_line(line, config)
   local start, len = string.find(line, '%s*export%s+[%a%d_]+%s*=')
   if not start or not len then
      ui_dbg_verb("config: Warning: could not parse line "..line.."\n")
      return
   end

   local value = string.sub(line, len + 1)

   local l = line:gsub('=', ' = ', 1) -- Make sure space around =
   local t = split(l, "%s+")
   if t[1] ~= "export" then
      ui_dbg_verb("config: Warning: No export for config line "..line.."\n")
      table.insert(t, 1, "export")
   end
   if t[3] ~= "=" then
      ui_dbg_verb("config: Error: No = where expected in "..line.."\n")
      return
   end
   local key = t[2]
   config[key] = trim_string(value)
   if not Config.known_config_vars[string.lower(key)] then
      ui_dbg_info("config: Info: Unknown key "..key.."\n")
   end
   ui_dbg_verb("config: Parsed config line: "..key.." = "..config[key].."\n")
end

-- Read in the current configuration
-- Add entries to the config table parameter
-- Return number of lines read or -1 on error with error string
function config_read(config, filename)
   filename = filename or Platform.cfg_filename

   local cfg_file = io.open(filename)
   local count = 0

   if not cfg_file then
      local err_str = string.format("config_read: Configuration file %s " ..
                                    "not found\n", filename)
      return -1, err_str
   end
   for line in cfg_file:lines() do
      count = count + 1
      if line ~= "" and not string.find(line, '%s*#') then
         parse_config_line(line, config)
      end
   end
   return count
end

function sfs_save()
   -- Check if SFS should be used and save to it if so
   if Platform.cfg_use_sfs and Platform.cfg_use_sfs == 1 then
      if verbose then
         rv = os.execute("(cd "..Platform.sfs_parent_dir.." && sfs_create)")
      else
         rv = os.execute("(cd "..Platform.sfs_parent_dir.." && sfs_create > /dev/null)")
      end
      if rv ~= 0 then
         err_string = string.format("config_save: Error creating SFS "..
                                    "flash file from " .. 
                                    Platform.sfs_parent_dir .. "\n")
         return -1, err_string
      end
   end
   return 0
end

-- Save current configuration
-- No sanitizing is done in this routine
function config_save(config, verbose)
   os.execute("mkdir -p "..Platform.cfg_history_dir)
   local cfg_history = Platform.cfg_history_dir .. os.date("%d%b%y-%H-%M")
   local file, msg =io.open(cfg_history, "r")
   local cfg_fname = Platform.cfg_filename
   local top_stuff = [[
#!/bin/sh
# This file contains environment variables.  It is sourced on initialization
# and is read and modified by user interface code.  Please only add export
# commands to this file.  Do not place other executable code in this file.

]]

   if file then
      ui_dbg_verb("config_save: Warning: Cfg history file %s replaced\n",
                  cfg_history)
      io.close(file)
   end

   rv = os.execute("cp " .. cfg_fname .. " " .. cfg_history)
   if rv ~= 0 then
      ui_dbg_verb("config: Warning: Could not copy %s to %s \n", cfg_fname,
                  cfg_history)
   end

   file, msg = io.open(cfg_fname, "w")
   if not file then
      err_string = string.format("Error:  Could not create %s\n", cfg_fname)
      return -1, err_string
   end

   -- Sort the variables
   local keys = get_keys_as_sorted_list(config)

   file:write(top_stuff)
   for i, k in ipairs(keys) do
      v = config[k]
      ui_dbg_verb("config: export %s=%s\n", k, tostring(v))
      file:write("export "..k.."="..tostring(v).."\n")
   end

   if config.__ADDITIONAL_CONTENTS__ then
      file:write(config.__ADDITIONAL_CONTENTS__)
   end
   io.close(file)

   return sfs_save(verbose)
end

-- Clear the configuration; move sysenv, config.bcm. rc.soc, system_* 
-- to a saved location
-- This still needs work.  It should probably keep the controller address
-- and port number and clear everything else.
function config_clear()
   os.execute("mkdir -p "..Platform.cfg_history_dir)
   local cfg_history = Platform.cfg_history_dir .. os.date("%d%b%y-%H-%M")
   local file, msg = io.open(cfg_history, "r")

   if file then
      ui_dbg_verb("config_save: Warning: Cfg history file %s replaced\n",
                  cfg_history)
      io.close(file)
      os.execute("rm -f " .. cfg_history)
   end

   os.execute("mkdir -p "..cfg_history)
   
   -- Move existing files
   rv = os.execute("mv " .. Platform.cfg_filename .. " " .. cfg_history)

   sfs_dir = Platform.sfs_parent_dir .. "/sfs/"
   other_files = {"config.bcm ", "rc.soc ", "system_init ", "system_config "}
   for i, name in pairs(other_files) do
      fname = sfs_dir .. name
      os.execute("[ -e " .. fname .. " ] && mv " .. fname .. " " .. cfg_history)
   end

   return sfs_save()

end

-- Generate /etc/ifcfg-<intf> and save to overlay
function ifcfg_create(interface, values)

   if not interface then return end

   exec_str = "/sbin/ifcfg-gen " .. interface
   exec_str = exec_str .. " " .. values.dhcp_config or "disable"
   if values.switch_ip then
      exec_str = exec_str .. " " .. values.switch_ip
      if values.netmask then
         exec_str = exec_str .. " " .. values.netmask
         if values.gateway then
            exec_str = exec_str .. " " .. values.gateway
         end
      end
   end

   os.execute(exec_str)
end
