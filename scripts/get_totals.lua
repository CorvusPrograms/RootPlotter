function extract_totals(args)
   local pattern = args[1]
   local data = args[2]
   any_normed = false
   for k,v in ipairs(data) do
      if v:normalize() then
         any_normed = true
      end
   end

   x = expand_data(data, args[1])

   for k,v in ipairs(x) do
      final = finalize_input_data(v.inputs)
      print_totals(final, any_normed)
   end
   return ret
end

function plot(args)
  table.remove(args, 1)
  extract_totals(args)
end

