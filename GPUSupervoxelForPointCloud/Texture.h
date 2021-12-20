#pragma once
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include<ctime>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>

using namespace std;

class Texture
{
	GLFWwindow * window;

	int screenWidth, screenHeight;
	int width_t, height_t;
	int voxel_num;
	int ac_voxel_num;
	float width_ratio, height_ratio, depth_ratio;
	int seeds_num;
	int currt_seg_id;

	int current_buffer;
	float lambda_1;//curr_dist = curr_plane_dist+lambda_1*curr_normal_dist+lambda_2*curr_pos_dist;
	float lambda_2;
	float ratio;
	int min_size;
	GLuint frameBuffer_data;
	GLuint data_texture[3];
	GLenum buffers_data[3];
	GLuint frameBuffer_seed;
	GLuint seed_texture[2];
	GLenum buffers_seed[2];
	GLuint frameBuffer_flood;
	GLuint flood_texture[4];
	GLenum buffers_flood[4];

	//unsigned int VBOs[2], VAOs[2];
	unsigned int VAO_t, VBO_t;
	unsigned int VAO_p, VBO_p;

	int Lloyd_max_iter;
public:
	Texture();
	~Texture();
	int bit;
	void process_input(GLFWwindow *window);
	void set_variables();
	int init_texture(vector<float>& data_3d, vector<float>& normal_3d, vector<float>& voxel_gridId, vector<float>& seed_neighbors, vector<float>& seed_matrix, int width_, int height_,int ac_voxel_num,int min_size);
	void print_2D_buffer_RGB(int index); 
	void print_2D_buffer_RGBA(int index);
	void print_2D_buffer_seedMatrix(int index, int width_, int height_);
	void print_2D_buffer_seedNeighbor(int index, int width_, int height_);
	void print_2D_grid(int index);
	void print_seedNeighbor(vector<float>& seed_neighbors);

	void print_flood_RED(int index);
	void print_flood_RGBA(int index);
	void print_flood_RGB(int index);
	void seg(Shader& seg, int lloyd_curr_iter);
	void aver_surface_seg(Shader& aver, vector<float>& seeds_3d, vector<float>& seed_valid);
	void aver_surface_swap(Shader& aver,vector<float>& seed_cov_info, vector<float>& seed_valid);
	void cov_matrix(Shader& cov, vector<float>& seed_cov_info, vector<float>& seed_valid);
	void swapping(Shader& swapping,int iter);
	void update_params(float lambda1, float lambda2);
	void label(int*& labels);
	void update_seeds(vector<float>& seed_neighbors);
	void update_seed_matrix(vector<float>& seed_matrix);
};

