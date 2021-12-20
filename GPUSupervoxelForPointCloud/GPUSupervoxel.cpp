#include "GPUSupervoxel.h"
#include<iostream>
#include <numeric> 


GPUSupervoxel::GPUSupervoxel()
{
}


GPUSupervoxel::~GPUSupervoxel()
{
}


void GPUSupervoxel::voxelization(const std::vector<double> &points,
	const std::vector<double> &normals, double voxelResolution_, double seedResolution_)
{
	voxelResolution = voxelResolution_;
	seedResolution = seedResolution_;

	
	// step 1: init
	// step 1.1: bounding box
	//double bbox[6];
	bbox[0] = bbox[2] = bbox[4] = 1e100;
	bbox[1] = bbox[3] = bbox[5] = -1e100;

	int vnb = (int)points.size() / 3;
	for (int v = 0; v < vnb; ++v)
	{
		const double *p = &points[3 * v];

		if (p[0] < bbox[0]) bbox[0] = p[0];
		if (p[0] > bbox[1]) bbox[1] = p[0];
		if (p[1] < bbox[2]) bbox[2] = p[1];
		if (p[1] > bbox[3]) bbox[3] = p[1];
		if (p[2] < bbox[4]) bbox[4] = p[2];
		if (p[2] > bbox[5]) bbox[5] = p[2];
	}

	// step 1.2: voxelize
	width = int((bbox[1] - bbox[0]) / voxelResolution + 1);
	height = int((bbox[3] - bbox[2]) / voxelResolution + 1);
	depth = int((bbox[5] - bbox[4]) / voxelResolution + 1);
	cout << "voxel resolution (width:" << width << ",height:" << height << ",depth:" << depth<<")" << endl;
	int slice_num = width * height;
	int voxel_num = slice_num * depth;



	voxels.clear();
	// voxel centroids and normals
	for (int v = 0; v < vnb; ++v)
	{
		const double *p = &points[3 * v];
		const double *pn = &normals[3 * v];

		int i = int((p[0] - bbox[0]) / voxelResolution);
		int j = int((p[1] - bbox[2]) / voxelResolution);
		int k = int((p[2] - bbox[4]) / voxelResolution);

		i = i >= width ? width - 1 : i;
		i = i < 0 ? 0 : i;
		j = j >= height ? height - 1 : j;
		j = j < 0 ? 0 : j;
		k = k >= depth ? depth - 1 : k;
		k = k < 0 ? 0 : k;

		int index = k * width * height + j * width + i;
		int pos = -1;

		//valid_flag[index] = 1;

		std::unordered_map<int, int>::iterator mit = voxelMap.find(index);
		if (mit == voxelMap.end())
		{
			pos = (int)voxels.size();
			voxels.push_back(Voxel());

			voxels[pos].index = pos;//valid index of voxel
			voxels[pos].id = index;//3d index
			voxels[pos].i = i;
			voxels[pos].j = j;
			voxels[pos].k = k;

			voxels[pos].centroid[0] = 0.0;
			voxels[pos].centroid[1] = 0.0;
			voxels[pos].centroid[2] = 0.0;
			voxels[pos].normal[0] = 0.0;
			voxels[pos].normal[1] = 0.0;
			voxels[pos].normal[2] = 0.0;

			voxels[pos].members.clear();

			voxelMap.insert(std::make_pair(index, pos));//3d index, valid index
		}
		else
			pos = mit->second;

		voxels[pos].members.push_back(v);
		voxels[pos].centroid[0] += p[0];
		voxels[pos].centroid[1] += p[1];
		voxels[pos].centroid[2] += p[2];

		voxels[pos].normal[0] += pn[0];
		voxels[pos].normal[1] += pn[1];
		voxels[pos].normal[2] += pn[2];
	}
	voxels.shrink_to_fit();
	int voxelNumber = (int)voxels.size();
	width_t = int(sqrt(voxelNumber)) + 1;
	height_t = width_t;
	cout << "voxelNumber:" << voxelNumber << ",width_2dTexture:" << width_t << ",height_2dTexture:" << height_t << endl;
	s = int(width_t);




	data_3d.resize(width_t*height_t * 3, 0);
	normal_3d.resize(width_t*height_t * 3, 0);
	for (int pos = 0; pos < voxelNumber; ++pos)
	{
		int memberNumber = (int)voxels[pos].members.size(); // > 0

		voxels[pos].centroid[0] /= memberNumber;
		voxels[pos].centroid[1] /= memberNumber;
		voxels[pos].centroid[2] /= memberNumber;

		double nx = voxels[pos].normal[0] / memberNumber;
		double ny = voxels[pos].normal[1] / memberNumber;
		double nz = voxels[pos].normal[2] / memberNumber;
		double norm = std::sqrt(nx * nx + ny * ny + nz * nz);

		if (norm == 0.0)
		{
			voxels[pos].normal[0] = 1.0;
			voxels[pos].normal[1] = 0;
			voxels[pos].normal[2] = 0;
		}
		else
		{
			voxels[pos].normal[0] = nx / norm;
			voxels[pos].normal[1] = ny / norm;
			voxels[pos].normal[2] = nz / norm;
		}

		data_3d[pos * 3] = voxels[pos].centroid[0];
		data_3d[pos * 3 + 1] = voxels[pos].centroid[1];
		data_3d[pos * 3 + 2] = voxels[pos].centroid[2];
		normal_3d[pos * 3] = voxels[pos].normal[0];
		normal_3d[pos * 3 + 1] = voxels[pos].normal[1];
		normal_3d[pos * 3 + 2] = voxels[pos].normal[2];

	}



	// step 1.3: init seeds
	superWidth = int((bbox[1] - bbox[0]) / seedResolution + 1);
	superHeight = int((bbox[3] - bbox[2]) / seedResolution + 1);
	superDepth = int((bbox[5] - bbox[4]) / seedResolution + 1);
	cout << "supervoxel resolution (superWidth:" << superWidth << ",superHeight:" << superHeight << ",superDepth:" << superDepth<<")" << endl;


	std::vector<double> superCentroids;
	std::vector<double> nearestDist;
	vector<int> s_vnum;
	int aver_vnum = 0;
	int spos = -1;
	int snum = 0;

	for (int pos = 0; pos < voxelNumber; ++pos)
	{
		const double *voxcent = voxels[pos].centroid;

		int si = int((voxcent[0] - bbox[0]) / seedResolution);
		int sj = int((voxcent[1] - bbox[2]) / seedResolution);
		int sk = int((voxcent[2] - bbox[4]) / seedResolution);

		si = si >= superWidth ? superWidth - 1 : si;
		si = si < 0 ? 0 : si;
		sj = sj >= superHeight ? superHeight - 1 : sj;
		sj = sj < 0 ? 0 : sj;
		sk = sk >= superDepth ? superDepth - 1 : sk;
		sk = sk < 0 ? 0 : sk;

		int sid = sk * superWidth * superHeight + sj * superWidth + si;

		std::unordered_map<int, int>::iterator smit = supervoxelMap.find(sid);
		if (smit == supervoxelMap.end())
		{
			spos = (int)supervoxels.size();
			Supervoxel supervoxel;
			supervoxel.index = spos;
			supervoxel.id = sid;
			supervoxels.push_back(supervoxel);
			superCentroids.push_back(bbox[0] + double(si + 0.5) * seedResolution);
			superCentroids.push_back(bbox[2] + double(sj + 0.5) * seedResolution);
			superCentroids.push_back(bbox[4] + double(sk + 0.5) * seedResolution);
			nearestDist.push_back(1e100);
			s_vnum.push_back(0);
			supervoxelMap.insert(std::make_pair(sid, spos));
		}
		else
			spos = smit->second;

		s_vnum[spos]++;
		voxels[pos].grid = spos;
		double dx = superCentroids[3 * spos] - voxcent[0];
		double dy = superCentroids[3 * spos + 1] - voxcent[1];
		double dz = superCentroids[3 * spos + 2] - voxcent[2];
		double dist = (dx * dx + dy * dy + dz * dz);
		if (dist < nearestDist[spos])
		{
			nearestDist[spos] = dist;
			supervoxels[spos].centroid = &(voxels[pos]);
		}
	}
	seeds_num = supervoxels.size();
	cout << "seed number:" << seeds_num << endl;
	seeds_t.resize(seeds_num * 3, -1);//xyid

	int index_w, index_h;
	for (int spos = 0; spos < seeds_num; ++spos)
	{
		index_h = int(supervoxels[spos].centroid->index / width_t);
		index_w = supervoxels[spos].centroid->index - index_h * width_t;
		seeds_t[spos * 3] = float(index_w + 0.5);
		seeds_t[spos * 3 + 1] = float(index_h + 0.5);
		seeds_t[spos * 3 + 2] = float(spos);
		//cout << supervoxels[spos].centroid->i << "," << supervoxels[spos].centroid->j << "," << supervoxels[spos].centroid->k << endl;
		aver_vnum += s_vnum[spos];
	}
	aver_vnum = aver_vnum / seeds_num;
	min_size = aver_vnum / 4;
	seed_neighbors.resize(seeds_num * 27 * 4, -1);//27 neighbors at most
	seed_neighbor_map.resize(seeds_num * 27, -1);
	seed_valid.resize(seeds_num, 1);
	int superSlice = superWidth * superHeight;
	int off_set_slice[3] = { -superSlice,0,superSlice };
	int off_set[9] = { -superWidth - 1,-superWidth,-superWidth + 1,-1,0,1,superWidth - 1,superWidth,superWidth + 1 };	int sid_max = superWidth * superHeight*superDepth;

	for (int spos = 0; spos < seeds_num; ++spos)
	{
		int id = supervoxels[spos].id;
		int curr_id;
		int ng_id;
		int ng_num = 0;
		for (int k = 0; k < 3; k++)
		{
			curr_id = id + off_set_slice[k];
			for (int i = 0; i < 9; i++)
			{
				ng_id = curr_id + off_set[i];
				if (ng_id >= 0 && ng_id < sid_max)
				{
					std::unordered_map<int, int>::iterator smit = supervoxelMap.find(ng_id);
					if (smit != supervoxelMap.end())
					{
						int start_sn = spos * 27 + ng_num;
						int id2 = smit->second;
						seed_neighbors[start_sn * 4] = seeds_t[id2 * 3];
						seed_neighbors[start_sn * 4 + 1] = seeds_t[id2 * 3 + 1];
						seed_neighbors[start_sn * 4 + 2] = seeds_t[id2 * 3 + 2];
						seed_neighbors[start_sn * 4 + 3] = seed_valid[id2];
						seed_neighbor_map[start_sn] = id2;//unchanged during algorithm
						ng_num++;
					}
				}
			}
		}
	}

	voxel_gridId.resize(width_t*height_t, -1);
	for (int pos = 0; pos < voxelNumber; ++pos)
	{
		voxel_gridId[pos] = voxels[pos].grid;
	}

	seed_cov_info.resize(seeds_num * 27 * 3, -1);
}

