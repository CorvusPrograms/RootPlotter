keys = {}
sources={}

for k,v in ipairs(all_data_sources) do
   table.insert(sources, v:name())
   for _, kk in pairs(v:keys()) do
      keys[kk] = true
   end
end

io.stderr:write(
   string.format("Keys for data sources %s are\n", table.concat(sources, ", "))
)

for k,_ in pairs(keys) do
   print(k)
end
