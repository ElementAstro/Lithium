function hello_world()
    print("Hello, world!")
end

function print_int(n)
    if type(n) ~= "number" then
        error("print_int(): argument should be a number", 2)
    end
    print("This is an integer: " .. n)
end

function print_float(n)
    if type(n) ~= "number" then
        error("print_float(): argument should be a number", 2)
    end
    print("This is a float: " .. n)
end

function print_string(str)
    if type(str) ~= "string" then
        error("print_string(): argument should be a string", 2)
    end
    print("This is a string: " .. str)
end

function eeee(str)
    print("aaa")
end
