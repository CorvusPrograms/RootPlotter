x = DataSource:new("test")
y = DataSource:new("test")

s1 = SourceSet({x,y});


function plot(program, ...)
   x = program(...)
   print(x)
end
--
-- function ratioplot(data, ...)
--    split = Split:new(2,1)
--    split[0]:assign(simple, data=data data=Get xlabel="Hello", ylabel="Hello")
--    split[1]:assign(ratio, data={data[0],data[1]} , xlabel="Hello", ylabel="Hello")
--    return split
-- end





plot{ratioplot, hname="WPt_Lep", num="tag1", den=Stack("test2"), ylabel="Hello", legend=true}
plot{ratioplot,  num="tag1", den=Stack("test2")}
plot{simple, name="WPt", data=Data{Normalize("tag2", Integral(Stack("tag1"))), Stack("tag2")}, xlabel="Test"}
plot{simple, data=Data{Normalize("tag2", 1), Stack("tag2"), normalize_to=1}, xlabel="Test"}
