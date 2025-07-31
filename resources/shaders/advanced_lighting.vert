#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

// Traditional interpolation output
out vec3 Normal;
out vec3 WorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

// Whether to use normal mapping
uniform bool useNormalMapping;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    WorldPos = FragPos;
    TexCoord = aTexCoord;
    
    // Traditional normal calculation
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    if (useNormalMapping) {
        // Calculate TBN matrix for tangent space
        vec3 T = normalize(mat3(model) * aTangent);
        vec3 N = normalize(mat3(model) * aNormal);
        T = normalize(T - dot(T, N) * N); // Gram-Schmidt orthogonalization
        vec3 B = cross(N, T);
        
        // If input bitangent is available, use it
        if (length(aBitangent) > 0.1) {
            B = normalize(mat3(model) * aBitangent);
        }
        
        // Build TBN matrix (world space to tangent space)
        mat3 TBN = transpose(mat3(T, B, N));
        
        // Transform light and viewer positions to tangent space
        TangentLightPos = TBN * lightPos;
        TangentViewPos = TBN * viewPos;
        TangentFragPos = TBN * FragPos;
    }
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
