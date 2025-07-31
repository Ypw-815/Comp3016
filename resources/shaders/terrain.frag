#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 VertexColor;
in vec4 FragPosLightSpace;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

// Shadow mapping uniforms
uniform sampler2D shadowMap;
uniform float shadowBias;
uniform float normalBias;
uniform int filterMode;
uniform float shadowMapSize;
uniform mat4 lightSpaceMatrix;

// Filter mode constants
const int FILTER_NEAREST = 0;
const int FILTER_LINEAR = 1;
const int FILTER_PCF_2x2 = 2;
const int FILTER_PCF_3x3 = 3;
const int FILTER_PCF_5x5 = 4;

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 1.0;
    }
    float currentDepth = projCoords.z;
    if (filterMode == FILTER_NEAREST) {
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        return currentDepth - bias > closestDepth ? 0.0 : 1.0;
    }
    else if (filterMode == FILTER_LINEAR) {
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        return currentDepth - bias > closestDepth ? 0.0 : 1.0;
    }
    else {
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        int kernelSize;
        if (filterMode == FILTER_PCF_2x2) {
            kernelSize = 1;
        } else if (filterMode == FILTER_PCF_3x3) {
            kernelSize = 1;
        } else {
            kernelSize = 2;
        }
        int sampleCount = 0;
        for(int x = -kernelSize; x <= kernelSize; ++x) {
            for(int y = -kernelSize; y <= kernelSize; ++y) {
                vec2 offset = vec2(x, y) * texelSize;
                float pcfDepth = texture(shadowMap, projCoords.xy + offset).r;
                shadow += currentDepth - bias > pcfDepth ? 0.0 : 1.0;
                sampleCount++;
            }
        }
        shadow /= float(sampleCount);
        return shadow;
    }
}

void main()
{
    // Basic lighting calculation
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 ambient = 0.3 * lightColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 0.2 * spec * lightColor;

    // Shadow bias
    float cosTheta = max(dot(norm, lightDir), 0.0);
    float bias = max(shadowBias * (1.0 - cosTheta), shadowBias * 0.1);
    vec3 offsetPos = FragPos + norm * normalBias;
    vec4 offsetLightSpace = lightSpaceMatrix * vec4(offsetPos, 1.0);
    float shadow = ShadowCalculation(offsetLightSpace, bias);

    // Combine lighting with shadow and vertex color (biome color)
    vec3 result = (ambient + shadow * (diffuse + specular)) * VertexColor;

    // Add slight fog based on distance for atmosphere
    float distance = length(viewPos - FragPos);
    float fogFactor = exp(-distance * 0.02);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec3 fogColor = vec3(0.7, 0.8, 0.9); // Light blue-gray
    result = mix(fogColor, result, fogFactor);

    FragColor = vec4(result, 1.0);
}
