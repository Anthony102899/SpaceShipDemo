#version 430

out vec4 Color;

in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;

uniform sampler2D sampler1;
uniform sampler2D sampler2;
uniform vec4 ambientLight;
uniform vec3 lightPosition;
uniform vec3 eyePositionWorld;
uniform vec3 lightPositionWorld1;
uniform vec3 lightcolor1;
uniform vec3 lightPositionWorld2;
uniform vec3 lightcolor2;
uniform float delta;
uniform float delta1;
uniform int normalMapping_flag;

void main()
{
	vec3 normal;
	if(normalMapping_flag == 1)
	{
		normal = texture(sampler2, UV).rgb;
		normal = normalize(normal * 2.0 - 1.0);
	}else{
		normal = normalize(normalWorld);
	 }
	vec3 T = texture(sampler1, UV).rgb;
	vec3 eyeVectorWorld = normalize(eyePositionWorld - vertexPositionWorld);

	vec3 lightVectorWorld1 = normalize(lightPositionWorld1 - vertexPositionWorld);
	float brightness1 = dot(lightVectorWorld1, normal);
	vec3 diffuseLight1 = max(vec3(brightness1), 0.0);
	vec3 reflectedLightVectorWorld1 = reflect(-lightVectorWorld1, normal);
	float s1 = clamp(dot(reflectedLightVectorWorld1, eyeVectorWorld), 0, 3);
	s1 = pow(s1, 80);
	vec3 specularLight1 = vec3(s1,s1,s1);
	float distance1 = length(lightPositionWorld1 - vertexPositionWorld);
	float attenuation1 = 1.0f / (1 + distance1 + 0.01 * distance1 * distance1);
	vec3 theColor = T * ambientLight.xyz * lightcolor1 * delta + diffuseLight1 * lightcolor1 + specularLight1 * lightcolor1 * attenuation1;

	vec3 lightVectorWorld2 = normalize(lightPositionWorld2 - vertexPositionWorld);
	float brightness2 = dot(lightVectorWorld2, normal);
	vec3 diffuseLight2 = clamp(vec3(brightness2), 0, 1);
	vec3 reflectedLightVectorWorld2 = reflect(-lightVectorWorld2, normal);
	float s2 = clamp(dot(reflectedLightVectorWorld2, eyeVectorWorld), 0, 1);
	s2 = pow(s2, 80);
	vec3 specularLight2 = vec3(s2,s2,s2);
	float distance2 = length(lightPositionWorld2 - vertexPositionWorld);
	float attenuation2 = 1.0f / (1 +  distance2 + distance2 * distance2);
	theColor = theColor + diffuseLight2 * lightcolor2 * attenuation1 + specularLight2 * lightcolor2 * attenuation1;
	Color =  vec4(theColor, 1.0f);
}