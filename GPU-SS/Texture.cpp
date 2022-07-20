#include "Texture.h"



Texture::Texture()
{
	current_buffer = 1;
	ratio = 0.5;
}


Texture::~Texture()
{
}


void Texture::process_input(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		printf("next step\n");
		Sleep(1000);
	}

}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


void Texture::set_variables()
{
	width_ratio = float(1.0) / float(width_t);
	height_ratio = float(1.0) / float(height_t);
	ratio = 1.0;
	current_buffer = 2;
	lambda_1 = 0.1;
	lambda_2 = 0.1;
}

int Texture::init_texture(vector<float>& data_3d, vector<float>& normal_3d, vector<float>& voxel_gridId, vector<float>& seed_neighbors, vector<float>& seed_cov, int width_, int height_,int ac_voxel_num_,int min_size_)
{
	width_t = width_;
	height_t = height_;
	min_size = min_size_;
	voxel_num = width_t * height_t;
	ac_voxel_num = ac_voxel_num_;
	//cout << ac_voxel_num << endl;
	currt_seg_id = 0;
	bit = 27;
	seeds_num = seed_neighbors.size() / (bit * 4);//x,y,id,valid

	set_variables();//set variables


	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	screenWidth = width_t;
	screenHeight = height_t;
	window = glfwCreateWindow(screenWidth, screenHeight, "GPU_Project", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwHideWindow(window);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	const GLubyte* OpenGLVersion = glGetString(GL_VERSION);
	printf("OpenGL version：%s\n", OpenGLVersion);
	int value;
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &value);
	printf("GL_MAX_3D_TEXTURE_SIZE：%d\n", value);
	

	frameBuffer_data = 0;
	buffers_data[0] = GL_COLOR_ATTACHMENT0;
	buffers_data[1] = GL_COLOR_ATTACHMENT1;
	buffers_data[2] = GL_COLOR_ATTACHMENT2;
	frameBuffer_seed = 0;
	buffers_seed[0] = GL_COLOR_ATTACHMENT0;
	buffers_seed[1] = GL_COLOR_ATTACHMENT1;
	
	frameBuffer_flood = 0;
	buffers_flood[0] = GL_COLOR_ATTACHMENT0;
	buffers_flood[1] = GL_COLOR_ATTACHMENT1;
	buffers_flood[2] = GL_COLOR_ATTACHMENT2;
	buffers_flood[3] = GL_COLOR_ATTACHMENT3;
	glGenFramebuffers(1, &frameBuffer_data);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_data);

	glGenTextures(1, data_texture + 0);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[0]);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width_t, height_t, 0, GL_RGB, GL_FLOAT, &data_3d[0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, data_texture[0], 0);

	//print_2D_buffer_RGB(0);
	

	glGenTextures(1, data_texture + 1);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[1]);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width_t, height_t, 0, GL_RGB, GL_FLOAT, &normal_3d[0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, data_texture[1], 0);


	//print_2D_buffer_RGB(1);


	glGenTextures(1, data_texture + 2);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[2]);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, width_t, height_t, 0, GL_RED, GL_FLOAT, &voxel_gridId[0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2, data_texture[2], 0);

	//print_2D_grid(2);
	/*glDrawBuffers(1, buffers_data + 2);
	glClearColor(5.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	print_2D_grid(2);*/
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("something wrong about data framebuffer!");
		return false;
	}


	glGenFramebuffers(1, &frameBuffer_seed);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_seed);
	//bit=27;2dx,y, id, valid
	glGenTextures(1, seed_texture);
	glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[0]);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, bit, seeds_num, 0, GL_RGBA, GL_FLOAT, &seed_neighbors[0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, seed_texture[0], 0);

	
	/*
	seed_matrix：
	<valid,num,-1>,<aver_pos(x,y,z)>,<sum_normal>,<cov_first3>,<cov_last3>,<N_first3>,<N_last3>
	   0,1,2,            3,4,5,         6,7,8,      9,10,11,    12,13,14,   15,16,17,  18,19,20
	*/
	glGenTextures(1, seed_texture + 1);
	glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[1]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, bit, seeds_num, 0, GL_RGB, GL_FLOAT, &seed_cov[0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, seed_texture[1], 0);

	//glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[0]);
	//print_2D_buffer_seedNeighbor(0, 27, seeds_num);
	////print_seedNeighbor(seed_neighbors);

	//glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[1]);
	//print_2D_buffer_seedMatrix(1,27,seeds_num);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("something wrong about seed framebuffer!");
		return false;
	}


	glGenFramebuffers(1, &frameBuffer_flood);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);

	glGenTextures(1, flood_texture);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[0]);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, width_t, height_t, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, flood_texture[0], 0);
	glDrawBuffers(1, buffers_flood);
	glClearColor(0.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenTextures(1, flood_texture+1);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[1]);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, width_t, height_t, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+1, flood_texture[1], 0);
	glDrawBuffers(1, buffers_flood+1);
	glClearColor(1.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glGenTextures(1, flood_texture + 2);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[2]);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, width_t, height_t, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2, flood_texture[2], 0);
	glDrawBuffers(1, buffers_flood+2);
	glClearColor(1.0f, 2.0f, 3.0f, 4.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glGenTextures(1, flood_texture + 3);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[3]);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width_t, height_t, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 3, flood_texture[3], 0);
	glDrawBuffers(1, buffers_flood+3);
	glClearColor(1.0f, 22.0f, 33.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/*glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[0]);
	print_flood_RED(0);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[1]);
	print_flood_RED(1);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[2]);
	print_flood_RGBA(2);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[3]);
	print_flood_RGB(3);*/


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("something wrong about flood framebuffer!");
		return false;
	}
	
	float quad_vertices[] = {
		-1.0, 1.0, 1.0, 0.0, 0.0, 0.0, height_t,//upper left
		1.0, 1.0, 0.0, 1.0, 0.0, width_t, height_t,//upper right
		1.0, -1.0, 0.0, 0.0, 1.0, width_t, 0.0,//lower right
		-1.0, 1.0, 1.0, 0.0, 0.0, 0.0, height_t,//upper left
		1.0, -1.0, 0.0, 0.0, 1.0, width_t, 0.0,//lower right
		-1.0, -1.0, 1.0, 1.0, 0.0, 0.0, 0.0//lower left
	};
	glGenVertexArrays(1, &VAO_t);
	glGenBuffers(1, &VBO_t);

	// quad setup
	// ----------
	glBindVertexArray(VAO_t);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_t);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	vector<float> vertices;
	vertices.reserve(width_t*height_t * 2);
	for (int j = 0; j < height_t; j++)
	{
		for (int i = 0; i < width_t; i++)
		{
			vertices.push_back(i + 0.5);
			vertices.push_back(j + 0.5);
		}
	}

	glGenVertexArrays(1, &VAO_p);
	glGenBuffers(1, &VBO_p);
	glBindVertexArray(VAO_p);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_p);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);//2*sizeof(float)
	glEnableVertexAttribArray(0);



	return 0;
}

