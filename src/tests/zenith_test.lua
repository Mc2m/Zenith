
local RequestManager = require('RequestManager')
local Request = require('Request')
local RequestExecute = require('Request_execute')

local r = RequestExecute:new("function test() test = 'bleh' end test()")

RequestManager:send("Test pipe",r)
