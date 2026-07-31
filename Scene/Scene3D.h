#pragma once
#include <memory>
#include "Model.h"
#include "Fan.h"
#include "Ground.h"

class Scene3D : public Model {

public:

#ifdef PLATFORM_ANDROID
    explicit Scene3D(AAssetManager* assetMgr);
#else
    Scene3D(); 
#endif

    void InitModel() override;
    void Render() override;
    void Resize(int w, int h) override;
    
    static void SetCameraDistance(float d);    // called by SceneHUD's zoom buttons
    bool PickAt(float screenX, float screenY);
    

private:
#ifdef PLATFORM_ANDROID
AAssetManager* assetMgr = nullptr;
#endif

    std::unique_ptr<Fan> fan;
    std::unique_ptr<Ground> ground;

    int screenWidth = 800, screenHeight = 600;

    GLuint depthMapFBO = 0;
    GLuint depthMap = 0;
    const unsigned int SHADOW_WIDTH = 1024;
    const unsigned int SHADOW_HEIGHT = 1024;
    //global light source for fan and ground
    inline static glm::vec3 sharedLightPos{0.0f, 5.6f, 3.0f};
    inline static glm::vec3 viewSpaceLightPos{0.0f, 0.0f, 0.0f};
    inline static glm::vec3 sharedAmbient{0.15f, 0.15f, 0.15f};
    inline static glm::vec3 sharedDiffuse{0.9f, 0.9f, 0.9f};
    inline static glm::vec3 sharedSpecular{1.0f, 1.0f, 1.0f};

    inline static float cameraDistance = 9.0f;

};