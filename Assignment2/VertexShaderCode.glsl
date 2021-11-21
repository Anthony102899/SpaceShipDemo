#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;

uniform int movable;
uniform mat4 modelTransformMatrix;
uniform mat4 view;
uniform mat4 projection;

out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;
void main()
{
	vec4 v = vec4(position, 1.0);
	vec4 out_position;
	vec4 newPosition;
	vec4 normal_temp;
	if(movable == 1){
		out_position = projection * view * modelTransformMatrix * v;
		normal_temp = modelTransformMatrix * vec4(normal, 0);
		newPosition = modelTransformMatrix * v;
	}
	normalWorld = normal_temp.xyz;
	vertexPositionWorld = newPosition.xyz;
	gl_Position = out_position;	
	UV = vertexUV;
}
