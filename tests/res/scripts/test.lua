local t = require("constants")
local cam = nil

function _ready()
    print("Entity id:", self.id)
   
    local mouse_pos = Input.get_mouse_position()
    print("Mouse Position: " .. mouse_pos.x .. ", " .. mouse_pos.y)
   
    cam = self:get_node("MainCamera")
    if cam ~= nil then
        print("Camera found at:")
    else
        error("Camera not found!")
    end
end

function _process(dt)
    if cam == nil then return end
   
    if Input.is_key_pressed(SCANCODE_W) then
        cam:move_forward(dt)
    end
   
    if Input.is_key_pressed(SCANCODE_S) then
        cam:move_backward(dt)
    end
   
    if Input.is_key_pressed(SCANCODE_A) then
        cam:move_left(dt)
    end
   
    if Input.is_key_pressed(SCANCODE_D) then
        cam:move_right(dt)
    end
   

    if Input.is_key_pressed(SCANCODE_LSHIFT) then
        cam.speed = 15.0
    else
        cam.speed = 10.0
    end
end

function _input(event)
   
    if event.type == EVENT_MOUSE_MOTION then
        if (event.motion.state & BUTTON_MASK.LEFT) ~= 0 then
            cam:look_at(event.motion.xrel, -event.motion.yrel, 1.0)
        end
    end
   
    if event.type == EVENT_MOUSE_WHEEL then
        cam:zoom(event.wheel.y)
    end
end

function destroy()
    print("godzilla giroflex")
end