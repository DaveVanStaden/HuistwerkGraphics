#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 vDirection;
out vec3 worldPosition;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calculate world position (optional, if needed in fragment shader)
    worldPosition = vec3(world * vec4(aPos, 1.0));

    // For skybox, direction from center (camera) to vertex, in view space
    vDirection = aPos;

    gl_Position = projection * view * world * vec4(aPos, 1.0);
}