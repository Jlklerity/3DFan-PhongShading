#define LOG_TAG "Ground"
#include "Ground.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>

static const GLfloat kPositions[6][3] = {
    {-20.0f, -0.5f, -20.0f}, // Triangle 1
    {-20.0f, -0.5f,  20.0f},
    { 20.0f, -0.5f,  20.0f},
    
    { 20.0f, -0.5f,  20.0f}, // Triangle 2
    { 20.0f, -0.5f, -20.0f},
    {-20.0f, -0.5f, -20.0f}

};

static const GLfloat kNormals[6][3] = {
    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},

    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 1.0f, 0.0f}
};

static constexpr GLuint ATTRIB_POSITION = 0;
static constexpr GLuint ATTRIB_NORMAL = 1;

#ifdef PLATFORM_ANDROID
Ground::Ground(AAssetManager* assetMgr) : mgr(assetMgr) {}
#else
Ground::Ground() {
}
#endif

Ground::~Ground()
{
    if (vao)  glDeleteVertexArrays(1, &vao);
    if (vbo)  glDeleteBuffers(1, &vbo);
    if (program) glDeleteProgram(program);
}
void Ground::InitModel() {
    // Empty for Part 1!
    
    // In Part 3, you will:
    // 1. Define an array of vertices for ONE large flat quad (two triangles) 
    //    lying flat on the XZ plane (e.g., y = -0.5 or 0.0).
    // 2. Generate and bind your VAO and VBO for that quad.
    // 3. Load and compile your new GroundVertex.glsl and GroundFragment.glsl shaders.
    LOGI("Ground::InitModel");

#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        mgr,
        "shader/GroundVertex.glsl",
        "shader/GroundFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "GroundVertex.glsl",
        "GroundFragment.glsl");
#endif

    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    // VBO: positions sub-region then colours sub-region
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, posSize + normSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,       posSize, kPositions);
    glBufferSubData(GL_ARRAY_BUFFER, posSize, normSize, kNormals);

    // VAO: captures attribute layout + EBO binding
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_POSITION);
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(ATTRIB_NORMAL,   3, GL_FLOAT, GL_FALSE, 0, (void*)posSize);

    // Seal the VAO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (!program) { LOGE("Ground: failed to build shader program"); return; }
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
    if (uShiny     >= 0) glUniform1f(uShiny, 32.0f);
    // Light
    if (uLightAmbt  >= 0) glUniform3f(uLightAmbt,  1.0f, 1.0f, 1.0f);
    if (uLightDiff >= 0) glUniform3f(uLightDiff, 0.8f, 0.8f, 0.8f);
    if (uLightSpec >= 0) glUniform3f(uLightSpec, 1.0f, 1.0f, 1.0f);
    glUseProgram(0);
    
    LOGI("Ground::InitModel done (VAO=%u, VBO=%u, EBO=%u)", vao, vbo, ebo);
}

void Ground::Render() 
{
    if (!program || !vao) return;
    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);

    render_ground();
    glUseProgram(0);
}

void Ground::render_ground()
{   
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();

    transform.TransformPushMatrix();
    transform.TransformTranslate(0.f, -2.5f, 0.f);
    
    glm::mat4 mvp = *transform.TransformGetModelViewProjectionMatrix();
    glm::mat4 mv = *transform.TransformGetModelViewMatrix();
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(mv)));
    
    
    if(uMVP >= 0) glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    if(uMV >= 0)  glUniformMatrix4fv(uMV,  1, GL_FALSE, glm::value_ptr(mv));
    if(uNormalMatrix >= 0) glUniformMatrix3fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMat));
    if(uPartColor >= 0) glUniform3f(uPartColor, 1.f, 1.f, 1.f);
    transform.TransformPopMatrix();

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Ground::Resize(int w, int h) {
    // Usually, Ground doesn't need to do anything here!
    // Scene3D handles the camera and perspective projection matrix on resize,
    // so the Ground just relies on the matrix that Scene3D passes down.
    float aspect = (h > 0) ? (float)w / (float)h : 1.0f;
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadIdentity();
    transform.TransformSetPerspective(glm::radians(60.0f), aspect, 0.01f, 100.0f, 0);
}

void Ground::UpdateCamera(float distance)
{
    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadIdentity();

    transform.TransformTranslate(0.0f, 0.8f, -distance);

    transform.TransformSetMatrixMode(MODEL_MATRIX);
}

void Ground::SetLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) {
    if (!program) return;
    
    glUseProgram(program);
    
    if (uLightPos >= 0)  glUniform3fv(uLightPos, 1, glm::value_ptr(pos));
    if (uLightAmbt >= 0) glUniform3fv(uLightAmbt, 1, glm::value_ptr(ambient));
    if (uLightDiff >= 0) glUniform3fv(uLightDiff, 1, glm::value_ptr(diffuse));
    if (uLightSpec >= 0) glUniform3fv(uLightSpec, 1, glm::value_ptr(specular));
    
    glUseProgram(0);
}