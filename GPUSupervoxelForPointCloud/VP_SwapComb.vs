#version 430 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aCoord;



void main()
{
	gl_Position = vec4(aPos.x,aPos.y,0.0f,1.0f);
}