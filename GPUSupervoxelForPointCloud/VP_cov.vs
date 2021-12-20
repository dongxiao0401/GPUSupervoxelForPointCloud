#version 410 core
layout (location = 0) in vec2 aPos;

uniform sampler2DRect seg;
uniform sampler2DRect seedMatrix;
uniform sampler2DRect voxelPos;
uniform float width;
uniform float height;
uniform float seeds_num;

out vec3 cov_first3;
out vec3 cov_last3;


void main()
{
	float target_id;
	vec2 target_pos;

	vec3 voxel_pos = texture(voxelPos,aPos.xy).rgb;
	target_id = texture(seg,aPos.xy).x;
	if(target_id<0)
	{
		target_id = seeds_num;
		target_pos.y = floor(target_id/width);
		target_pos.x = target_id-target_pos.y*width;
		target_pos += 0.5;
		cov_first3 = vec3(0.0,0.0,0.0);
		cov_last3 = vec3(0.0,0.0,0.0);
		
	}else
	{
		target_pos.y = floor(target_id/width);
		target_pos.x = target_id-target_pos.y*width;
		target_pos += 0.5;
		//vec4 seed_pos = texture(averPos,target_pos).xyzw;
		float seed_num = texture(seedMatrix,vec2(0,target_id)).g;
		vec3 seed_pos = texture(seedMatrix,vec2(1,target_id)).rgb;
		if(seed_num<=1)
		{
			cov_first3 = vec3(0.0,0.0,0.0);
			cov_last3 = vec3(0.0,0.0,0.0);
		}else
		{
			cov_first3.x = (voxel_pos.x-seed_pos.x)*(voxel_pos.x-seed_pos.x);
			cov_first3.y = (voxel_pos.x-seed_pos.x)*(voxel_pos.y-seed_pos.y);
			cov_first3.z = (voxel_pos.x-seed_pos.x)*(voxel_pos.z-seed_pos.z);
			cov_last3.x = (voxel_pos.y-seed_pos.y)*(voxel_pos.y-seed_pos.y);
			cov_last3.y = (voxel_pos.y-seed_pos.y)*(voxel_pos.z-seed_pos.z);
			cov_last3.z = (voxel_pos.z-seed_pos.z)*(voxel_pos.z-seed_pos.z);
		}

		
	}
	////uv to opengl
	target_pos.x = (2*target_pos.x)/width-1;
	target_pos.y = (2*target_pos.y)/height-1;
	gl_Position = vec4(target_pos,0.0f,1.0f);
	
	
}