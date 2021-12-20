#pragma once

#ifndef UTILITY_H
#define UTILITY_H

#include <fstream>
#include <string>



static bool read_points_normals(std::string& posName, std::string& normalName, std::vector<double> &points, std::vector<double> &normals)
{
	points.clear();
	std::ifstream fileStream_pos((char*)posName.c_str());
	if (!fileStream_pos)
	{
		cout << "load points failed!" << endl;
		return false;
	}
		
	std::ifstream fileStream_normal((char*)normalName.c_str());
	if (!fileStream_normal)
	{
		cout << "load normal failed!" << endl;
		return false;
	}
		
	while (!fileStream_pos.eof())
	{
		double x, y, z;
		//char note;

		fileStream_pos >> x >> y >> z;


		points.push_back(x);
		points.push_back(y);
		points.push_back(z);
	}
	fileStream_pos.close();

	while (!fileStream_normal.eof())
	{
		double x, y, z;
		//char note;

		fileStream_normal >> x >> y >> z;


		normals.push_back(x);
		normals.push_back(y);
		normals.push_back(z);
	}
	fileStream_normal.close();

	if (points.size() != normals.size())
	{
		cout << "size doesn't match" << endl;
		return false;
	}
	return true;
}


static void write_labels(const std::vector<int> &labels, std::string& fileName)
{
	std::ofstream labelresult(fileName);

	if (labelresult)
	{
		int vnb = (int)labels.size();
		for (int i = 0; i < vnb; ++i)
		{
			labelresult << labels[i];
			if (i < vnb - 1)
				labelresult << std::endl;
		}
	}

	labelresult.close();
}


#endif