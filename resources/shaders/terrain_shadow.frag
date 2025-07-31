#version 410 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 VertexColor;
in vec4 FragPosLightSpace;

// Shadow map
uniform sampler2D shadowMap;

// Lighting properties
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

// Shadow properties
uniform float shadowBias;
uniform float normalBias;
uniform int filterMode;
uniform float shadowMapSize;

// Filter mode constants
const int FILTER_NEAREST = 0;
const int FILTER_LINEAR = 1;
const int FILTER_PCF_2x2 = 2;
const int FILTER_PCF_3x3 = 3;
const int FILTER_PCF_5x5 = 4;

// Calculate shadow value
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Perform perspective division
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Calculate bias based on normal and light direction
    float bias = max(shadowBias * (1.0 - dot(normal, lightDir)), normalBias);
    
    // If outside shadow map range, consider not in shadow
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 1.0;
    }
    
    // Get current fragment depth
    float currentDepth = projCoords.z;
    
    // Choose shadow calculation method based on filter mode
    if (filterMode == FILTER_NEAREST) {
        // Nearest neighbor filtering - hard shadows
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        return currentDepth - bias > closestDepth ? 0.0 : 1.0;
    }
    else if (filterMode == FILTER_LINEAR) {
        // Linear filtering
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        return currentDepth - bias > closestDepth ? 0.0 : 1.0;
    }
    else {
        // PCF (Percentage Closer Filtering) soft shadows
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        
        int kernelSize;
        if (filterMode == FILTER_PCF_2x2) {
            kernelSize = 1;
        } else if (filterMode == FILTER_PCF_3x3) {
            kernelSize = 1;
        } else { // FILTER_PCF_5x5
            kernelSize = 2;
        }
        
        for(int x = -kernelSize; x <= kernelSize; ++x) {
            for(int y = -kernelSize; y <= kernelSize; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 0.0 : 1.0;
            }
        }
        
        int totalSamples = (kernelSize * 2 + 1) * (kernelSize * 2 + 1);
        shadow /= float(totalSamples);
        
        return shadow;
    }
}

void main()
{
    // Basic lighting calculation
    vec3 ambient = 0.3 * lightColor;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = 0.2 * spec * lightColor;
    
    // Calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace, norm, lightDir);
    
    // Combine lighting and shadow (ambient light not affected by shadow)
    vec3 lighting = ambient + shadow * (diffuse + specular);
    vec3 result = lighting * VertexColor;
    
    // Add distance-based fog effect
    float distance = length(viewPos - FragPos);
    float fogFactor = exp(-distance * 0.02);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    vec3 fogColor = vec3(0.7, 0.8, 0.9); // Light blue-gray
    result = mix(fogColor, result, fogFactor);
    
    FragColor = vec4(result, 1.0);
}
