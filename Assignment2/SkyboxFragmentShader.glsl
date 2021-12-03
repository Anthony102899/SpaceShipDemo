#version 430

out vec4 Color;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
   vec3 FragColor = texture(skybox, TexCoords).rgb;
   Color = vec4(FragColor, 1.0);
}