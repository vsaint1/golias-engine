local SDL = require("constants")

local speed = 100


function ready()
    print("Entity Id: ".. self.id .. " Name: " .. self.name)
    local viewport = Engine:get_config():get_viewport()
    local window = Engine:get_config():get_window()
    print("Viewport: " .. viewport.width .. "x" .. viewport.height .. " Scale: " .. viewport.scale)
    print("Window: Width: " .. window.width .. " Height: " .. window.height .. " DPI Scale: " .. window.dpi_scale)
    -- Scene.change_scene("GameScene")

    local mouse_pos = Input.get_mouse_position()
    print("Mouse Position: " .. mouse_pos.x .. ", " .. mouse_pos.y)
end

function update(delta_time)

    if Input.is_key_pressed(SDL_SCANCODE_D) then
        self.transform.position.x = self.transform.position.x + speed * delta_time
    end

    if Input.is_key_pressed(SDL_SCANCODE_A) then
        self.transform.position.x = self.transform.position.x - speed * delta_time
    end

    if Input.is_key_pressed(SDL_SCANCODE_W) then
        self.transform.position.y = self.transform.position.y - speed * delta_time
    end

    if Input.is_key_pressed(SDL_SCANCODE_S) then
        self.transform.position.y = self.transform.position.y + speed * delta_time
    end
end

function destroy() 
    print("godzilla giroflex")
end
