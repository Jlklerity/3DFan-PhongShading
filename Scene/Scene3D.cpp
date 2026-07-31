#define LOG_TAG "Scene3D"
#include "Scene3D.h"
#include "Platform.h"
#include <glm/gtc/matrix_transform.hpp>


#ifdef PLATFORM_ANDROID
Scene3D::Scene3D(AAssetManager* assetMgr) {
    fan = std::make_unique<Fan>(assetMgr);
    ground = std::make_unique<Ground>(assetMgr);
}
#else
Scene3D::Scene3D() 
{
    fan = std::make_unique<Fan>();
    ground = std::make_unique<Ground>();
}
#endif

void Scene3D::InitModel() 
{
    if (!fan) return;
    if (!ground) return;

    ground->InitModel();
    fan->InitModel();
    
    ground->SetLight(viewSpaceLightPos, sharedAmbient, sharedDiffuse, sharedSpecular);
    fan->SetLight(viewSpaceLightPos, sharedAmbient, sharedDiffuse, sharedSpecular);
}

void Scene3D::Render()
{
    if (fan) fan->Render();
    if (ground) ground->Render();
    
    if (fan)    fan->UpdateCamera(cameraDistance);
    if (ground) ground->UpdateCamera(cameraDistance);
    
    
}

void Scene3D::Resize(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    
    if (fan) fan->Resize(w, h);
    if (ground) ground->Resize(w, h);
}

void Scene3D::SetCameraDistance(float d) 
{
    cameraDistance = d;
}

bool Scene3D::PickAt(float screenX, float screenY)
{
    if (screenHeight <= 0 || !fan) return false;

    // Same perspective Fan::Resize()/Ground::Resize() already build — kept
    // local here since Scene3D doesn't currently own a shared projection.
    float aspect = (float)screenWidth / (float)screenHeight;
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f);
    glm::mat4 invProj = glm::inverse(proj);

    float ndcX = (2.0f * screenX / (float)screenWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / (float)screenHeight); // top-left px -> NDC (y-up)

    glm::vec4 viewSpaceNear = invProj * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    viewSpaceNear /= viewSpaceNear.w;

    // Camera sits at the view-space origin, so the ray direction is just the
    // unprojected near-plane point, normalized.
    glm::vec3 rayDirView = glm::normalize(glm::vec3(viewSpaceNear));

    return fan->PickAt(rayDirView);
}





