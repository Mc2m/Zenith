
local Request = require 'Request'

local ExecuteRequest = {
}
ExecuteRequest.__index = ExecuteRequest
setmetatable(ExecuteRequest,Request)

function ExecuteRequest:new(content)
    return Request.new(ExecuteRequest,content)
end

function ExecuteRequest:execute(data)
    local f = loadstring(data[1])
    table.remove(data,1)
    if type(f) == 'function' then return f(unpack(data)) end
end

return ExecuteRequest

