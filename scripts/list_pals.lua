function get_keys(t)
  local keys={}
  for key,_ in pairs(t) do
    table.insert(keys, key)
  end
  return keys
end

print("The palettes available for use are:")
print(table.concat(get_keys(palettes), ", "))
