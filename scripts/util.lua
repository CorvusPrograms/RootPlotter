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
