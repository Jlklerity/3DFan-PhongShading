#version 300 es
precision highp float;

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;



uniform mat4 ModelViewProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;

out vec3 WorldPos;
out vec3 ViewPos;
out vec3 Normal;

void main() {
    WorldPos    = VertexPosition;
    vec4 viewSpacePos = ModelViewMatrix * vec4(WorldPos, 1.0);
    ViewPos = viewSpacePos.xyz;
    Normal = NormalMatrix * VertexNormal;

    gl_Position = ModelViewProjectionMatrix * vec4(VertexPosition, 1.0);
}
