#version 300 es
precision highp float;

in vec3 WorldPos;
in vec3 ViewPos;
in vec3 Normal;

uniform vec3 LightAmbient;
uniform vec3 LightDiffuse;
uniform vec3 LightSpecular;
uniform vec3 LightPosition;
uniform float ShininessFactor;

out vec4 outColor;

vec3 PhongShading() {

    float cellX = floor(WorldPos.x * 1.5);
    float cellZ = floor(WorldPos.z * 1.5);
    float check = mod(cellX + cellZ, 2.0);

    vec3 dark  = vec3(0.16, 0.19, 0.23);
    vec3 light = vec3(0.75, 0.78, 0.82);
    vec3 materialColor = mix(dark, light, check);

    // Subtle radial fade so the plane's edge doesn't end in a hard line
    float d = clamp(length(WorldPos.xz) / 8.0, 0.0, 1.0);
    materialColor *= 1.0 - 0.45 * d * d;

    vec3 N = normalize(Normal);
    vec3 L = normalize(LightPosition - ViewPos);
    vec3 V = normalize(-ViewPos);
    vec3 R = reflect(-L, N);

    
    vec3 ambient = LightAmbient * materialColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = LightDiffuse * materialColor * diff;

    float spec = pow(max(dot(V, R), 0.0), ShininessFactor);
    vec3 specular = LightSpecular * spec;

    vec3 FinalColor = ambient + diffuse + specular;
    return FinalColor;
}

void main() {
    
    PhongShading();
    outColor = vec4(PhongShading(), 1.0);
}