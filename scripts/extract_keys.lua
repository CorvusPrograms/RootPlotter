keys = {}
sources={}
all_data_sources = {}

for _,v in pairs(_G) do
   if type(v) == "userdata" then
      if (getmetatable(v) or {}).__name == "sol.DataSource" then
         table.insert(all_data_sources, v)
      end
    end
end



for k,v in ipairs(all_data_sources) do
   print(v:name())
end

for k,v in ipairs(all_data_sources) do
   print(v:name())
   table.insert(sources, v:name())
   
   for _, kk in pairs(v:keys()) do
      keys[kk] = true
   end
end

io.stderr:write(
   string.format("Keys for data sources %s are\n", table.concat(sources, ", "))
)

all_keys = {}

for k, _ in pairs(keys) do
   table.insert(all_keys, k)
end

table.sort(all_keys)

for _,k in pairs(all_keys) do
   print(k)
end
