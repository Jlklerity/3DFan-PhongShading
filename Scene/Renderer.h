#pragma once

#include <memory>
#include "Model.h"
#include "Platform.h"
#include "Scene3D.h"
#include "SceneHUD.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer {
public:
    static Renderer& Instance() { static Renderer i; return i; }
    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    void setAssetManager(AAssetManager* mgr);
#endif

    bool initializeRenderer();
    void resize(int w, int h);
    void render();
    
    void TouchEventDown(float x, float y);
    void TouchEventRelease(float x, float y);

private:
    Renderer()  = default;
    ~Renderer();

    void createModels();
    void initializeModels();
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    std::unique_ptr<Scene3D> scene3D;
    std::unique_ptr<SceneHUD> sceneHUD;
    
    int screenWidth  = 0;
    int screenHeight = 0;
};
