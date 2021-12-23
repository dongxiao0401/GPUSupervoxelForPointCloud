#version 430 core
layout(location = 0) out float FragSeedId;

uniform sampler2DRect seg;
uniform sampler2DRect voxelPos;
uniform sampler2DRect voxelNormal;
uniform sampler2DRect voxelGrid;
uniform sampler2DRect seedNeighbor;
uniform sampler2DRect seedMatrix;
uniform float lambda1;
uniform float lambda2;



void main()
{
	vec2 texCoord = gl_FragCoord.xy;
	vec3 v_pos = texture(voxelPos,texCoord).xyz;
	vec3 v_norm = texture(voxelNormal,texCoord).xyz;
	mat3 v_N = mat3(v_norm.x*v_norm.x,v_norm.x*v_norm.y,v_norm.x*v_norm.z,v_norm.y*v_norm.x,v_norm.y*v_norm.y,v_norm.y*v_norm.z,v_norm.z*v_norm.x,v_norm.z*v_norm.y,v_norm.z*v_norm.z);
	mat3 U_i;
	mat3 U_i_n;
	//mat3 N_i;
	vec3 n_i;
	float num_i;
	float segId_i;
	
	mat3 U_j;
	mat3 U_j_n;
	//mat3 N_j;
	vec3 n_j;
	float num_j;
	float segId_j;
	

	float min_dist = 100000000;
	float delta_eg=0;
	float delta_eg_fit=0;
	float delta_eg_normal=0;
	float delta_eg_pos=0;
	float NearestSeedId=-1;
	float voxel_gridId = texture(voxelGrid,texCoord).x;

	
	vec3 dp1;
	vec3 dp2;
	vec3 tm1;
	vec3 tm2;
	
	if(voxel_gridId<0)
	{
		FragSeedId = -1;
	}
	else
	{
		segId_i = texture(seg,texCoord).x;
		dp1 = texture(seedMatrix,vec2(3,segId_i)).rgb;
		dp2 = texture(seedMatrix,vec2(4,segId_i)).rgb;
		U_i = mat3(dp1.x,dp1.y,dp1.z,dp1.y,dp2.x,dp2.y,dp1.z,dp2.y,dp2.z);
		tm1 = texture(seedMatrix,vec2(1,segId_i)).rgb;//pos_i
		num_i = texture(seedMatrix,vec2(0,segId_i)).g;
		dp1 = (num_i*tm1-v_pos)/(num_i-1)-tm1;
		dp2 = v_pos-tm1;
		U_i_n = U_i-(num_i-1)*mat3(dp1.x*dp1.x,dp1.x*dp1.y,dp1.x*dp1.z,dp1.y*dp1.x,dp1.y*dp1.y,dp1.y*dp1.z,dp1.z*dp1.x,dp1.z*dp1.y,dp1.z*dp1.z)-mat3(dp2.x*dp2.x,dp2.x*dp2.y,dp2.x*dp2.z,dp2.y*dp2.x,dp2.y*dp2.y,dp2.y*dp2.z,dp2.z*dp2.x,dp2.z*dp2.y,dp2.z*dp2.z);
		n_i = texture(seedMatrix,vec2(2,segId_i)).rgb;
		//n_i = n_i/num_i;
		
		
		float ng_index=0.5;
		vec4 ng_tex = texture(seedNeighbor,vec2(ng_index,voxel_gridId+0.5)).xyzw;
		segId_j = ng_tex.z;
		
		while(ng_tex.x>0 && ng_index<27)
		{
			if(ng_tex.w>0.5)
			{
				if(segId_j!=segId_i)
				{
					dp1 = texture(seedMatrix,vec2(3,segId_j)).rgb;
					dp2 = texture(seedMatrix,vec2(4,segId_j)).rgb;
				    U_j = mat3(dp1.x,dp1.y,dp1.z,dp1.y,dp2.x,dp2.y,dp1.z,dp2.y,dp2.z);
					tm1 = texture(seedMatrix,vec2(1,segId_j)).rgb;//pos_j
					num_j = texture(seedMatrix,vec2(0,segId_j)).g;
					dp1 = tm1-(num_j*tm1+v_pos)/(num_j+1);
					dp2 = v_pos-(num_j*tm1+v_pos)/(num_j+1);
					U_j_n = U_j+num_j*mat3(dp1.x*dp1.x,dp1.x*dp1.y,dp1.x*dp1.z,dp1.y*dp1.x,dp1.y*dp1.y,dp1.y*dp1.z,dp1.z*dp1.x,dp1.z*dp1.y,dp1.z*dp1.z)+mat3(dp2.x*dp2.x,dp2.x*dp2.y,dp2.x*dp2.z,dp2.y*dp2.x,dp2.y*dp2.y,dp2.y*dp2.z,dp2.z*dp2.x,dp2.z*dp2.y,dp2.z*dp2.z);
					n_j = texture(seedMatrix,vec2(2,segId_j)).rgb;
					//n_j = n_j/num_j;
					
		
					delta_eg_pos = U_i_n[0][0]+U_i_n[1][1]+U_i_n[2][2]+U_j_n[0][0]+U_j_n[1][1]+U_j_n[2][2]-(U_i[0][0]+U_i[1][1]+U_i[2][2]+U_j[0][0]+U_j[1][1]+U_j[2][2]);//CVT energy				
					delta_eg_fit = dot(normalize(n_i)*(U_i_n-U_i),normalize(n_i))+ dot(normalize(n_j)*(U_j_n-U_j),normalize(n_j));
					delta_eg_normal = dot(normalize(n_i)*v_N,normalize(n_i))-dot(normalize(n_j)*v_N,normalize(n_j));
					

					delta_eg = delta_eg_fit + lambda1*delta_eg_normal + lambda2*delta_eg_pos;
					//delta_eg = delta_eg_pos;
					//delta_eg = delta_eg_fit+lambda1*delta_eg_normal;
					
					if(delta_eg<min_dist)
					{
						min_dist = delta_eg;
						NearestSeedId = segId_j;
					}
				}
			}
			

			ng_index=ng_index+1;
			if(ng_index<27)
			{
				ng_tex = texture(seedNeighbor,vec2(ng_index,voxel_gridId+0.5)).xyzw;
				segId_j = ng_tex.z;
			}
				
		}
		
		if(min_dist<0)
		{
			FragSeedId = NearestSeedId;
		}else
		{
			FragSeedId = segId_i;
		}
	
	
	}
}