void GPUSupervoxel::print_valid(vector<int>& valid_flag)
{
	string txt_name = "valid_flag.txt";
	ofstream f_debug(txt_name);
	int slice_index = 0;
	for (int t = 0; t < depth; t++)//depth
	{
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {

				f_debug << std::left << std::setw(4) << valid_flag[slice_index++] << " ";
			}
			f_debug << endl;
		}
		f_debug << endl;
		f_debug << endl;
	}

	f_debug.close();
}
void GPUSupervoxel::nearest_valid_seed_voxel(std::vector<float>& seeds_3d, int Lloyd_curr_iter)
{
	if (Lloyd_curr_iter % 2 == 0)
	{
		int fail_num = 0;//fail to find the nearest voxel in preset radius
		for (int t = 0; t < seeds_num; t++)
		{
			if (seed_valid[t] > 0)
			{
				int i = int((seeds_3d[t * 4] - bbox[0]) / voxelResolution);
				int j = int((seeds_3d[t * 4 + 1] - bbox[2]) / voxelResolution);
				int k = int((seeds_3d[t * 4 + 2] - bbox[4]) / voxelResolution);


				i = i >= width ? width - 1 : i;
				i = i < 0 ? 0 : i;
				j = j >= height ? height - 1 : j;
				j = j < 0 ? 0 : j;
				k = k >= depth ? depth - 1 : k;
				k = k < 0 ? 0 : k;

				//cout << i << "," << j << "," << k << endl;

				int index_h = 0, index_w = 0;
				int radius = 1;
				bool found = false;
				while (true)
				{
					for (int a = -radius; a <= radius; ++a)
					{
						int na = k + a;
						if (na < 0 || na >= depth)
							continue;

						for (int b = -radius; b <= radius; ++b)
						{
							int nb = j + b;
							if (nb < 0 || nb >= height)
								continue;

							for (int c = -radius; c <= radius; ++c)
							{
								int nc = i + c;
								if (nc < 0 || nc >= width)
									continue;

								if (abs(a) != radius && abs(b) != radius && abs(c) != radius)
									continue;

								//int nindex = nc * width * height + nb * width + na;
								int nindex = na * width * height + nb * width + nc;
								std::unordered_map<int, int>::iterator mit = voxelMap.find(nindex);
								if (mit == voxelMap.end())
									continue;

								int npos = mit->second;
								found = true;

								index_h = int(npos / width_t);
								index_w = npos - index_h * width_t;
								seeds_t[t * 3] = float(index_w + 0.5);
								seeds_t[t * 3 + 1] = float(index_h + 0.5);

								break;
							}
							if (found)
								break;
						}
						if (found)
							break;
					}
					if (found)
						break;
					++radius;
					if (radius > 10)
					{
						fail_num++;
						break;
					}

				}
				//cout << radius << endl;
			}
		}
		//cout << "fail_num:" << fail_num << endl;
	}
	else
	{
		int fail_num = 0;
		for (int t = 0; t < seeds_num; t++)
		{
			if (seed_valid[t] > 0)
			{
				int i = int((seeds_3d[t * 4] - bbox[0]) / voxelResolution);
				int j = int((seeds_3d[t * 4 + 1] - bbox[2]) / voxelResolution);
				int k = int((seeds_3d[t * 4 + 2] - bbox[4]) / voxelResolution);


				i = i >= width ? width - 1 : i;
				i = i < 0 ? 0 : i;
				j = j >= height ? height - 1 : j;
				j = j < 0 ? 0 : j;
				k = k >= depth ? depth - 1 : k;
				k = k < 0 ? 0 : k;

				//cout << i << "," << j << "," << k << endl;
				int index_h = 0, index_w = 0;
				int radius = 1;
				bool found = false;
				while (true)
				{
					for (int a = radius; a >= -radius; --a)
					{
						int na = k + a;
						if (na < 0 || na >= depth)
							continue;

						for (int b = radius; b >= -radius; --b)
						{
							int nb = j + b;
							if (nb < 0 || nb >= height)
								continue;

							for (int c = radius; c >= -radius; --c)
							{
								int nc = i + c;
								if (nc < 0 || nc >= width)
									continue;

								if (abs(a) != radius && abs(b) != radius && abs(c) != radius)
									continue;

								int nindex = na * width * height + nb * width + nc;
								std::unordered_map<int, int>::iterator mit = voxelMap.find(nindex);
								if (mit == voxelMap.end())
									continue;

								int npos = mit->second;
								found = true;
								index_h = int(npos / width_t);
								index_w = npos - index_h * width_t;
								seeds_t[t * 3] = float(index_w + 0.5);
								seeds_t[t * 3 + 1] = float(index_h + 0.5);
								break;
							}
							if (found)
								break;
						}
						if (found)
							break;
					}
					if (found)
						break;
					++radius;
					if (radius > 10)
					{
						fail_num++;
						break;
					}

				}
				//cout << radius << endl;

			}

		}
		//cout << "fail_num:" << fail_num << endl;
	}
}
void GPUSupervoxel::update_seeds_neighbors()
{
	for (int spos = 0; spos < seeds_num; ++spos)
	{
		for (int i = 0; i < 27; i++)
		{
			int index = spos * 27 + i;
			int id2 = seed_neighbor_map[index];
			if (id2 > -1)
			{
				seed_neighbors[index * 4] = seeds_t[id2 * 3];
				seed_neighbors[index * 4 + 1] = seeds_t[id2 * 3 + 1];
				seed_neighbors[index * 4 + 2] = seeds_t[id2 * 3 + 2];//seed id
				//Do not update the validness of seeds. The invalid seed in this iteration can be used in next iteration.
				//seed_neighbors[index * 4 + 3] = seed_valid[id2];
			}
		}
	}
}

