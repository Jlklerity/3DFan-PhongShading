#define LOG_TAG "Fan"
#include "Fan.h"
#include <algorithm>
#include <cmath>
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>

namespace 
{
    bool raySphereHit(const glm::vec3& d, const glm::vec3& center, float radius, float& outT)
    {
        float tca = glm::dot(center, d);         // ray origin is (0,0,0) in view space
        if (tca < 0.0f) return false;            // sphere is behind the camera
        glm::vec3 closest = d * tca;
        float distSq = glm::dot(center - closest, center - closest);
        if (distSq > radius * radius) return false;
        outT = tca;
        return true;
    }
    bool raySegmentHit(const glm::vec3& d, const glm::vec3& A, const glm::vec3& B, float radius, float& outT)
    {
        glm::vec3 segDir = B - A;
        float a = glm::dot(d, d);
        float b = glm::dot(d, segDir);
        float c = glm::dot(segDir, segDir);
        float dD = glm::dot(d, -A);
        float e = glm::dot(segDir, -A);
        float denom = a * c - b * b;

        float t, s;
        if (std::abs(denom) > 1e-6f) {
            t = (b * e - c * dD) / denom;
            s = (a * e - b * dD) / denom;
        } else {
            t = 0.0f;
            s = (c > 1e-6f) ? (e / c) : 0.0f;
        }
        t = std::max(t, 0.0f);              // ray: no hits behind the camera
        s = std::clamp(s, 0.0f, 1.0f);      // segment: clamp to [A, B]

        glm::vec3 closestOnRay = d * t;
        glm::vec3 closestOnSeg = A + segDir * s;
        float distSq = glm::dot(closestOnRay - closestOnSeg, closestOnRay - closestOnSeg);
        if (distSq > radius * radius) return false;
        outT = t;
        return true;
    }

}


static const glm::vec3 kHighlightColor = {1.00f, 0.62f, 0.10f};

