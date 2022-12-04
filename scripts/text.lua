common_subs={
   {"(%a+)_medium", "%1(Medium WP)"},
   {"pt(%d+)", "pt#geq%1"},
   {"gte" , "#geq"},
   {"_" , " - "},
   {"pt" , "p_{T}"},
   {"(%d)Lep" , "%1 Lepton"}
}


function do_subs(str)
   for _, pair in pairs(common_subs) do
      pat,rep = pair[1], pair[2]
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
