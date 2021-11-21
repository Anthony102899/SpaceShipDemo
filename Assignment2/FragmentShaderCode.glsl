#version 430

in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;

out vec4 Color;

uniform sampler2D sampler1;
uniform vec3 ambientLight;
uniform vec3 lightPosition;
uniform vec3 eyePositionWorld;
uniform int independent;

void main()
{
	vec3 lightVectorWorld = normalize(lightPosition - vertexPositionWorld);
	float brightness = dot(lightVectorWorld, normalize(normalWorld));
	vec4 diffuseLight = vec4(brightness / 2, brightness / 2 , brightness / 2.4, 1.0);

	vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld, normalWorld);
	vec3 eyeVectorWorld = normalize(eyePositionWorld - vertexPositionWorld);
	float s = clamp(dot(reflectedLightVectorWorld, eyeVectorWorld),0,1);
	s = pow(s, 50);
	vec4 specularLight = vec4(s,s,s/1.2,1);
	vec3 color;
	if(independent == 0){
		color = texture(sampler1, UV).rgb * ambientLight;
		Color = vec4(color, 1.0) + clamp(diffuseLight, 0, 1) + specularLight;
	}else{
		color = texture(sampler1, UV).rgb;
		Color = vec4(color, 1.0);
	}
}