static const GLfloat kPositions[24][3] = {
    // +Z face (front)
    {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
    // -Z face (back)
    { 0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f},
    // +X face (right)
    { 0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f},
    // -X face (left)
    {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f},
    // +Y face (top)
    {-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f},
    // -Y face (bottom)
    {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f},
};

static const GLfloat kNormals[24][3] = {
    {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f,  1.0f}, // +Z
    {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, // -Z
    {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f,  0.0f}, {1.0f, 0.0f,  0.0f}, // +X
    {-1.0f,0.0f,  0.0f}, {-1.0f,0.0f,  0.0f}, {-1.0f,0.0f,  0.0f}, {-1.0f,0.0f,  0.0f}, // -X
    {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f,  0.0f}, // +Y
    {0.0f,-1.0f,  0.0f}, {0.0f,-1.0f,  0.0f}, {0.0f,-1.0f,  0.0f}, {0.0f,-1.0f,  0.0f}, // -Y
};

static const GLushort kIndices[36] = {
     0, 1, 2,   2, 3, 0,   // +Z
     4, 5, 6,   6, 7, 4,   // -Z
     8, 9,10,  10,11, 8,   // +X
    12,13,14,  14,15,12,   // -X
    16,17,18,  18,19,16,   // +Y
    20,21,22,  22,23,20,   // -Y
};

static const float bladeColors[4][3] = {
    {0.8f, 0.1f, 0.1f}, // Red
    {0.1f, 0.8f, 0.1f}, // Green
    {0.1f, 0.1f, 0.8f}, // Blue
    {0.8f, 0.8f, 0.1f}  // Yellow
};


static constexpr float kBaseRadius   = 0.90f;  // scale(1.6, 0.25, 0.8)
static constexpr float kPoleRadius   = 0.15f;  
static constexpr float kHubRadius    = 0.26f;  // scale(0.3, 0.3, 0.3)
static constexpr float kBladeRadius  = 0.15f;  // blade cross-section half-thickness

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_NORMAL = 1;

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* assetMgr) : mgr(assetMgr) {assetMgr = mgr; }
#else
Fan::Fan() {}
#endif
 

Fan::~Fan()
{
    if (vao)     { glDeleteVertexArrays(1, &vao);  vao     = 0; }
    if (vbo)     { glDeleteBuffers(1, &vbo);        vbo     = 0; }
    if (ebo)     { glDeleteBuffers(1, &ebo);        ebo     = 0; }
    if (program) { glDeleteProgram(program);        program = 0; }
}

void Fan::InitModel() {
    LOGI("Fan::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/FanVertex.glsl",
        "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "FanVertex.glsl",
        "FanFragment.glsl");
#endif

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // VBO: positions sub-region then colours sub-region
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + normSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize, kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, normSize, kNormals);

    // EBO
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);

    // VAO: captures attribute layout + EBO binding
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(ATTRIB_NORMAL,   3, GL_FLOAT, GL_FALSE, 0, (void*)posSize);

    // EBO binding is recorded inside the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // Seal the VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (!program) { LOGE("Fan: failed to build shader program"); return; }
    uMVP = glGetUniformLocation(program, "ModelViewProjectionMatrix");
    uMV           = glGetUniformLocation(program, "ModelViewMatrix");
    uNormalMatrix = glGetUniformLocation(program, "NormalMatrix");
    uLightAmbt   = glGetUniformLocation(program, "LightAmbient"); 
    uLightDiff = glGetUniformLocation(program, "LightDiffuse");
    uLightSpec = glGetUniformLocation(program, "LightSpecular");
    uLightPos  = glGetUniformLocation(program, "LightPosition");
    uMatAmb    = glGetUniformLocation(program, "MaterialAmbient");
    uMatDiff   = glGetUniformLocation(program, "MaterialDiffuse");
    uMatSpec   = glGetUniformLocation(program, "MaterialSpecular");
    uPartColor = glGetUniformLocation(program, "uPartColor");
    uShiny = glGetUniformLocation(program, "ShininessFactor");
    
    glUseProgram(program);
    // Material
    if (uMatAmb    >= 0) glUniform3f(uMatAmb,   1.0f,  1.0f,  1.0f);
    if (uMatDiff   >= 0) glUniform3f(uMatDiff,  0.75f, 0.375f, 0.0f); // orange
    if (uMatSpec   >= 0) glUniform3f(uMatSpec,  1.0f,  1.0f,  1.0f);
    if (uShiny     >= 0) glUniform1f(uShiny, 24.0f);
    // Light
    if (uLightAmbt  >= 0) glUniform3f(uLightAmbt,  1.0f, 1.0f, 1.0f);
    if (uLightDiff >= 0) glUniform3f(uLightDiff, 0.8f, 0.8f, 0.8f);
    if (uLightSpec >= 0) glUniform3f(uLightSpec, 1.0f, 1.0f, 1.0f);
    glUseProgram(0);
    
    LOGI("Fan::InitModel done (VAO=%u, VBO=%u, EBO=%u)", vao, vbo, ebo);
}

// ---------------------------------------------------------------------------
// Resize
// ---------------------------------------------------------------------------
void Fan::Resize(int w, int h)
{
    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 1000.0f, 0);
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------
void Fan::Render()
{
    if (!program || !vao) return;
    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);

    
    spinAngle += currentSpeed;
    showcaseAngle += 0.25f;
    if(spinAngle >= 360.0f) spinAngle -= 360.0f;
    if (showcaseAngle >= 360.0f) showcaseAngle -= 360.0f;
    render_Fan();
    glUseProgram(0);
}

