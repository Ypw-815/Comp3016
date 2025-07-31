#version 410 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

// Material properties
uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

// Lighting properties
uniform vec3 lightPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;

// Shadow properties
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

/**
 * Calculate shadow value
 * @param projCoords Projection coordinates
 * @param bias Depth bias
 * @return Shadow value (0.0 = completely in shadow, 1.0 = completely lit)
 */
float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    // Perform perspective division
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
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

/**
 * Calculate Blinn-Phong lighting
 */
vec3 BlinnPhong(vec3 color, vec3 normal, vec3 lightColor, vec3 lightDir, vec3 viewDir)
{
    // Ambient light
    vec3 ambient = 0.15 * color;
    
    // Diffuse reflection
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular reflection
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    
    return ambient + diffuse + specular;
}

void main()
{
    vec3 color = texture(texture_diffuse1, TexCoord).rgb;
    vec3 normal = normalize(Normal);
    
    // Calculate lighting direction
    vec3 lightDirection = normalize(-lightDir); // Assume lightDir points from light source
    vec3 viewDirection = normalize(viewPos - FragPos);
    
    // Calculate basic lighting
    vec3 lighting = BlinnPhong(color, normal, lightColor, lightDirection, viewDirection);
    
    // Calculate shadow bias (considering surface angle)
    float cosTheta = max(dot(normal, lightDirection), 0.0);
    float bias = max(shadowBias * (1.0 - cosTheta), shadowBias * 0.1);
    
    // Add normal bias to reduce shadow artifacts
    vec3 offsetPos = FragPos + normal * normalBias;
    vec4 offsetLightSpace = lightSpaceMatrix * vec4(offsetPos, 1.0);
    
    // Calculate shadow value
    float shadow = ShadowCalculation(offsetLightSpace, bias);
    
    // Apply shadow: ambient light not affected by shadow, diffuse and specular affected by shadow
    vec3 ambient = 0.15 * color;
    vec3 diffuseAndSpecular = lighting - ambient;
    vec3 finalColor = ambient + shadow * diffuseAndSpecular;
    
    // Output final color
    FragColor = vec4(finalColor, 1.0);
    
    // Debug output (optional)
    // FragColor = vec4(vec3(shadow), 1.0); // Display shadow map
}
