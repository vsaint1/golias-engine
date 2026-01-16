#include "core/input/canvas_input_manager.h"

#include "core/engine.h"
#include "scene/ui/canvas_component.h"

namespace golias {

    void CanvasInputManager::SetActive(bool active) {
        isActive = active;
    }

    bool CanvasInputManager::IsActive() const {
        return isActive;
    }

    void CanvasInputManager::SetActiveCanvas(CanvasComponent* pCanvas) {
        activeCanvas  = pCanvas;
        hoveredWidget = nullptr;
        pressedWidget = nullptr;
    }

    CanvasComponent* CanvasInputManager::GetActiveCanvas() const {
        return activeCanvas;
    }

    void CanvasInputManager::Update(float deltaTime) {
        if (!isActive || !activeCanvas) {
            return;
        }

        auto& engine       = Engine::GetInstance();
        auto& inputManager = engine.GetInputManager();

        const bool mousePressed  = inputManager.IsMouseButtonJustPressed(SDL_BUTTON_LEFT);
        const bool mouseReleased = inputManager.IsMouseButtonJustReleased(SDL_BUTTON_LEFT);

        glm::vec2 pos = inputManager.GetMousePosition();
        pos.y         = engine.GetSceneRenderer().GetRenderingDevice()->GetViewport().height - pos.y;


        WidgetComponent* hitWidget = nullptr;
        auto widgets               = CollectWidgets(activeCanvas);

        for (auto it = widgets.rbegin(); it != widgets.rend(); ++it) {
            WidgetComponent* w = *it;

            if (!w) {
                continue;
            }

            if (w->HitTest(pos)) {
                hitWidget = w;
                break;
            }
        }


        if (hoveredWidget != hitWidget) {
            if (hoveredWidget) {
                hoveredWidget->OnPointerExit();
            }

            hoveredWidget = hitWidget;

            if (hoveredWidget) {
                hoveredWidget->OnPointerEnter();
            }
        }


        if (mousePressed && hoveredWidget && !pressedWidget) {
            pressedWidget = hoveredWidget;
            pressedWidget->OnPointerDown();
        }


        if (mouseReleased && pressedWidget) {
            pressedWidget->OnPointerUp();

            if (pressedWidget == hoveredWidget) {
                pressedWidget->OnClick();
            }

            pressedWidget = nullptr;
        }
    }

    std::vector<WidgetComponent*> CanvasInputManager::CollectWidgets(CanvasComponent* pCanvas) {
        std::vector<WidgetComponent*> widgets;

        if (!pCanvas) {
            return widgets;
        }

        GameObject* pCanvasObject = pCanvas->GetOwner();
        if (!pCanvasObject) {
            return widgets;
        }

        const auto& children = pCanvasObject->GetChildren();

        for (const auto& child : children) {
            if (!child) {
                continue;
            }

            if (auto* pWidget = child->GetComponent<WidgetComponent>()) {
                pCanvas->CollectWidget(pWidget, widgets);
            }
        }

        return widgets;
    }

} // namespace golias