// ---------------------------------------------------------------------------
// renderFan – build MVP, bind VAO, draw
// ---------------------------------------------------------------------------
void Fan::render_Fan()
{
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // shared "world" — pull the whole fan back and tilt it, every part inherits this
    transform.TransformRotate(glm::radians(showcaseAngle), 0.0f, 1.0f, 0.0f);
    transform.TransformTranslate(0.0f, -0.3f, 0.0f);
    // ---- base ----
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -2.6f, 0.0f);
        transform.TransformScale(1.6f, 0.25f, 0.8f);
        drawPart(0.45f, 0.28f, 0.12f, PART_BASE);   // brown
    transform.TransformPopMatrix();

    // ---- pole ----
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, -1.21f, 0.0f);
        transform.TransformScale(0.15f, 2.53f, 0.15f);
        drawPart(0.55f, 0.55f, 0.58f, PART_POLE);   //light gray
    transform.TransformPopMatrix();

    // ---- hub ----
    transform.TransformPushMatrix();
        transform.TransformTranslate(0.0f, 0.2f, 0.0f);
        transform.TransformScale(0.3f, 0.3f, 0.3f);
        drawPart(0.20f, 0.20f, 0.22f, PART_HUB);   // dark gray
    transform.TransformPopMatrix();

    // 4 blades
    for (int i = 0; i < 4; ++i) {
        transform.TransformPushMatrix();
            transform.TransformTranslate(0.0f, 0.2f, 0.15f);
            transform.TransformRotate(glm::radians(spinAngle + i * 90.0f), 0.0f, 0.0f, 1.0f);
            transform.TransformTranslate(0.0f, 0.55f, 0.0f);
            transform.TransformScale(0.22f, 0.8f, 0.05f);
            drawPart(bladeColors[i][0], bladeColors[i][1], bladeColors[i][2], PART_BLADE1 + i);  
        transform.TransformPopMatrix();
    }
}

void Fan::SetSpeed(float speed) 
{ 
    currentSpeed = speed; 
}

void Fan::SetLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) const {
    if (!program) return;
    
    glUseProgram(program);

    if (uLightPos >= 0)  glUniform3fv(uLightPos, 1, glm::value_ptr(pos));
    if (uLightAmbt >= 0) glUniform3fv(uLightAmbt, 1, glm::value_ptr(ambient));
    if (uLightDiff >= 0) glUniform3fv(uLightDiff, 1, glm::value_ptr(diffuse));
    if (uLightSpec >= 0) glUniform3fv(uLightSpec, 1, glm::value_ptr(specular));
    
    glUseProgram(0);
}

void Fan::UpdateCamera(float distance)
{
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadIdentity();

    transform.TransformTranslate(0.0f, 0.8f, -distance);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
}

bool Fan::PickAt(const glm::vec3& rayDirView)
{
    int   bestPart = -1;
    float bestT     = std::numeric_limits<float>::max();

    for (int i = 0; i < PART_COUNT; ++i) {
        const PickVolume& v = pickVolumes[i];
        float t;
        bool hit = v.isSegment
                     ? raySegmentHit(rayDirView, v.segA, v.segB, v.radius, t)
                     : raySphereHit (rayDirView, v.center, v.radius, t);
        if (hit && t < bestT) { bestT = t; bestPart = i; }
    }

     if (bestPart == -1) return false;
    highlighted[bestPart] = !highlighted[bestPart];
    return true;
}

void Fan::cachePickVolume(int partId, const glm::mat4& mv)
{
    if (partId < 0 || partId >= PART_COUNT) return;
    PickVolume& v = pickVolumes[partId];

    if (partId >= PART_BLADE1) {
        v.isSegment = true;
        v.segA = glm::vec3(mv * glm::vec4(0.0f,  0.5f, 0.0f, 1.0f)); // tip
        v.segB = glm::vec3(mv * glm::vec4(0.0f, -0.5f, 0.0f, 1.0f)); // root
        v.radius = kBladeRadius;
    } else {
        v.isSegment = false;
        v.center = glm::vec3(mv[3]);
        v.radius = (partId == PART_BASE) ? kBaseRadius
                 : (partId == PART_POLE) ? kPoleRadius
                                          : kHubRadius;
    }
}

void Fan::drawPart(float r, float g, float b, int partId)
{
    glm::mat4 mvp = *transform.TransformGetModelViewProjectionMatrix();
    glm::mat4 mv  = *transform.TransformGetModelViewMatrix();
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(mv)));

    cachePickVolume(partId, mv);   // refresh this part's view-space pick volume

    if (uMVP >= 0) glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    if (uMV  >= 0) glUniformMatrix4fv(uMV,  1, GL_FALSE, glm::value_ptr(mv));
    if (uNormalMatrix >= 0) glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMat));

    glm::vec3 color = highlighted[partId] ? kHighlightColor : glm::vec3(r, g, b);
    if (uPartColor >= 0) glUniform3f(uPartColor, color.r, color.g, color.b);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
}