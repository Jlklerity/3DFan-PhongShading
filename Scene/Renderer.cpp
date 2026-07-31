#define LOG_TAG "Renderer"
#include "Renderer.h"

Renderer::~Renderer() { clearModels(); }

#ifdef PLATFORM_ANDROID
void Renderer::setAssetManager(AAssetManager* mgr) { assetMgr = mgr; }
#endif

bool Renderer::initializeRenderer()
{
    LOGI("Renderer::initializeRenderer");
    createModels();
    initializeModels();
    return true;
}

void Renderer::createModels()
{
#ifdef PLATFORM_ANDROID
    scene3D = std::make_unique<Scene3D>(assetMgr);
    sceneHUD = std::make_unique<SceneHUD>(assetMgr);
#else
    scene3D = std::make_unique<Scene3D>();      //make Scene3D that renders Fan and Ground
    sceneHUD = std::make_unique<SceneHUD>();    //make SceneHUD that renders HUD
#endif

}

void Renderer::initializeModels()  
{ 
    if (scene3D) scene3D->InitModel();
    if (sceneHUD) sceneHUD->InitModel(); 
}

void Renderer::clearModels()     
{   
    scene3D.reset(); 
    sceneHUD.reset();
}

void Renderer::resize(int w, int h)
{
    screenWidth = w; screenHeight = h;
    glViewport(0, 0, w, h);

    if (scene3D) scene3D->Resize(w, h);
    if (sceneHUD) sceneHUD->Resize(w, h);

    LOGI("Renderer::resize %d x %d", w, h);
}

void Renderer::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     //clear framebuffer
    glEnable(GL_DEPTH_TEST);                                //Render Scene3D (depth test ON)
    if (scene3D) scene3D->Render();
    glDisable(GL_DEPTH_TEST);                               //Disable Depth Test
    if (sceneHUD) sceneHUD->Render();                       //Render SceneHUD (drawn on top of everything)
    glEnable(GL_DEPTH_TEST);                                //Enable Depth Test (restore state for next frame)
}

void Renderer::TouchEventDown(float x, float y)    
{ 
    if (sceneHUD && sceneHUD->TouchDown(x,y)) { return; }
    if (scene3D) { scene3D->PickAt(x,y); }
}
void Renderer::TouchEventRelease([[maybe_unused]] float x, [[maybe_unused]] float y)  //[[maybe_unused] allowed in c++17 above
{
    if (sceneHUD) sceneHUD->TouchRelease();    
}

 

