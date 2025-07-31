#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    // Transform vertex to light space coordinate system
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
