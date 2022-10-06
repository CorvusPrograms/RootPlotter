common_subs={
   [ "gte" ]="#gte",
   [ "pt" ]="p_{T}",
   [ "_" ]=" "
}


function do_subs(str)
   for pat, rep in pairs(common_subs) do
      str=str:gsub(pat, rep)
   end
   return str
end


function do_subs_table(tbl)
   tbl.xlabel = tbl.xlabel and do_subs(tbl.xlabel) or nil 
   tbl.ylabel = tbl.ylabel and do_subs(tbl.ylabel) or nil 
   tbl.title = tbl.title and do_subs(tbl.title) or nil 
   return tbl
end
