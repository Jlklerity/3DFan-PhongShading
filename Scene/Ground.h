#pragma once

#include "Model.h"
#include "Platform.h"
#include "Transform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Ground : public Model {

public:

#ifdef PLATFORM_ANDROID
    explicit Ground(AAssetManager* assetMgr);
#else
    Ground();
#endif

    ~Ground() override;

    void InitModel()                    override;
    void Render()                       override;
    void Resize(int w, int h)           override;
    void UpdateCamera(float distance);
    void SetLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
    
private:
    void render_ground();

#ifdef PLATFORM_ANDROID
    AAssetManager* mgr = nullptr;
#endif

    unsigned int program = 0;
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
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

    GLsizeiptr posSize   = 6 * 3 * sizeof(GLfloat);   
    GLsizeiptr normSize = 6 * 3 * sizeof(GLfloat);

    Transform transform;
};

