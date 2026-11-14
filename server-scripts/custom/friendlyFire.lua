-- Apply the server's friendly-fire policy to each player after authentication.
-- Configure config.friendlyFire in CoreScripts/scripts/config.lua.

local customEventHooks = require("customEventHooks")

customEventHooks.registerHandler("OnPlayerAuthentified", function(eventStatus, pid)
    local friendlyFireEnabled = config.friendlyFire ~= false
    tes3mp.SetGameSettingValue(pid, "friendly fire", tostring(friendlyFireEnabled))
    tes3mp.SendSettings(pid, false, false)
end)

return {}
