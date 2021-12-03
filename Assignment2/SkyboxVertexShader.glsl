#version 430

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 Coords;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = Coords;
    gl_Position = projection * view * vec4(aPos, 1.0);
}