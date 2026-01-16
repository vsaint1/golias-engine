#pragma once
#include <vector>

namespace golias {
    class CanvasComponent;
    class WidgetComponent;

    class CanvasInputManager {
    public:
        virtual ~CanvasInputManager() = default;

        void SetActive(bool active);
        bool IsActive() const;

        void Update(float deltaTime);

        void SetActiveCanvas(CanvasComponent* pCanvas);
        CanvasComponent* GetActiveCanvas() const;
        
    
        std::vector<WidgetComponent*> CollectWidgets(CanvasComponent* pCanvas);

    private:
        bool isActive = false;
        CanvasComponent* activeCanvas = nullptr;

        WidgetComponent* hoveredWidget = nullptr;
        WidgetComponent* pressedWidget = nullptr;
    };
} // namespace golias
