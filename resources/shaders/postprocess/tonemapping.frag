#version 410 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform int toneMappingMode;
uniform float exposure;
uniform float gamma;

// Reinhard tone mapping
vec3 ReinhardToneMapping(vec3 color) {
    return color / (color + vec3(1.0));
}

// ACES tone mapping (cinematic tone mapping)
vec3 ACESToneMapping(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// Uncharted 2 tone mapping
vec3 Uncharted2Partial(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 Uncharted2ToneMapping(vec3 color) {
    float exposureBias = 2.0;
    vec3 curr = Uncharted2Partial(color * exposureBias);
    vec3 whiteScale = 1.0 / Uncharted2Partial(vec3(11.2));
    return curr * whiteScale;
}

void main() {
    vec3 hdrColor = texture(screenTexture, TexCoord).rgb;
    
    // Apply exposure
    vec3 mapped = hdrColor * exposure;
    
    // Tone mapping
    if (toneMappingMode == 1) {
        // Reinhard
        mapped = ReinhardToneMapping(mapped);
    } else if (toneMappingMode == 2) {
        // ACES
        mapped = ACESToneMapping(mapped);
    } else if (toneMappingMode == 3) {
        // Uncharted 2
        mapped = Uncharted2ToneMapping(mapped);
    }
    
    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));
    
    FragColor = vec4(mapped, 1.0);
}
