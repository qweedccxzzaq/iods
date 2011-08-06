#!/usr/bin/lua
--
-- Copyright 2011 Big Switch Networks
--
-- Usage:  clean_iods.lua <in-filename> [<out-filename>]
--
-- Removes all text in a file between lines containing
-- IODS_EXCLUDE_BEGIN and IODS_EXCLUDE_END, inclusive.
--
-- Default output file is <in-filename>.iods

start_exclude = "# *IODS_EXCLUDE_BEGIN"
end_exclude = "# *IODS_EXCLUDE_END"

if not arg[1] then
   print("Usage: Must specify input file\n");
   os.exit(1)
end

outfilename = arg[2] or arg[1] .. ".iods"

if arg[1] == "--help" then
   print("Usage: clean_iods.lua <in-filename> [<out-filename>]")
   print("Strip lines between IODS exclusion markers:")
   print("Start marker:  " .. start_exclude)
   print("End marker:    " .. end_exclude)
   print("Default output filename is <in-filename>.iods")
   os.exit(0)
end

if arg[1] == outfilename then
   print("Output file cannot equal input file")
   os.exit(1)
end

file = io.open(arg[1], "r")
if not file then
   print("Could not open file " .. arg[1])
   os.exit(1)
end

outfile = io.open(outfilename, "w")
if not outfile then
   print("Could not open output file " .. outfilename)
   io.close(file)
   os.exit(1)
end

excluding = false
while true do
   line = file:read("*line")
   if not line then
      break
   end
   if excluding then
      if string.find(line, end_exclude) then
         excluding = false
      end
   else
      if string.find(line, start_exclude) then
         excluding = true
      else
         outfile:write(line .. "\n")
      end
   end
end

io.close(file)
io.close(outfile)

if excluding then
   print("ERROR file ended in exclusion state\n")
   os.exit(1)
end
