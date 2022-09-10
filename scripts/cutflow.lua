

function cutflow(hist_name, cuts, data)
   ret = { {"name", data:name()} }
   for k,v in ipairs(cuts) do
      hist = r_get_hist(data, hist_name .. v)
      table.insert(ret, { v , r_total_hist_entries(hist)})
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
