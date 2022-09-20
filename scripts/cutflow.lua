function cutflow(hist_name, cuts, data, divide, separator)
   separator = separator or "_"
   ret = { {"name", data:name()} }
   local first = true
   local first_value
   local prev_name = hist_name
   local next_name
   for k,v in ipairs(cuts) do
      next_name = prev_name .. ( v ~= "" and separator  or "" ) .. v
      hist = r_get_hist(data, next_name)
      prev_name = next_name
      val = r_total_hist_entries(hist)
      first_value = first and val or first_value
      if divide then
         val = val / first_value
      end
      first = first and false
      table.insert(ret, { v , val})
   end
   return ret
end

function insert_find(t, k , v)
   found = false
   for _, tab in ipairs(t) do
      if tab.key == k  then
         table.insert(tab.value,v)
         found = true
      end
   end
   if not found then
      table.insert(t,{key=k,value={v}})
   end

end

function combine_to_table (...)
   ret = {}
   for i,v in ipairs{...} do
      for _, rest in pairs(v) do
         k, x = table.unpack(rest)
         insert_find(ret, k,x)
      end
   end
   return ret
end

function print_cols_as_rows(t)
   rows = {}
   for k,v in pairs(t) do
      if rows[1] then
         table.insert(rows[1], v.key)
      else 
         rows[1] = {v.key}
      end
      for i,cell in ipairs(v.value) do
         if rows[i+1] then
            table.insert(rows[i+1], cell )
         else 
            rows[i+1] = {cell}
         end
      end
   end
   for i,row in ipairs(rows) do
      io.stdout:write(table.concat(row, "\t"), "\n")
   end
end
