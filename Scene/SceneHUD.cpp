#define LOG_TAG "SceneHUD"
#include "SceneHUD.h"
#include "Scene3D.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

#ifdef PLATFORM_ANDROID
SceneHUD::SceneHUD(AAssetManager* mgr) : assetMgr(mgr) {}
#else
SceneHUD::SceneHUD() {}
#endif

SceneHUD::~SceneHUD()
{
    if (vao)     glDeleteVertexArrays(1, &vao);
    if (vbo)     glDeleteBuffers(1, &vbo);
    if (program) glDeleteProgram(program);
}

void SceneHUD::InitModel() {
    LOGI("SceneHUD::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetMgr,
        "shader/HUDVertex.glsl",
        "shader/HUDFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "HUDVertex.glsl",
        "HUDFragment.glsl");
#endif
    
    if (!program) { LOGE("SceneHUD: failed to build shader program"); return; }

    uProjection = glGetUniformLocation(program, "PROJECTION");
    uColor      = glGetUniformLocation(program, "Color");

    // Safe default projection in case Render() ever precedes Resize().
    orthoMatrix = glm::ortho(0.0f, (float)screenW, (float)screenH, 0.0f, -1.0f, 1.0f);

    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
 
    // Seal the VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
 
    buildGeometry(screenW, screenH);

    LOGI("SceneHUD::InitModel done (VAO=%u, VBO=%u)", vao, vbo);

}

// ---------------------------------------------------------------------------
// Render state: the HUD draws OVER the 3D scene - depth test off - and its
// idle buttons are translucent, so blending is on. releaseStates() undoes
// both, honouring the state-ownership contract from tutorial 2.
// ---------------------------------------------------------------------------
void SceneHUD::setStates()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SceneHUD::releaseStates()
{
    glDisable(GL_BLEND);
}

void SceneHUD::Render() 
{
    if (!program || !vao) return; 
    setStates();

    glUseProgram(program);
    if (uProjection >= 0)
        glUniformMatrix4fv(uProjection, 1, GL_FALSE, glm::value_ptr(orthoMatrix));

    glBindVertexArray(vao);

    // --- 4 control buttons: +SPD / -SPD / +ZOOM / -ZOOM ---
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        if (uColor >= 0) {
            if (i == heldButton) glUniform4f(uColor, 1.00f, 0.62f, 0.10f, 0.95f); // held: amber
            else                 glUniform4f(uColor, 1.00f, 1.00f, 1.00f, 0.40f); // idle: ghost white
        }
        glDrawArrays(GL_TRIANGLES, buttonVertexOffset + i * 6, 6);
    }

    // --- speed indicator: 20 segments stitched edge-to-edge into one bar ---
    int filled = std::clamp((int)std::lround(speed), 0, MAX_SPEED_INDICATOR_SEGMENTS);
    for (int i = 0; i < MAX_SPEED_INDICATOR_SEGMENTS; ++i) {
        if (uColor >= 0) {
            if (i < filled) glUniform4f(uColor, 0.f, 1.f, 0.f, 1.f); // filled: green
            else            glUniform4f(uColor, 0.1f, 0.1f, 0.1f, 1.f); // empty
        }
        glDrawArrays(GL_TRIANGLES, segmentVertexOffset + i * 6, 6);
    }

    glBindVertexArray(0);
    glUseProgram(0);

    releaseStates();
}

void SceneHUD::Resize(int w, int h) 
{
    if (w <= 0 || h <= 0) return;

    // Pixel-space projection with the origin at the TOP-left and y growing
    // downward - the same space the platform reports touches in. Built
    // directly with glm::ortho: one unit = one pixel.
    orthoMatrix = glm::ortho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);

    buildGeometry(w, h);
}

void SceneHUD::buildGeometry(int w, int h)
{
    screenW = w;
    screenH = h;
    vertexData.clear();
    vertexData.reserve((BUTTON_COUNT + MAX_SPEED_INDICATOR_SEGMENTS) * 6 * 2);

    int button_w = static_cast<int>(w * 0.14f);
    int button_h = static_cast<int>(h * 0.075f);
    buildButtons(button_w, button_h);

    int speed_w = static_cast<int>(w * 0.5f);  //total bar width
    int speed_h = static_cast<int>(w * 0.035f);     //segment height
    buildSpeedIndicator(speed_w, speed_h);

    if(!vbo) return;  // geometry requested before InitModel() ran; bail safely

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneHUD::appendQuad(float x, float y, float w, float h)
{
    const GLfloat quad[12] = {
        x,   y,
        x,   y+h,
        x+w, y,

        x+w, y,
        x,   y+h,
        x+w, y+h
    };
    vertexData.insert(vertexData.end(), std::begin(quad), std::end(quad));
}

void SceneHUD::buildButtons(int button_w, int button_h)
{
    const float margin  = screenW * 0.04f;
    const float rowGap  = button_h * 0.6f;
    const float topOffset = screenH * 0.18f;

    const float leftx = margin;
    const float rightx = screenW - margin - button_w;

    const HUDRect rects[BUTTON_COUNT] = {
        { leftx,  topOffset,                       (float)button_w, (float)button_h }, // BTN_SPD_PLUS
        { leftx,  topOffset + button_h + rowGap,   (float)button_w, (float)button_h }, // BTN_SPD_MINUS
        { rightx, topOffset,                       (float)button_w, (float)button_h }, // BTN_ZOOM_PLUS
        { rightx, topOffset + button_h + rowGap,   (float)button_w, (float)button_h }, // BTN_ZOOM_MINUS
    };

    for (int i = 0; i < BUTTON_COUNT; ++i)
    {
        buttonRects[i] = rects[i];
        appendQuad(rects[i].x, rects[i].y, rects[i].w, rects[i].h);
    }
}

void SceneHUD::buildSpeedIndicator(int speed_w, int speed_h)
{
    const float barX = (screenW - speed_w) * 0.5f;
    const float barY = screenH * 0.85f;
    const float segW = (float)speed_w / MAX_SPEED_INDICATOR_SEGMENTS;

    for(int i = 0; i < MAX_SPEED_INDICATOR_SEGMENTS; ++i){
        HUDRect r{ barX + i * segW, barY, segW, (float)speed_h };
        segmentRects[i] = r;
        appendQuad(r.x, r.y, r.w, r.h);
    }
}

bool SceneHUD::TouchDown(float x, float y) 
{
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        const HUDRect& r = buttonRects[i];
        if (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h) {
            heldButton = i;
            switch (i) {
                case BTN_SPD_PLUS:  
                    speed = std::min(speed + 1.0f, (float)MAX_SPEED_INDICATOR_SEGMENTS); 
                    Fan::SetSpeed(speed);
                    break;
                case BTN_SPD_MINUS: 
                    speed = std::max(speed - 1.0f, 0.0f); 
                    Fan::SetSpeed(speed); 
                    break;
                case BTN_ZOOM_IN:
                    distance = std::max(distance - 1.0f, MAX_ZOOM_IN);
                    Scene3D::SetCameraDistance(distance);
                    break;
                case BTN_ZOOM_OUT:
                    distance = std::min(distance + 1.0f, MAX_ZOOM_OUT);
                    Scene3D::SetCameraDistance(distance);
                    break;
                default: 
                    break; // BTN_ZOOM_PLUS / BTN_ZOOM_MINUS: wire into Scene3D::SetCameraDistance via Renderer
            }
            return true;
        }
    }
    return false;
}

void SceneHUD::TouchRelease() 
{
    heldButton = -1; // Reset whichever button was currently being held down
}