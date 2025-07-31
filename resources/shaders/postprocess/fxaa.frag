#version 410 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float lumaThreshold;
uniform float mulReduce;
uniform float minReduce;
uniform float maxSpan;
uniform vec2 texelStep;

void main() {
    vec3 rgbNW = texture(screenTexture, TexCoord + (vec2(-1.0, -1.0) * texelStep)).xyz;
    vec3 rgbNE = texture(screenTexture, TexCoord + (vec2(1.0, -1.0) * texelStep)).xyz;
    vec3 rgbSW = texture(screenTexture, TexCoord + (vec2(-1.0, 1.0) * texelStep)).xyz;
    vec3 rgbSE = texture(screenTexture, TexCoord + (vec2(1.0, 1.0) * texelStep)).xyz;
    vec3 rgbM  = texture(screenTexture, TexCoord).xyz;

    const vec3 luma = vec3(0.299, 0.587, 0.114);
    
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * 1.0/mulReduce), 1.0/minReduce);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(maxSpan, maxSpan), max(vec2(-maxSpan, -maxSpan), dir * rcpDirMin)) * texelStep;

    vec3 rgbA = 0.5 * (
        texture(screenTexture, TexCoord + dir * (1.0/3.0 - 0.5)).xyz +
        texture(screenTexture, TexCoord + dir * (2.0/3.0 - 0.5)).xyz);
        
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(screenTexture, TexCoord + dir * -0.5).xyz +
        texture(screenTexture, TexCoord + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);

    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        FragColor = vec4(rgbA, 1.0);
    } else {
        FragColor = vec4(rgbB, 1.0);
    }
}
