
local Request = {
    --data = nil
}
Request.__index = Request

function Request:new(...)
    local r = setmetatable({},self)
    r.data = {...}
    return r
end

function Request:prepare()
    --return data and wait(nil)
    self.data.execute = self.execute
    return self.data,self.delay
end

function Request:execute(data)
end

return Request
