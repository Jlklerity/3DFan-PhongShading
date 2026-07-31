#pragma once

#include "Model.h"
#include "Platform.h"
#include "Fan.h"
#include <glm/glm.hpp>
#include <array>
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

struct HUDRect {
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
};

class SceneHUD : public Model {
public:

#ifdef PLATFORM_ANDROID
    explicit SceneHUD(AAssetManager* assetMgr);
#else
    SceneHUD();
#endif

    ~SceneHUD()     override;

    void InitModel()            override;
    void Render()               override;
    void Resize(int w, int h)   override;
    void setStates();            
    void releaseStates();       


    bool TouchDown(float x, float y);       // HUD hit-testing — Part 5
    void TouchRelease();

    int activeButton() const { return heldButton; }
    float CurrentSpeed() const { return speed; }

private:

    void buildGeometry(int w, int h);
    void buildButtons(int button_w, int button_h);
    void buildSpeedIndicator(int speed_w, int speed_h);
    void appendQuad(float x, float y, float w, float h);

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    constexpr static int MAX_SPEED_INDICATOR_SEGMENTS { 20 }; 
    constexpr static float MAX_ZOOM_IN { 1.0f };
    constexpr static float MAX_ZOOM_OUT { 9.0f };
    constexpr static int BUTTON_COUNT { 4 };

    enum ButtonId { 
        BTN_SPD_PLUS = 0, 
        BTN_SPD_MINUS = 1, 
        BTN_ZOOM_IN = 2, 
        BTN_ZOOM_OUT = 3 
    };

    float speed = 0.0f;
    float distance = MAX_ZOOM_OUT;
    int heldButton = -1;      // which button (if any) is held

    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLint uProjection = -1;
    GLint uColor = -1;

    int screenW = 800, screenH = 600;

    glm::mat4 orthoMatrix {1.0f};

    std::vector<GLfloat> vertexData;
    std::array<HUDRect, BUTTON_COUNT> buttonRects{};
    std::array<HUDRect, MAX_SPEED_INDICATOR_SEGMENTS> segmentRects{};

    static constexpr GLsizei buttonVertexOffset  = 0;
    static constexpr GLsizei segmentVertexOffset = BUTTON_COUNT * 6;

};