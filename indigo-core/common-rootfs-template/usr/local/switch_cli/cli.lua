#!/bin/lua
-- (c) 2011 Big Switch Networks
-- Switch CLI front end

require "ui_utils"
require "global"
require "interpreter"

-- Get the next line of input
-- Handle line editing, completion, help internally
function get_line_by_char(CLI)
   repeat
      in_char, err = io.read(1)
      if err then
         return -1
      end

      if not is_eol(in_char) then
         in_char = process_char(CLI, in_char)
      end
   until is_eol(in_char)

   return 0
end

-- Add line to history, clear current line
function update_history(CLI, rv)
   if CLI.line ~= "" and CLI.history[-1] ~= CLI.line then
      table.insert(CLI.history, CLI.line)
      if #CLI.history >= CLI.history_max then
         table.remove(CLI.history, 1)
      end
   end
   CLI.line = ""
   CLI.position = 0
   CLI.history_ptr = nil
   CLI.last_rv = rv
end

--
-- Process input interactively from standard in
--

-- Run the CLI repeated on stdin/stdout
function cli_loop(CLI)
   -- Select line mode: io.read(*l) vs per-character line mode

   printf("Welcome to the Switch CLI\n")
   printf("Firmware version: \n  " .. Global.version .. "\n")

   local rv, cmd_entry, parsed, completions, str, line_handled

   while true do
      display_current_line(CLI)
      if get_line_by_char(CLI) < 0 then
         ui_dbg_error("End of input\n")
         break
      end
      io.write("\r\n")

      -- Parse and execute line
      rv = process_line(Global.command_table, CLI.line, CLI.interactive)
      if rv == 1 then break end
      update_history(CLI, rv)
   end

   printf("Bye\n")
   return 0
end

-- Run command(s) non-interactively
function cli_batch(CLI)
   local script, source
   if CLI.input_file then
      script = read_file(CLI.input_file)
      source = CLI.input_file
   elseif CLI.cmd_line_cmd then
      script = CLI.cmd_line_cmd
      source = "command line"
   else -- batch mode; read from stdin
      script = io.read("*all")
      source = "standard input"
   end

   for i,line in ipairs(split(script, "\n")) do
      ui_dbg_info("Running command: %s\n", line)
      rv, str = process_line(Global.command_table, line, false)
      if rv < 0 then
         printf("Error at line %d in %s: %s\n", i, source, str or "unknown")
         printf("Attempting to run: %s\n", line)
         return -1
      end
   end
   return 0
end

function parse_cmd_line_args(CLI, arg)
   local consume_next = false

   for i,a in ipairs(arg) do
      ui_dbg_vverb("Parsing command line arg %s\n", a)
      if not consume_next then
         if a == "-f" then
            CLI.input_file = arg[i+1]
            consume_next = true
            CLI.interactive = false
            ui_dbg_info("Running from file: %s\n", CLI.input_file)
         elseif a == "-d" then
            CLI.debug_level = get_debug_arg(arg[i+1])
            dbg_lvl_set(CLI.debug_level)
            ui_dbg_info("Debug level " .. arg[i+1])
            consume_next = true
         elseif a == "-e" then
            CLI.cmd_line_cmd = arg[i+1]
            consume_next = true
            CLI.interactive = false
            ui_dbg_info("Running from command line: %s\n", arg[i+1])
         elseif a == "-v" then -- Short hand for -d verbose
            CLI.debug_level = get_debug_arg("verbose")
            dbg_lvl_set(CLI.debug_level)
            ui_dbg_info("Debug level " .. arg[i+1])
         elseif a == "-b" then -- batch mode, non-interactive
            CLI.interactive = false
            ui_dbg_info("Running in batch mode\n")
         elseif a == "-m" then -- Force max port for debugging
            Global.port_count = tonumber(arg[i+1])
            consume_next = true
            ui_dbg_info("Set max port to %d\n", Global.port_count)
         else
            printf("Unrecognized command line arg %s\n", arg[i])
            os.exit(1)
         end
      else
         consume_next = false
      end
   end
end

------- Debug
--dbg_lvl_set(DBG_LVL_VVERB)
dbg_lvl_set(DBG_LVL_WARN)
------- 

CLI.interactive = true
Global.version = get_build_version()
parse_cmd_line_args(CLI, arg)
if not Global.port_count then Global.port_count = port_count_get() end
interpreter_init()

-- TODO Remove this debug check
str = validate_command_table(Global.command_table)
if str then
   printf("Internal error. Bad command table: %s\n", str)
   os.exit(-1)
end

if CLI.interactive then
   rv = cli_loop(CLI)
else
   rv = cli_batch(CLI)
end

os.exit(rv)
