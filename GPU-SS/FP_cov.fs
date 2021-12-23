#version 410 core
layout (location = 0) out vec4 FragCov0;
layout (location = 1) out vec3 FragCov1;

in vec3 cov_first3;
in vec3 cov_last3;
void main()
{
	FragCov0 = vec4(cov_first3,1.0);
	FragCov1 = cov_last3;
}