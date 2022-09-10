function namegroup(match)
   match_idx={}
   idx = 1
   argmatcher = "%((%w+):(.-)%)"
   found,_, name = match:find(argmatcher)
   while found do
      match_idx[name] = idx
      idx = idx + 1
      found,_, name = match:find(argmatcher, found+1)
   end
   return match_idx, match:gsub(argmatcher, "(%2)")
end

function getnamed(s,match)
   local t,  m = namegroup(match)
   local found = {string.find(s,m)}
   found = {unpack(found,3)}
   local ret={}
   for k,v in pairs(t) do
      ret[k] = found[v]
   end
   return ret
end

function expand (s,t)
   s = string.gsub(s, "$(%w+)", function (n) return t[n] end)
   return s
end

function overwrite_table(t1,t2)
   local newtable = {}
   for k,v in pairs(t1) do
      newtable[k] = v
   end
   for k,v in pairs(t2) do
      newtable[k] = v
   end
   return newtable
end


function VLOG(level, ...)
   if VERBOSITY >= level then
      io.stderr:write(string.format(...), "\n")
   end
end

function tprint(tbl, indent)
   if not indent then indent = 0 end
   local toprint = string.rep(" ", indent) .. "{\r\n"
   indent = indent + 2 
   for k, v in pairs(tbl) do
      toprint = toprint .. string.rep(" ", indent)
      if (type(k) == "number") then
         toprint = toprint .. "[" .. k .. "] = "
      elseif (type(k) == "string") then
         toprint = toprint  .. k ..  "= "   
      end
      if (type(v) == "number") then
         toprint = toprint .. v .. ",\r\n"
      elseif (type(v) == "string") then
         toprint = toprint .. "\"" .. v .. "\",\r\n"
      elseif (type(v) == "table") then
         toprint = toprint .. tprint(v, indent + 2) .. ",\r\n"
      else
         toprint = toprint .. "\"" .. tostring(v) .. "\",\r\n"
      end
   end
   toprint = toprint .. string.rep(" ", indent-2) .. "}"
   return toprint
end

function prt(t)
   print(tprint(t))
end
