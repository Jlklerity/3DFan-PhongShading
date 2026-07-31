#version 300 es
precision mediump float;

uniform vec4 Color;
out vec4 outColor;

void main() {
    outColor = Color;
}