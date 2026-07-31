#pragma once

#include "Model.h"
#include "Platform.h"
#include "Transform.h"
#include <array>
#include <limits>
#include <chrono>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Fan : public Model{
public:

#ifdef PLATFORM_ANDROID
    explicit Fan(AAssetManager* assetMgr);
#else
    Fan();
#endif

    ~Fan() override;

    void InitModel()                override;
    void Render()                   override;
    void Resize(int w, int h)       override;
    static void SetSpeed(float speed); 
    void SetLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) const;
    void UpdateCamera(float distance);
    bool PickAt(const glm::vec3& rayDirView);
    bool IsPartHighlighted(int partId) const { return partId >= 0 && partId < PART_COUNT && highlighted[partId]; }
        
private:
    void render_Fan();
    void drawPart(float r, float g, float b, int partId);
    void cachePickVolume(int partId, const glm::mat4& mv);

    enum partId {
        PART_BASE = 0,
        PART_POLE,
        PART_HUB,
        PART_BLADE1, PART_BLADE2, PART_BLADE3, PART_BLADE4,
        PART_COUNT
    };

    struct PickVolume {
        bool        isSegment = false;
        glm::vec3   center{0.0f};
        float       radius = 0.0f;
        glm::vec3   segA{0.0f}, segB{0.0f};
    };

    std::array<PickVolume, PART_COUNT> pickVolumes{};
    std::array<bool, PART_COUNT> highlighted{};

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif
    GLuint program = 0;
    GLuint vao     = 0;
    GLuint vbo     = 0;
    GLuint ebo     = 0;
    GLint uMVP    = -1;
    GLint uMV      = -1;
    GLint uNormalMatrix = -1;
    GLint uMatAmb    = -1;
    GLint uMatDiff   = -1;
    GLint uMatSpec   = -1;
    GLint uLightAmbt = -1;
    GLint uLightDiff = -1;
    GLint uLightSpec = -1;
    GLint uLightPos = -1;
    GLint uPartColor = -1;
    GLint uShiny     = -1;

    // VBO sub-region sizes
    GLsizeiptr posSize   = 24 * 3 * sizeof(GLfloat);   
    GLsizeiptr normSize = 24 * 3 * sizeof(GLfloat);

    float spinAngle = 0.0f;                             // current blade angle (degrees)
    float showcaseAngle = 0.0f;
    inline static float currentSpeed = 0.0f;

    Transform transform;                                // provided matrix stack (model/view/projection)
};





