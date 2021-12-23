#version 410 core
layout (location = 0) out vec4 FragPos;
layout (location = 1) out vec3 FragNormal;

in vec3 pos;
in vec3 normal;
void main()
{
	FragPos = vec4(pos,1.0);
	FragNormal = normal;
}