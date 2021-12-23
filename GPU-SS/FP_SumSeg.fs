#version 410 core
layout (location = 0) out vec4 FragPos;

in vec3 pos;

void main()
{
	FragPos = vec4(pos,1.0);
}