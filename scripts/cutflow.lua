function cutflow(hist_name, cuts, data)
   ret = { {"name", data:name()} }
   for k,v in ipairs(cuts) do
      hist = r_get_hist(data, hist_name .. v)
      table.insert(ret, { v , r_total_hist_entries(hist)})
   end
   return ret
end

function combine_to_table (...)
   ret = {}
   for i,v in ipairs{...} do
      for _, rest in pairs(v) do
         k, x = table.unpack(rest)
         if ret then
            table.insert(ret, {k , x} )
         else
            ret = {{k,x}}
         end
      end
   end
   return ret
end

function print_table_as_table(t)
   rows = {}
   for k,v in pairs(t) do
      if rows[1] then
         table.insert(rows[1], k)
      else 
         rows[1] = {k}
      end
      for i,cell in ipairs(v) do
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
