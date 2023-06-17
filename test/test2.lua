function test_my_class()
    my_obj:SayHello('Alice')
print(my_obj:Add(1, 2))

end

function test_cpp_func()
    local x = 10
    local y = 20
    local z = mul(x, y)
    print("mul(" .. x .. ", " .. y .. ") = " .. z)
end
