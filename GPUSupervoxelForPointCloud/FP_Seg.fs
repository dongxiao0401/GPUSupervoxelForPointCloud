#version 430 core
layout(location = 0) out float FragSeedId;


uniform sampler2DRect voxelPos;
uniform sampler2DRect voxelNormal;
uniform sampler2DRect voxelGrid;
uniform sampler2DRect seedNeighbor;
uniform float lambda_1;
uniform float lambda_2;



void main()
{
	vec2 texCoord = gl_FragCoord.xy;
	vec3 voxel_pos = texture(voxelPos,texCoord).xyz;
	vec3 voxel_normal = texture(voxelNormal,texCoord).xyz;
	vec3 ng_seed_pos;
	vec3 ng_seed_normal;
	float min_dist = 100000000;
	float curr_pos_dist=0;
	float curr_normal_dist=0;
	float curr_plane_dist=0;
	float curr_dist;
	float NearestSeedId=-1;
	float voxel_gridId = texture(voxelGrid,texCoord).x;
	if(voxel_gridId<0)
	{
		FragSeedId = -1;
	}
	else
	{
		float ng_index=0.5;
		vec4 ng_tex = texture(seedNeighbor,vec2(ng_index,voxel_gridId)).xyzw;
		while(ng_tex.x>0 && ng_index<27)
		{
		    if(ng_tex.w>0.5)
			{
				ng_seed_pos = texture(voxelPos,ng_tex.xy).xyz;
				ng_seed_normal = texture(voxelNormal,ng_tex.xy).xyz;
				curr_pos_dist = dot((voxel_pos-ng_seed_pos),(voxel_pos-ng_seed_pos));
				curr_normal_dist = 1-pow(dot(voxel_normal,ng_seed_normal),2);
				curr_plane_dist = pow(dot((voxel_pos-ng_seed_pos),ng_seed_normal),2);
				//curr_dist = curr_pos_dist;//+ratio*curr_normal_dist+10.0*curr_plane_dist;      
				curr_dist = curr_plane_dist+lambda_1*curr_normal_dist+lambda_2*curr_pos_dist;
				
				
				if(curr_dist<min_dist)
				{
					min_dist = curr_dist;
					NearestSeedId = ng_tex.z;
				}
			}
			ng_index=ng_index+1;
			if(ng_index<27)
				ng_tex = texture(seedNeighbor,vec2(ng_index,voxel_gridId)).xyzw;
		}


		FragSeedId = NearestSeedId;
	}
	
	
	
}