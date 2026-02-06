
local MainMenu = {}

function MainMenu:Awake()
    Debug.Log("MainMenu: Awake")
    Cursor.Unlock()
    Cursor.SetEnabled(true)
end

function MainMenu:Start()
    Debug.Log("MainMenu: Start on " .. self:GetName())

    local canvas = self.gameObject:GetParent()
    if not canvas then
        Debug.LogError("MainMenu: no parent canvas!")
        return
    end

    local playObj = canvas:FindChildByName("PlayButton")
    if playObj then
        local btn = playObj:GetComponent("Button")
        if btn then
            btn.OnClick = function()
                self:PlayGame()
            end
        end
    end

    local quitObj = canvas:FindChildByName("QuitButton")
    if quitObj then
        local btn = quitObj:GetComponent("Button")
        if btn then
            btn.OnClick = function()
                self:QuitGame()
            end
        end
    end
end

function MainMenu:Update(dt)
   
end

function MainMenu:PlayGame()
    Debug.Log("MainMenu: Play clicked")
    SceneManager.LoadScene("scenes/main.gscene", true)
end

function MainMenu:QuitGame()
    Debug.Log("MainMenu: Quit clicked")
    Engine.Quit()
end

return MainMenu