void Texture::print_seedNeighbor(vector<float>& seed_neighbors)
{
	string txtname = "seed_neighbors.txt";
	ofstream f_debug(txtname);
	for (int i = 0; i <seeds_num; i++)
	{
		for (int j = 0; j < 27; j++)
		{
			int id = i * 27 + j;
			f_debug << std::left << std::setw(4) << seed_neighbors[id*4] << " ";
			f_debug << std::left << std::setw(4) << seed_neighbors[id*4 + 1] << " ";
			f_debug << std::left << std::setw(4) << seed_neighbors[id* 4 + 2] << " ";
			f_debug << std::left << std::setw(4) << seed_neighbors[id* 4 + 3] << " ";
		}
		f_debug << endl;
	}
	f_debug.close();
}

void Texture::seg(Shader& seg, int lloyd_curr_iter)
{
	seg.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	glDrawBuffers(1, buffers_flood);
	glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[0]);
	seg.setInt("voxelPos", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[1]);
	seg.setInt("voxelNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[2]);
	seg.setInt("voxelGrid", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[0]);
	seg.setInt("seedNeighbor", 3);
	seg.setInt("seedNeighbor", 3);
	seg.setFloat("lambda_1", lambda_1);
	seg.setFloat("lambda_2", lambda_2);
	glBindVertexArray(VAO_t);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	currt_seg_id = 0;//save seg imn flood_texture[0]

	//print_flood_RED(0);
}
void Texture::aver_surface_seg(Shader& aver, vector<float>& seeds_3d, vector<float>& seed_valid)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	aver.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	glDrawBuffers(1, buffers_flood + 2);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[currt_seg_id]);
	aver.setInt("seg", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[0]);
	aver.setInt("voxelPos", 1);
	aver.setFloat("width", float(width_t));
	aver.setFloat("height", float(height_t));
	aver.setFloat("seeds_num", float(seeds_num));
	glBindVertexArray(VAO_p);
	glDrawArrays(GL_POINTS, 0, width_t*height_t);
	glDisable(GL_BLEND);
	/*print_flood_RGBA(2);*/

	int read_height_t = seeds_num / width_t + 1;
	float* res = new float[width_t*read_height_t * 4];
	glReadBuffer(buffers_flood[2]);
	glReadPixels(0, 0, width_t, read_height_t, GL_RGBA, GL_FLOAT, res);
	seeds_3d.clear();
	seeds_3d.resize(seeds_num * 4, -1);
	int dis_num = 0;
	for (int i = 0; i < seeds_num; i++)
	{
		if (res[i * 4 + 3] < 1)
		{
			seed_valid[i] = 0;
			seeds_3d[i * 4] = 0;
			seeds_3d[i * 4 + 1] = 0;
			seeds_3d[i * 4 + 2] = 0;
			dis_num++;
			//cout << i << "disappear:"<< res[i * 4 + 3] << endl;
		}
		else
		{
			seed_valid[i] = 1;
			seeds_3d[i * 4] = res[i * 4] / res[i * 4 + 3];
			seeds_3d[i * 4 + 1] = res[i * 4 + 1] / res[i * 4 + 3];
			seeds_3d[i * 4 + 2] = res[i * 4 + 2] / res[i * 4 + 3];
		}
		seeds_3d[i * 4 + 3] = float(i);
		//cout <<seeds_3d[i * 4 + 3]<<":"<< seeds_3d[i * 4] << "," << seeds_3d[i * 4 + 1] << "," << seeds_3d[i * 4 + 2]<< endl;
	}
	cout << "disappear seeds number:" << dis_num << endl;
	if (res) delete[] res;
}

