
Array = function(items)
    a =  string.format("[%s]", items)
    return a
end
        
Key = function(key)
    fmt = "\"%s\":"
    k = string.format(fmt, key)
    return k
end

Val = function(fmt, val)
    v = string.format(fmt, val)
    return v
end

KeyVal = function(vfmt, key, val, vfmt)
    local fmt = "\"%s\":"..vfmt or "%s"
    local kv = kv:format(fmt, key, val)
    return kv
end

Str = function(str)
    local fmt = "\"%s\""
    local s = string.format(fmt, str) 
    return s 
end

Num = function(num)
    local fmt = "%f"
    local n = string.format(fmt, num)
    return n
end

Int = function(int)
    local fmt = "%d"
    local n = string.format(fmt, int)
    return n
end

Int64 = function(int64)
    local fmt = "%d"
    local n = string.format(fmt, int64)
    return n
end

Bool = function(Bool)
    fmt = "%s"
    local b = string.format(fmt, Bool)
    return b
end



array = Array(String("test"), Number(99))

print(array)