#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 ViewPos;
in vec4 FragPosLightSpace;

// Material properties
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

// Point light properties
struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Spotlight properties
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Directional light properties
struct DirLight {
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform PointLight pointLight;
uniform SpotLight spotLight;
uniform DirLight dirLight;

uniform vec3 viewPos;
uniform bool useBlinnPhong;
uniform bool enableSpotLight;
uniform bool enableDirLight;

// Shadow mapping uniforms
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
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

// Shadow calculation function
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }
    float currentDepth = projCoords.z;
    float bias = max(shadowBias * (1.0 - max(dot(normal, lightDir), 0.0)), shadowBias * 0.1);
    float shadow = 0.0;
    if (filterMode == FILTER_NEAREST) {
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    } else if (filterMode == FILTER_LINEAR) {
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    } else {
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        int kernelSize = (filterMode == FILTER_PCF_5x5) ? 2 : 1;
        int sampleCount = 0;
        for(int x = -kernelSize; x <= kernelSize; ++x) {
            for(int y = -kernelSize; y <= kernelSize; ++y) {
                vec2 offset = vec2(x, y) * texelSize;
                float pcfDepth = texture(shadowMap, projCoords.xy + offset).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
                sampleCount++;
            }
        }
        shadow /= float(sampleCount);
    }
    return shadow;
}

// Function declarations
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0);
    
    // Directional light with shadow
    float shadow = 0.0;
    vec3 mainLightDir = vec3(0.0);
    if (enableDirLight) {
        mainLightDir = normalize(-dirLight.direction);
        shadow = ShadowCalculation(FragPosLightSpace, norm, mainLightDir);
        result += (1.0 - shadow) * CalcDirLight(dirLight, norm, viewDir);
    }
    
    // Point light
    result += CalcPointLight(pointLight, norm, FragPos, viewDir);
    
    // Spotlight
    if (enableSpotLight) {
        result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
    }
    
    FragColor = vec4(result, 1.0);
}

// Calculate directional light
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading
    float spec;
    if (useBlinnPhong) {
        // Blinn-Phong
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
        // Standard Phong
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    
    // Combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
    
    return (ambient + diffuse + specular);
}

// Calculate point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading
    float spec;
    if (useBlinnPhong) {
        // Blinn-Phong
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
        // Standard Phong
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

// Calculate spotlight
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading
    float spec;
    if (useBlinnPhong) {
        // Blinn-Phong
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
        // Standard Phong
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
    
    // Apply spotlight intensity (ambient light not affected)
    diffuse *= intensity;
    specular *= intensity;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}