void GPUSupervoxel::update_seeds_neighbors_swap()
{
	for (int spos = 0; spos < seeds_num; ++spos)
	{
		for (int i = 0; i < 27; i++)
		{
			int index = spos * 27 + i;
			int id2 = seed_neighbor_map[index];
			if (id2 > -1)
			{
				seed_neighbors[index * 4] = seeds_t[id2 * 3];
				seed_neighbors[index * 4 + 1] = seeds_t[id2 * 3 + 1];
				seed_neighbors[index * 4 + 2] = seeds_t[id2 * 3 + 2];
				seed_neighbors[index * 4 + 3] = seed_valid[id2];
			}
		}
	}
}

int GPUSupervoxel::JFASupervoxel(const std::vector<double> &points,
	const std::vector<double> &normals, double voxelResolution_, double seedResolution_, double lambda1, double lambda2, string& save_name,int save_type)
{
	clock_t startTime, endTime, startTotal,endTotal;
	startTotal = clock();
	voxelResolution = voxelResolution_;
	seedResolution = seedResolution_;
	point_num = (int)points.size() / 3;

	
	startTime = clock();
	voxelization(points, normals, voxelResolution, seedResolution);
	endTime = clock();
	cout << "voxelization time : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	startTime = clock();
	texture.init_texture(data_3d, normal_3d, voxel_gridId, seed_neighbors, seed_cov_info, width_t, height_t, voxels.size(), min_size);
	endTime = clock();
	cout << "loading data to GPU time : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

    startTime = clock();
	int* render_color = new int[seeds_num * 3];
	for (int i = 0; i < seeds_num * 3; i++)
	{
		render_color[i] = rand() % 256;
	}
	Shader seg = Shader("VP_Seg.vs", "FP_Seg.fs");
	Shader aver_seg = Shader("VP_SumSeg.vs", "FP_SumSeg.fs");
	Shader aver_swap = Shader("VP_SumSwap.vs", "FP_SumSwap.fs");
	Shader cov = Shader("VP_cov.vs", "FP_cov.fs");
	Shader swap_comb = Shader("VP_SwapComb.vs", "FP_SwapComb.fs");
	//average position of supervoxel, compute the nearest voxel as new seed for next iteration
	vector<float> seeds_3d;
	int Lloyd_curr_iter = 0;
	//set the maximum iteration for Lloyd iteration
	const int Lloyd_max_iter = 15;
	//set the maximum iteration for Swapping iteration
	const int Swap_max_iter = 10;
	bool Lloyd_ends = false;
	int* voxel_labels = new int[width_t*height_t];
	
	//update the parameters: D = D(plane) + lambda1*D(normal) lambda2*D(pos)
	texture.update_params(lambda1, lambda2);
	while (!Lloyd_ends)
	{
		Lloyd_curr_iter++;
		cout << "Clustering iter" << Lloyd_curr_iter<<":" << endl;
		texture.seg(seg, Lloyd_curr_iter);

		if (Lloyd_curr_iter == Lloyd_max_iter)
		{
			Lloyd_ends = true;
		}
		else {
			texture.aver_surface_seg(aver_seg, seeds_3d, seed_valid);
			nearest_valid_seed_voxel(seeds_3d, Lloyd_curr_iter);
			update_seeds_neighbors();
			texture.update_seeds(seed_neighbors);
		}
	}

	//texture.label(voxel_labels);
	//generate_voxel_seg(save_file_name,labels, points, normals, render_color, Lloyd_curr_iter);
	endTime = clock();
	cout << "clustering time : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	startTime = clock();
	//You can try to change the parameters before swapping stage
	//texture.update_params(lambda1*2, lambda2*2);
	for (int i = 1; i < Swap_max_iter; i++)
	{
		cout << "Swapping iter" << i<<":" << endl;
		texture.aver_surface_swap(aver_swap, seed_cov_info, seed_valid);
		texture.update_seed_matrix(seed_cov_info);//update aver_pos in seed_matrix texture
		texture.cov_matrix(cov, seed_cov_info, seed_valid);
		texture.update_seed_matrix(seed_cov_info);
		update_seeds_neighbors_swap();
		texture.update_seeds(seed_neighbors);
		texture.swapping(swap_comb, i);
	}

	texture.label(voxel_labels);
	endTime = clock();
	cout << "swapping time : " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	endTotal = clock();
	cout << "total time of algorithm : " << (double)(endTotal - startTotal) / CLOCKS_PER_SEC << "s" << endl;
	
	if (save_type == 0) {
		//generate the label of the points, then evaluate supervoxel performance
		generate_label(save_name, voxel_labels);
		cout<<"save label in: "<<save_name<<endl;
	}
	else if(save_type==1)
	{
		//generate the position and color of the points, show it in CloudCompare software
		generate_seg(save_name, voxel_labels, points, normals, render_color);
		cout << "save visualization in: " << save_name << endl;
	}
	

	if (render_color) delete[] render_color;
	if (voxel_labels) delete[] voxel_labels;
	return 0;
}