void Texture::aver_surface_swap(Shader& aver, vector<float>& seed_cov_info, vector<float>& seed_valid)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	aver.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	GLenum draw2[2] = { GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	glDrawBuffers(2, draw2);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[currt_seg_id]);
	aver.setInt("seg", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[0]);
	aver.setInt("voxelPos", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[1]);
	aver.setInt("voxelNormal", 2);
	aver.setFloat("width", float(width_t));
	aver.setFloat("height", float(height_t));
	aver.setFloat("seeds_num", float(seeds_num));
	glBindVertexArray(VAO_p);
	glDrawArrays(GL_POINTS, 0, width_t*height_t);
	glDisable(GL_BLEND);
	/*glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[2]);
	print_flood_RGBA(2);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[3]);
	print_flood_RGB(3);*/

	/*
	seed_matrix:
	<valid,num,-1>,<aver_pos(x,y,z)>,<sum_normal>,<cov_first3>,<cov_last3>,<N_first3>,<N_last3>
	   0,1,2,            3,4,5,         6,7,8,      9,10,11,    12,13,14,   15,16,17,  18,19,20     
	*/
	int read_height_t = seeds_num / width_t + 1;
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[2]);
	float* res1 = new float[width_t*read_height_t * 4];
	glReadBuffer(buffers_flood[2]);
	glReadPixels(0, 0, width_t, read_height_t, GL_RGBA, GL_FLOAT, res1);

	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[3]);
	float* res2 = new float[width_t*read_height_t * 3];
	glReadBuffer(buffers_flood[3]);
	glReadPixels(0, 0, width_t, read_height_t, GL_RGB, GL_FLOAT, res2);
	
	int rr = bit * 3;
	seed_cov_info.clear();
	seed_cov_info.resize(seeds_num*rr, -1);
	int dis_num=0;
	for (int i = 0; i < seeds_num; i++)
	{
		if (res1[i * 4 + 3] <= 1)
		{
			seed_valid[i] = 0;
			//cout << i << "disappear!" << endl;
			seed_cov_info[i * rr] = 0;//invalid
			dis_num++;
		}
		else
		{
			seed_valid[i] = 1;
			seed_cov_info[i * rr] = 1;
			seed_cov_info[i * rr+1] = res1[i * 4 + 3];
			seed_cov_info[i * rr + 3] = res1[i * 4]/ res1[i * 4 + 3];
			seed_cov_info[i * rr + 4] = res1[i * 4+1]/ res1[i * 4 + 3];
			seed_cov_info[i * rr + 5] = res1[i * 4+2]/ res1[i * 4 + 3];
			seed_cov_info[i * rr + 6] = res2[i * 3];
			seed_cov_info[i * rr + 7] = res2[i * 3 + 1];
			seed_cov_info[i * rr + 8] = res2[i * 3 + 2];
		}
	
		//cout <<seeds_3d[i * 4 + 3]<<":"<< seeds_3d[i * 4] << "," << seeds_3d[i * 4 + 1] << "," << seeds_3d[i * 4 + 2]<< endl;
	}
	
	cout << "disappear seeds number:" << dis_num << endl;
	if (res1) delete[] res1;
	if (res2) delete[] res2;
}


