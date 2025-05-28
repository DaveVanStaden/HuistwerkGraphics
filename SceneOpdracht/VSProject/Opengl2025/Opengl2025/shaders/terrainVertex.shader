#version 330 core
layout(location = 0) in vec3 aPos; 
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

out vec2 uv;
out vec3 normal;
out vec3 worldPosition; // Position in world space

uniform mat4 world,view,projection;

uniform sampler2D mainTex; // Texture for height map
void main()
{
	vec3 pos = aPos;
	//object space offset
	vec4 worldPos = world * vec4(aPos, 1.0f);
	//world pos offsets!
	worldPos.y += texture(mainTex, vUV).r * 800.0f;

	gl_Position = projection * view * worldPos;
	uv = vUV;
	worldPosition = mat3(world) * aPos; 
}