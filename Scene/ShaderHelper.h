#pragma once

#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper {
public:

#ifdef PLATFORM_ANDROID
    static std::string loadAsset(AAssetManager* mgr, const char* path) {
        if (!mgr || !path) return {};
        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) { LOGE("ShaderHelper: cannot open asset: %s", path); return {}; }
        size_t sz = static_cast<size_t>(AAsset_getLength(asset));
        std::string src(sz, '\0');
        AAsset_read(asset, &src[0], sz);
        AAsset_close(asset);
        return src;
    }
    static GLuint buildProgramFromAssets(AAssetManager* mgr,
                                         const char* vert, const char* frag) {
        std::string v = loadAsset(mgr, vert);
        std::string f = loadAsset(mgr, frag);
        if (v.empty() || f.empty()) return 0;
        return buildProgram(v.c_str(), f.c_str());
    }
#endif

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_EMSCRIPTEN)
    static std::string loadFile(const char* filename) {
        for (const auto& prefix : std::vector<std::string>{
                "assets/shader/", "", "shader/", "assets/"}) {
            std::ifstream f(prefix + filename);
            if (f.is_open()) {
                std::stringstream buf; buf << f.rdbuf();
                LOGI("ShaderHelper: loaded %s", (prefix + filename).c_str());
                return buf.str();
            }
        }
        LOGE("ShaderHelper: cannot find %s", filename);
        return {};
    }
    static GLuint buildProgramFromFile(const char* vert, const char* frag) {
        std::string v = loadFile(vert), f = loadFile(frag);
        if (v.empty() || f.empty()) return 0;
        return buildProgram(v.c_str(), f.c_str());
    }
#endif

    static GLuint compileShader(GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        if (!s) return 0;
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetShaderInfoLog(s, len, nullptr, &log[0]);
            LOGE("ShaderHelper compile:\n%s", log.c_str());
            glDeleteShader(s); return 0;
        }
        return s;
    }

    static GLuint linkProgram(GLuint vert, GLuint frag) {
        GLuint p = glCreateProgram();
        glAttachShader(p, vert); glAttachShader(p, frag);
        glLinkProgram(p);
        GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len; glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetProgramInfoLog(p, len, nullptr, &log[0]);
            LOGE("ShaderHelper link:\n%s", log.c_str());
            glDeleteProgram(p); return 0;
        }
        glDetachShader(p,vert); glDetachShader(p,frag);
        glDeleteShader(vert);   glDeleteShader(frag);
        return p;
    }

    static GLuint buildProgram(const char* vs, const char* fs) {
        GLuint v = compileShader(GL_VERTEX_SHADER,   vs); if (!v) return 0;
        GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
        if (!f) { glDeleteShader(v); return 0; }
        return linkProgram(v, f);
    }
};
