#version 300 es

precision mediump float;

// Material property
uniform vec3 uPartColor;
uniform vec3 MaterialSpecular;

// Light property
uniform vec3 LightAmbient;
uniform vec3 LightSpecular;
uniform vec3 LightDiffuse;
uniform float ShininessFactor;
uniform vec3 LightPosition;

in vec3 normalCoord;
in vec3 eyeCoord;

layout(location = 0) out vec4 FinalColor;


vec3 PhongShading()
{
    vec3 normalizeNormal = normalize( normalCoord );
    vec3 normalizeEyeCoord = normalize( eyeCoord );
    vec3 normalizeLightVec = normalize( LightPosition - eyeCoord );

    // Diffuse Intensity
    float cosAngle = max( 0.0, dot( normalizeNormal, normalizeLightVec ));

    // Specular Intensity
    vec3 V = -normalizeEyeCoord; // Viewer's vector
    vec3 R = reflect( -normalizeLightVec, normalizeNormal ); // Reflectivity
    float sIntensity = pow( max( 0.0, dot( R, V ) ), ShininessFactor );

    // ADS color as result of Material & Light interaction
    vec3 ambient = uPartColor * LightAmbient;
    vec3 diffuse = uPartColor * LightDiffuse;
    vec3 specular = MaterialSpecular * LightSpecular;
    
    return ambient + ( cosAngle * diffuse ) + ( sIntensity * specular );
}
void main() {
    FinalColor = vec4(PhongShading(), 1.0);
}