#version 300 es
precision highp float;

layout(location = 0) in vec2 Position;

uniform mat4 PROJECTION;

void main() {
    gl_Position = PROJECTION * vec4(Position, 0.0, 1.0);
}