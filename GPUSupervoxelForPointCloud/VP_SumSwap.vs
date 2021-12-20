#version 410 core
layout (location = 0) in vec2 aPos;

uniform sampler2DRect seg;
uniform sampler2DRect voxelPos;
uniform sampler2DRect voxelNormal;
uniform float width;
uniform float height;
uniform float seeds_num;

out vec3 pos;
out vec3 normal;


void main()
{
	float target_id;
	vec2 target_pos;

	pos = texture(voxelPos,aPos.xy).rgb;
	normal = texture(voxelNormal,aPos.xy).rgb;
	target_id = texture(seg,aPos.xy).x;

	if(target_id<0)
		target_id = seeds_num+3;
 
	target_pos.y = floor(target_id/width);
	target_pos.x = target_id-target_pos.y*width;
	target_pos += 0.5;
	//uv to opengl
	target_pos.x = (2*target_pos.x)/width-1;
	target_pos.y = (2*target_pos.y)/height-1;
	gl_Position = vec4(target_pos,0.0f,1.0f);
}