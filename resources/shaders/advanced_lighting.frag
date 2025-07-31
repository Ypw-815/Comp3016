#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoord;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

// Traditional interpolated input
in vec3 Normal;
in vec3 WorldPos;

// Material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

// Multi-layer texture blending
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_normal2;
uniform sampler2D texture_blend_mask;

// Procedural texture
uniform sampler2D texture_noise;

// Material properties
struct Material {
    float shininess;
    float normalStrength;
    float parallaxScale;
    float parallaxBias;
    bool useParallax;
    bool useNormalMap;
    bool useMultiLayer;
    bool useNoise;
};

// Light properties
struct Light {
    vec3 diffuse;
    vec3 specular;
    float quadratic;
};
uniform vec3 viewPos;
uniform float time; // For texture animation
// Texture animation parameters
uniform vec2 textureSpeed1;
// Traditional interpolated input
in vec3 Normal;
in vec3 WorldPos;
uniform vec2 textureSpeed2;
// Material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform float noiseScale;
// Multi-layer texture blending
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_normal2;
uniform sampler2D texture_blend_mask;
uniform float noiseSpeed;
// Procedural texture
uniform sampler2D texture_noise;

// Distance-related parameters
uniform float viewDistance;
uniform float textureLODBias;

// Parallax mapping function
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    if (!material.useParallax) return texCoords;
    
    // Sample height
    float height = texture(texture_height1, texCoords).r;
    
    // Simple parallax offset
    vec2 p = viewDir.xy / viewDir.z * (height * material.parallaxScale + material.parallaxBias);
    return texCoords - p;
}

// Steep parallax mapping (higher quality)
vec2 SteepParallaxMapping(vec2 texCoords, vec3 viewDir) {
    if (!material.useParallax) return texCoords;
    
    const int numLayers = 32;
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    
    vec2 P = viewDir.xy * material.parallaxScale;
    vec2 deltaTexCoords = P / numLayers;
    
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(texture_height1, currentTexCoords).r;
    
    while (currentLayerDepth < currentDepthMapValue) {
// Material properties
// Light properties
// Attenuation
// For texture animation
// Texture animation parameters
// Distance-related parameters
// Parallax mapping function
// Sample height
// Simple parallax offset
// Steep parallax mapping (higher quality)
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(texture_height1, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }
    
    // Parallax occlusion mapping - interpolate for more accurate result
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(texture_height1, prevTexCoords).r - currentLayerDepth + layerDepth;
    
    float weight = afterDepth / (afterDepth - beforeDepth);
    return mix(currentTexCoords, prevTexCoords, weight);
}

// Calculate normal (from normal map or default)
vec3 CalculateNormal(vec2 texCoords) {
    if (material.useNormalMap) {
        // Sample from normal map
        vec3 normal = texture(texture_normal1, texCoords).rgb;
        normal = normal * 2.0 - 1.0; // Convert to [-1, 1] range
        normal.xy *= material.normalStrength;
        return normalize(normal);
    } else {
        // Use vertex normal (in tangent space is (0, 0, 1))
        return normalize(vec3(0.0, 0.0, 1.0));
    }
}

// Blinn-Phong lighting calculation (in tangent space)
vec3 CalculateLighting(vec3 normal, vec2 texCoords, vec3 fragPos, vec3 lightPos, vec3 viewPos) {
    // Diffuse texture sampling
    vec3 color = texture(texture_diffuse1, texCoords).rgb;
    
    // Multi-layer texture blending
    if (material.useMultiLayer) {
        vec3 color2 = texture(texture_diffuse2, texCoords + textureSpeed2 * time).rgb;
        float blendFactor = texture(texture_blend_mask, texCoords).r;
        color = mix(color, color2, blendFactor);
    }
    
    // Procedural noise texture
    if (material.useNoise) {
        vec2 noiseCoords = texCoords * noiseScale + time * noiseSpeed;
        float noise = texture(texture_noise, noiseCoords).r;
        color *= (0.8 + 0.4 * noise); // Modulate color
    }
    
    // Ambient light
    vec3 ambient = light.ambient * color;
    
    // Diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * diff * color;
    
    // Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specularMap = texture(texture_specular1, texCoords).rgb;
    vec3 specular = light.specular * spec * specularMap;
    
    // Calculate attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

// Traditional lighting calculation (world space)
vec3 CalculateTraditionalLighting() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - WorldPos);
    
    // Animated texture coordinates
    vec2 animTexCoords = TexCoord + textureSpeed1 * time;
    
    vec3 color = texture(texture_diffuse1, animTexCoords).rgb;
    
    // Ambient light
    vec3 ambient = light.ambient * color;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * color;
    
    // Specular
    vec3 viewDir = normalize(viewPos - WorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(texture_specular1, animTexCoords).rgb;
    
    return ambient + diffuse + specular;
}

void main()
{
    vec3 result;
    
    if (material.useNormalMap || material.useParallax) {
        // Use advanced texture techniques
        vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
        
        // Parallax mapping
        vec2 texCoords = TexCoord;
        if (material.useParallax) {
            texCoords = SteepParallaxMapping(TexCoord, viewDir);
            
            // Discard edge pixels
            if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) {
                discard;
            }
        }
        
        // Animated texture coordinates
        texCoords += textureSpeed1 * time;
        
        // Calculate normal
        vec3 normal = CalculateNormal(texCoords);
        
        // Calculate lighting
        result = CalculateLighting(normal, texCoords, TangentFragPos, TangentLightPos, TangentViewPos);
    } else {
        // Use traditional lighting
        result = CalculateTraditionalLighting();
    }
    
    // Distance-based texture filtering effect
    float distanceFromCamera = length(viewPos - WorldPos);
    float lodFactor = distanceFromCamera / viewDistance;
    
    if (lodFactor > 0.5) {
        // Reduce detail at far distance
        result = mix(result, vec3(dot(result, vec3(0.299, 0.587, 0.114))), (lodFactor - 0.5) * 0.3);
    }
    
    // Gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}