void Texture::cov_matrix(Shader& cov, vector<float>& seed_cov_info, vector<float>& seed_valid)
{
	int read_height_t = seeds_num / width_t + 1;
	float* res1 = new float[width_t*read_height_t * 4];
	float* res2 = new float[width_t*read_height_t * 3];
	int cluster_size = 0;
	int rr = bit * 3;
	int area = 0;
	vector<float> seed_cov;//covariance matrix, U,seeds_num*6
	//vector<float> seed_det;//covariance determinant
	//seed_det.resize(seeds_num, -1);
	seed_cov.resize(seeds_num * 6, -1);

	//1. calculate the first 3 elements of covariance matrix
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	cov.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	GLenum draw2[2] = { GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(2, draw2);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[currt_seg_id]);
	cov.setInt("seg", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[1]);
	cov.setInt("seedMatrix", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[0]);
	cov.setInt("voxelPos", 2);
	cov.setFloat("width", float(width_t));
	cov.setFloat("height", float(height_t));
	cov.setFloat("seeds_num", float(seeds_num));
	glBindVertexArray(VAO_p);
	glDrawArrays(GL_POINTS, 0, width_t*height_t);
	glDisable(GL_BLEND);

	/*glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[2]);
	print_flood_RGBA(2);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[3]);
	print_flood_RGB(3);*/

	/*
	seed_matrix：
	<valid,num,-1>,<aver_pos(x,y,z)>,<sum_normal>,<cov_first3>,<cov_last3>,<N_first3>,<N_last3>
	   0,1,2,            3,4,5,         6,7,8,      9,10,11,    12,13,14,   15,16,17,  18,19,20
	*/
	
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[2]);
	glReadBuffer(buffers_flood[2]);
	glReadPixels(0, 0, width_t, read_height_t, GL_RGBA, GL_FLOAT, res1);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[3]);
	glReadBuffer(buffers_flood[3]);
	glReadPixels(0, 0, width_t, read_height_t, GL_RGB, GL_FLOAT, res2);

	
	for (int i = 0; i < seeds_num; i++)
	{
		area = res1[i * 4 + 3];
		if (area > 1)
		{
			seed_cov_info[i * rr + 9] = res1[i * 4]/ area;
			seed_cov_info[i * rr + 10] = res1[i * 4 + 1]/ area;
			seed_cov_info[i * rr + 11] = res1[i * 4 + 2]/ area;
			seed_cov_info[i * rr + 12] = res2[i * 3]/ area;
			seed_cov_info[i * rr + 13] = res2[i * 3 + 1]/ area;
			seed_cov_info[i * rr + 14] = res2[i * 3 + 2]/ area;
		}
		else
		{
			seed_valid[i] = 0;
		}
	}
	
	if (res1) delete[] res1;
	if (res2) delete[] res2;
}

void Texture::update_params(float lambda1, float lambda2)
{
	lambda_1 = lambda1;
	lambda_2 = lambda2;
}

void Texture::swapping(Shader& swapping,int iter)
{
	swapping.use();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	glDrawBuffers(1, buffers_flood+(1-currt_seg_id));
	glClearColor(-1.0f, -1.0f, -1.0f, -1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, flood_texture[currt_seg_id]);
	swapping.setInt("seg", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[0]);
	swapping.setInt("voxelPos", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[1]);
	swapping.setInt("voxelNormal", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_RECTANGLE, data_texture[2]);
	swapping.setInt("voxelGrid", 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[0]);
	swapping.setInt("seedNeighbor", 4);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[1]);
	swapping.setInt("seedMatrix", 5);
	swapping.setFloat("lambda1", lambda_1);
	swapping.setFloat("lambda2", lambda_2);
	glBindVertexArray(VAO_t);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	currt_seg_id = 1 - currt_seg_id;

	//print_flood_RED(currt_seg_id);
}