void GPUSupervoxel::generate_label(string& save_file_name, int* labels)
{
	int voxelNumber = (int)voxels.size();
	int slice_num = width * height;
	std::vector<double> p_label;
	p_label.resize(point_num, -1);

	for (int pos = 0; pos < voxelNumber; ++pos)
	{
		int label = labels[pos];
		int memberNumber = (int)voxels[pos].members.size();
		int mem = 0;
		for (int i = 0; i < memberNumber; i++)
		{
			mem = voxels[pos].members[i];
			p_label[mem] = label;
		}
	}

	ofstream f_debug(save_file_name);
	for (int i = 0; i < point_num; i++)
	{
		f_debug << p_label[i] << endl;
	}
	f_debug.close();

}

void GPUSupervoxel::generate_seg(string& save_file_name, int* labels, const std::vector<double> &points, const std::vector<double> &normals, int* render_color)
{
	std::vector<int> p_color;
	p_color.resize(point_num * 3, -1);
	int voxelNumber = (int)voxels.size();
	int slice_num = width * height;

	for (int pos = 0; pos < voxelNumber; ++pos)
	{
		int label = labels[pos];
		int memberNumber = (int)voxels[pos].members.size();
		int r_ = render_color[label * 3], g_ = render_color[label * 3 + 1], b_ = render_color[label * 3 + 2];
		for (int i = 0; i < memberNumber; i++)
		{
			int mem = voxels[pos].members[i];
			p_color[mem * 3] = r_;
			p_color[mem * 3 + 1] = g_;
			p_color[mem * 3 + 2] = b_;
		}
	}

	ofstream f_debug(save_file_name);
	for (int i = 0; i < point_num; i++)
	{
		f_debug << setiosflags(ios::fixed) << setprecision(6) << points[i * 3] << "," << points[i * 3 + 1] << "," << points[i * 3 + 2] << "," << normals[i * 3] << "," << normals[i * 3 + 1] << "," << normals[i * 3 + 2] << ",";
		f_debug << p_color[i * 3] << "," << p_color[i * 3 + 1] << "," << p_color[i * 3 + 2] << endl;
	}
	f_debug.close();


}


//void GPUSupervoxel::print_data(vector<float>& data_3d, string& txt_name, int bit)
//{
//	int slice_num = width * height;
//	int texel_num = slice_num * depth;
//
//
//	ofstream f_debug(txt_name);
//	int slice_index = 0;
//	for (int t = 0; t < depth; t++)//depth
//	{
//		for (int i = 0; i < height; i++) {
//			//slice_index = (t * slice_num + i * width) * 4;
//			for (int j = 0; j < width; j++) {
//
//				f_debug << std::left << std::setw(4) << data_3d[slice_index++] << " ";
//				f_debug << std::left << std::setw(4) << data_3d[slice_index++] << " ";
//				f_debug << std::left << std::setw(4) << data_3d[slice_index++] << " ";
//				if (bit == 4)
//				{
//					f_debug << std::left << std::setw(4) << data_3d[slice_index++] << " ";
//				}
//			}
//			f_debug << endl;
//		}
//		f_debug << endl;
//		f_debug << endl;
//	}
//
//	f_debug.close();
//}
