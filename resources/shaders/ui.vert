#version 410 core
layout (location = 0) in vec2 aPos;        // Position
layout (location = 1) in vec2 aTexCoord;   // Texture coordinates

out vec2 TexCoord;

uniform mat4 projection;
uniform vec2 position;
uniform vec2 size;

void main()
{
    // Transform vertex to UI space
    vec2 vertexPos = aPos * size + position;
    
    // Convert to normalized device coordinates
    gl_Position = projection * vec4(vertexPos, 0.0, 1.0);
    
    TexCoord = aTexCoord;
}
