
local Pipes = Zenith.Pipe.Pipes

local RequestManager = {
    requests = {}
}

function RequestManager:send(pipeName, request)
    -- fetch the pipe
    local pipe = Pipes[pipeName]

    if pipe then
        local data,wait = request:prepare()

        if data and type(data.execute) == "function" then
            return pipe:send(wait,data)
        end
    end
end

function RequestManager:receive()
    -- check all pipes
    for name,pipe in pairs(Pipes) do
        local data = nil

        -- receive from the pipe
        data = pipe:receive(0,0)

        if data then
            local request = self.requests[name]
            if request then
                -- we still have a request running for that pipe
                -- resume it with the data on the pipe
                self.requests[name] = nil
                coroutine.resume(request)

                if coroutine.status(result) == 'dead' then
                    self.requests[name] = nil
                end

            elseif data.execute then
                local result
                if coroutine then
                    -- create request in coroutine
                    request = coroutine.create(data.execute)
                    result = {coroutine.resume(request, nil, data)}
                    if coroutine.status(request) ~= 'dead' then
                        self.requests[name] = request
                    end

                    if #result > 1 then
						table.remove(result,1)
					else
						result = nil
					end
                else
                    -- execute it
                    result = {data:execute()}
                end

                if result then
                    -- send back the result
                    pipe:send(0,unpack(result))
                end
            end
        end
    end
end

return RequestManager