void Texture::update_seed_matrix(vector<float>& seed_cov_info)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_seed);
	glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[1]);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, bit, seeds_num, 0, GL_RGB, GL_FLOAT, &seed_cov_info[0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, seed_texture[1], 0);
	//print_2D_buffer_seedMatrix(1, bit, seeds_num);
}

void Texture::update_seeds(vector<float>& seed_neighbors)
{
	/*for (int i = 0; i < seed_neighbors.size() / 4; i++)
	{
		cout << i << ":" << seed_neighbors[i * 4] << "," << seed_neighbors[i * 4 + 1] << "," << seed_neighbors[i * 4 + 2] << "," << seed_neighbors[i * 4 + 3] << endl;
	}*/

	//print_seedNeighbor(seed_neighbors);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_seed);
	glBindTexture(GL_TEXTURE_RECTANGLE, seed_texture[0]);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, 27, seeds_num, 0, GL_RGBA, GL_FLOAT, &seed_neighbors[0]);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, seed_texture[0], 0);
	//print_2D_buffer_seedNeighbor(0, 27, seeds_num);
}

void Texture::label(int*& labels)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_flood);
	float* res1 = new float[width_t*height_t];
	glReadBuffer(buffers_flood[currt_seg_id]);
	glReadPixels(0, 0, width_t, height_t, GL_RED, GL_FLOAT, res1);
	int nolabel_num = 0;
	for (int i = 0; i < width_t*height_t; i++)
	{
		if (int(res1[i]) < 0 && i < ac_voxel_num)
		{
			//cout << i << "<0!" << endl;
			nolabel_num++;
			labels[i] = 0;
		}
		else
		{
			labels[i] = int(res1[i]);
		}
			
		
	}
	cout << "number of voxels without label:" << nolabel_num << endl;
	if (res1) delete[] res1;
}


void Texture::print_2D_buffer_RGB(int index)
{
	float* res1 = new float[width_t*height_t * 3];
	glReadBuffer(buffers_data[index]);
	glReadPixels(0, 0, width_t, height_t, GL_RGB, GL_FLOAT, res1);
	string txtname = "data_" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	for (int ii = height_t - 1; ii >= 0; ii--) {
		for (int jj = 0; jj < width_t; jj++) {
			f_debug1 << std::setprecision(6) << res1[3 * (ii * width_t + jj)] << " ";
			f_debug1 << std::setprecision(6) << res1[3 * (ii * width_t + jj) + 1] << " ";
			f_debug1 << std::setprecision(6) << res1[3 * (ii * width_t + jj) + 2] << " ";
		}
		f_debug1 << endl;
	}
	f_debug1.close();
	if (res1) delete[] res1;
}

void Texture::print_2D_buffer_RGBA(int index)
{
	float* res1 = new float[width_t*height_t * 4];
	glReadBuffer(buffers_data[index]);
	glReadPixels(0, 0, width_t, height_t, GL_RGBA, GL_FLOAT, res1);
	string txtname = "data_" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	for (int ii = height_t - 1; ii >= 0; ii--) {
		for (int jj = 0; jj < width_t; jj++) {
			f_debug1 << std::setprecision(6) << res1[4 * (ii * width_t + jj)] << " ";
			f_debug1 << std::setprecision(6) << res1[4 * (ii * width_t + jj) + 1] << " ";
			f_debug1 << std::setprecision(6) << res1[4 * (ii * width_t + jj) + 2] << " ";
			f_debug1 << std::setprecision(6) << res1[4 * (ii * width_t + jj) + 3] << " ";
		}
		f_debug1 << endl;
	}
	f_debug1.close();
	if (res1) delete[] res1;
}


void Texture::print_2D_buffer_seedNeighbor(int index, int width_, int height_)
{
	float* res1 = new float[width_*height_ * 4];
	glReadBuffer(buffers_data[index]);
	glReadPixels(0, 0, width_, height_, GL_RGBA, GL_FLOAT, res1);
	string txtname = "seedNeighbor_" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	int ind = 0;
	for (int ii = 0; ii<height_; ii++) {
		ind = ii * width_ * 4;
		for (int jj = 0; jj < width_; jj++) {
			f_debug1 << std::left << std::setw(4) << res1[ind++] << " ";
			f_debug1 << std::left << std::setw(4) <<res1[ind++] << " ";
			f_debug1 << std::left << std::setw(4) <<res1[ind++] << " ";
			f_debug1 << std::left << std::setw(4) <<res1[ind++] << " ";

		}
		f_debug1 << endl;
	}
	f_debug1.close();
	if (res1) delete[] res1;
}

