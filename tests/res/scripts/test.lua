require("constants")

local speed = 100


function _ready()
    print("Entity Id: ".. self.id .. " Name: " .. self.name)
    local viewport = Engine:get_config():get_viewport()
    local window = Engine:get_config():get_window()
    print("Viewport: " .. viewport.width .. "x" .. viewport.height .. " Scale: " .. viewport.scale)
    print("Window: Width: " .. window.width .. " Height: " .. window.height .. " DPI Scale: " .. window.dpi_scale)

    local mouse_pos = Input.get_mouse_position()
    print("Mouse Position: " .. mouse_pos.x .. ", " .. mouse_pos.y)
end


function _process(delta_time)

    if Input.is_key_pressed(SCANCODE_D) then
        self.camera:move_right(delta_time)
    end

    if Input.is_key_pressed(SCANCODE_A) then
        self.camera:move_left(delta_time)
    end

    if Input.is_key_pressed(SCANCODE_W) then
        self.camera:move_forward(delta_time)
    end

    if Input.is_key_pressed(SCANCODE_S) then
        self.camera:move_backward(delta_time)
    end
    
    if Input.is_key_pressed(SCANCODE_LSHIFT) then
        self.camera.speed = 100
    else
        self.camera.speed = 50
    end
end

function _input(event)
   if event.type == EVENT_MOUSE_MOTION then
        if (event.motion.state & BUTTON_MASK.LEFT) ~=0 then
            self.camera:look_at(event.motion.xrel, -event.motion.yrel, 1.0)
        end
    end

    if event.type == EVENT_MOUSE_WHEEL then
        self.camera:zoom(event.wheel.y)
    end
end


function destroy() 
    print("godzilla giroflex")
end
