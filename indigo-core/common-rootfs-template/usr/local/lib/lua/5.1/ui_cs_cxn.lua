-- Copyright (c) 2010 Big Switch Networks
-- ui_cs_cxn.lua
-- Functions supporting connections to the command server

require "socket"
require "ui_utils"

-- These are hard coded for now
CMDSRV_HOST = '127.0.0.1'
CMDSRV_PORT = 8088

-- Read undelineated data from a socket as currently available
function read(soc, pattern, prefix)
   local value, status, partial = soc:receive(pattern, prefix)
   if value then
      return value, status
   end
   if partial and #partial > 0 then
      return partial, status
   end
   return nil, status
end

-- Send a request and accumulate all replies
-- Normally return (response, bytes)
-- On an error, return nil for response on error with -1 bytes 
-- and an error string
function transact(buf, host, port)
   local bytes = 0
   local resp = ""
   local s_soc

   host = host or CMDSRV_HOST
   port = port or CMDSRV_PORT

   s_soc = socket.connect(host, port)

   if not s_soc then
      t = split(string.sub(buf, 1, 50), " ")
      err_str = string.format("transact ERROR: Could not connect to " ..
                              "server; op: %s. uri: %s id %s\n",
                           tostring(t[1]), tostring(t[2]), tostring(t[3]))
      return nil, -1, err_str
   end
   s_soc:send(buf)
   ui_dbg_verb("transact: wrote %d bytes: %s\n", #buf, buf)
   -- Accumulate response
   while true do
      val, status = read(s_soc)
      data = val or partial
      if data ~= nil then
         resp = resp .. data
         bytes = bytes + #data
      end
      if status == "closed" then break end
   end

   s_soc:close()
   ui_dbg_verb("transact: got %d bytes: %s\n", bytes, resp)
   return resp, bytes
end