void Texture::print_2D_buffer_seedMatrix(int index, int width_, int height_)
{
	float* res1 = new float[width_*height_*3];
	glReadBuffer(buffers_data[index]);
	glReadPixels(0, 0, width_, height_, GL_RGB, GL_FLOAT, res1);
	string txtname = "seedMatrix_" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	int id = 0;
	for (int ii =0; ii < height_; ii++) {
		id = ii * width_*3;
		for (int jj = 0; jj < width_; jj++) {
			f_debug1 << std::left << std::setw(4) << res1[id++] << " ";
			f_debug1 << std::left << std::setw(4) << res1[id++] << " ";
			f_debug1 << std::left << std::setw(4) << res1[id++] << " ";

		}
		f_debug1 << endl;
	}
	f_debug1.close();
	if (res1) delete[] res1;
}

void Texture::print_2D_grid(int index)
{
	float* res1 = new float[width_t*height_t];
	glReadBuffer(buffers_data[index]);
	glReadPixels(0, 0, width_t, height_t, GL_RED, GL_FLOAT, res1);
	string txtname = "grid_" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	for (int ii = height_t - 1; ii >= 0; ii--) {
		for (int jj = 0; jj < width_t; jj++) {
			f_debug1 << std::left << std::setw(4) << res1[ii * width_t + jj] << " ";

		}
		f_debug1 << endl;
	}
	f_debug1.close();
	if (res1) delete[] res1;
}



void Texture::print_flood_RED(int index)
{
	float* res1 = new float[width_t*height_t];
	glReadBuffer(buffers_flood[index]);
	glReadPixels(0, 0, width_t, height_t, GL_RED, GL_FLOAT, res1);
	string txtname = "flood_" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	for (int ii = height_t - 1; ii >= 0; ii--) {
		for (int jj = 0; jj < width_t; jj++) {
			f_debug1 << std::left << std::setw(4) << res1[ii * width_t + jj] << " ";

		}
		f_debug1 << endl;
	}
	f_debug1.close();
	if (res1) delete[] res1;
}

void Texture::print_flood_RGBA(int index)
{
	float* res1 = new float[width_t*height_t * 4];
	glReadBuffer(buffers_flood[index]);
	glReadPixels(0, 0, width_t, height_t, GL_RGBA, GL_FLOAT, res1);
	string txtname = "flood_" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	for (int ii = height_t - 1; ii >= 0; ii--) {
		for (int jj = 0; jj < width_t; jj++) {
			f_debug1 << std::setprecision(6) << res1[4 * (ii * width_t + jj)] << " ";
			f_debug1 << std::setprecision(6) << res1[4 * (ii * width_t + jj) + 1] << " ";
			f_debug1 << std::setprecision(6) << res1[4 * (ii * width_t + jj) + 2] << " ";
			f_debug1 << std::setprecision(6) << res1[4 * (ii * width_t + jj) + 3] << " ";
		}
		f_debug1 << endl;
	}
	f_debug1.close();
	if (res1) delete[] res1;
}

void Texture::print_flood_RGB(int index)
{
	float* res1 = new float[width_t*height_t * 3];
	glReadBuffer(buffers_flood[index]);
	glReadPixels(0, 0, width_t, height_t, GL_RGB, GL_FLOAT, res1);
	string txtname = "flood_" + std::to_string(index) + ".txt";
	ofstream f_debug1(txtname);
	for (int ii = height_t - 1; ii >= 0; ii--) {
		for (int jj = 0; jj < width_t; jj++) {
			f_debug1 << std::setprecision(6) << res1[3 * (ii * width_t + jj)] << " ";
			f_debug1 << std::setprecision(6) << res1[3 * (ii * width_t + jj) + 1] << " ";
			f_debug1 << std::setprecision(6) << res1[3 * (ii * width_t + jj) + 2] << " ";
		}
		f_debug1 << endl;
	}
	f_debug1.close();
	if (res1) delete[] res1;
}
