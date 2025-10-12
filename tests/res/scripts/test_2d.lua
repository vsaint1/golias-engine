local t = require("constants")



function _ready()
    -- print("Entity Id: " .. self.id .. " Name: " .. self.name)
    local viewport = Engine:get_config():get_viewport()
    local window = Engine:get_config():get_window()
    print("Viewport: " .. viewport.width .. "x" .. viewport.height .. " Scale: " .. viewport.scale)
    print("Window: Width: " .. window.width .. " Height: " .. window.height .. " DPI Scale: " .. window.dpi_scale)
    -- Scene.change_scene("MainScene")

    local mouse_pos = Input.get_mouse_position()
    print("Mouse Position: " .. mouse_pos.x .. ", " .. mouse_pos.y)
    

end

function _process(dt)
    
    

end

function _input(event)

end

function destroy()
    print("godzilla giroflex")
end
