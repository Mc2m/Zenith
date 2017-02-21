
local Request = require 'request'

local ExecuteRequest = {}
ExecuteRequest.__index = ExecuteRequest
setmetatable(ExecuteRequest, Request)

function ExecuteRequest:new(content)
    return Request.new(ExecuteRequest, content)
end

function ExecuteRequest:execute(data)
    if type(data[1]) == 'string' then
        local f = loadstring(data[1])
        table.remove(data, 1)
        if f then return f(unpack(data)) end
    elseif type(data[1]) == 'function' then
        local f = data[1]
        table.remove(data, 1)
        return f(unpack(data))
    end
end

return ExecuteRequest

