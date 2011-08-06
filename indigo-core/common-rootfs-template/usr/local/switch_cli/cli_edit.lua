-- (c) 2010 Big Switch Networks
-- Line editing functions

function erase_line(CLI)
   local prompt = get_full_prompt(CLI)
   printf("\r%s\r", string.rep(" ", #CLI.line + #prompt))
end

-- Generate the prompt
function get_full_prompt(CLI)
   local prompt = CLI.prompt
   if CLI.enabled then
      prompt = prompt .. "# " 
   else
      prompt = prompt .. ": " 
   end
   return prompt
end

-- Redisplay the current line of input, no new line
function display_current_line(CLI)
   local line = CLI.line
   local prompt = get_full_prompt(CLI)
   local posn = CLI.position
   erase_line(CLI)
   printf("\r" .. prompt .. line)
   printf("\r" .. prompt .. string.sub(line, 1, posn))
end

-- Generate completions for the current position
function completion(CLI, in_char)
   local rv, cmd_entry, parsed, completions, str, prefix

   -- TODO Support completions in middle of line
   CLI.position = #CLI.line

   -- Parse as normal; check completions
   rv, cmd_entry, parsed, completions, str = 
      parse_line(CLI.line, Global.command_table)

   if rv == -1 then
      printf("\nError parsing input: " .. str .. "\n")
      return
   end

   if rv == 1 and completions == nil then
      if not CLI.line:find("%s$") then
         CLI.line = CLI.line .. " "
         CLI.position = #CLI.line
      end
      return
   end

   if completions and set_card(completions) > 0 then
      prefix = longest_prefix_set(completions)
      if prefix ~= "" then
         CLI.line = extend_last_token(CLI.line, prefix)
         if set_card(completions) == 1 then
            CLI.line = CLI.line .. " "
         end
         CLI.position = #CLI.line
      end
      if set_card(completions) > 1 then
         io.write("\n\rPossible completions:\n\r")
         for k,v in pairs(completions) do
            io.write(k .. "\n\r")
         end
      end
   end

   if not completions or set_card(completions) ~= 1 then
      if str and str ~= "" then
         io.write("\n\r" .. str .. "\n\r")
      end
   end
end

-- Go to end of line
function start_of_line(CLI, in_char)
   CLI.position = 0
end

-- Go to end of line
function goto_eol(CLI, in_char)
   CLI.position = #CLI.line
   io.write("\n\r")
end

function control_c(CLI, in_char)
   -- for now exit
   printf("\nControl C\n")
   os.exit(1)
end

-- Move back and delete character
function backspace(CLI, in_char)
   erase_line(CLI)
   if CLI.position > 0 then
      CLI.position = CLI.position - 1
      delete_current_char(CLI)
   end
   display_current_line(CLI)
end

function quit_char(CLI, in_char)
   if #CLI.line == 0 then
      CLI.line = "quit"
      return string.char(10)
   end
   -- ignore otherwise
end

-- Delete the current character
-- Note that this is at position + 1
function delete_current_char(CLI)
   if #CLI.line > 0 then
      start = string.sub(CLI.line, 1, CLI.position)
      the_rest = string.sub(CLI.line, CLI.position + 2)
      CLI.line = start..the_rest
   end
end

-- Swap to a newer line
function newer_line(CLI, in_char)
   if CLI.history_ptr == nil then return end
   if CLI.history_ptr >= #CLI.history then
      erase_line(CLI)
      CLI.line = ""
      CLI.position = 0
      return
   end
   CLI.history_ptr = CLI.history_ptr + 1

   erase_line(CLI)
   CLI.line = CLI.history[CLI.history_ptr]
   CLI.position = #CLI.line
end

-- Swap to an older line
-- To do:  Save current line and allow it to be restored
function older_line(CLI, in_char)
   if CLI.history_ptr == nil then
      CLI.history_ptr = #CLI.history
   elseif CLI.history_ptr > 0 then
      CLI.history_ptr = CLI.history_ptr - 1
   end

   if CLI.history_ptr > 0 then
      erase_line(CLI)
      CLI.line = CLI.history[CLI.history_ptr]
      CLI.position = #CLI.line
   end
end

-- Handle an escape sequence
function escape_start(CLI, in_char)
   next_key = io.read(1)
   if next_key ~= string.char(91) then return end
   next_key = io.read(1)
   if next_key == 'A' then
      older_line(CLI, next_key)
   elseif next_key == 'B' then
      newer_line(CLI, next_key)
   elseif next_key == 'C' then
      if CLI.position < #CLI.line then
         CLI.position = CLI.position + 1
      end
   elseif next_key == 'D' then
      if CLI.position > 0 then
         CLI.position = CLI.position - 1
      end
   end
   return
end

-- Any non-control character is accepted for input
function token_char(in_char)
   if not in_char then return false end
   if string.find(in_char, "%c") then return false end
   return true
end

function control_u(CLI, in_char)
   erase_line(CLI)
   CLI.line = ""
   CLI.position = 0
end

function control_k(CLI, in_char)
   local diff = #CLI.line - CLI.position
   CLI.line = string.sub(CLI.line, 1, CLI.position)
   io.write(string.rep(" ", diff))
end
   
-- From key entry to action
action_key_map = {
   ['?'] = show_help,
   ['\t'] = completion,
   [string.char(1)] = start_of_line,
   [string.char(3)] = control_c,
   [string.char(4)] = quit_char,
   [string.char(5)] = goto_eol,
   [string.char(8)] = backspace,
   [string.char(11)] = control_k, -- kill rest of line
   [string.char(127)] = backspace,
   [string.char(21)] = control_u, -- kill line
   [string.char(27)] = escape_start,
}

-- Insert a character into a string
function insert_char(line, position, char)
   start_line = string.sub(line, 1, position - 1)
   end_line = string.sub(line, position)
   return start_line .. char .. end_line
end

-- Process the next input character   
function process_char(CLI, in_char)
   -- assert CLI.interactive
   -- Treat no character as ^D
   if not in_char then in_char = string.char(4) end

   if action_key_map[in_char] then
      new_in_char = action_key_map[in_char](CLI, in_char)
      in_char = new_in_char or in_char
   elseif token_char(in_char) or in_char == ' ' then
      CLI.position = CLI.position + 1
      CLI.line = insert_char(CLI.line, CLI.position, in_char)
   else
      ui_dbg_verb("\nUnhandled char: code: %s\n", string.byte(in_char))
   end
   display_current_line(CLI)
   return in_char
end

function is_eol(in_char)
   return in_char == string.char(10) or in_char == string.char(13)
end
