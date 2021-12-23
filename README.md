We provide the source code that tested on Windows for the paper: 
> **GPU-based Supervoxel Segmentation for 3D Point Clouds**


## Runtime Environment 
```
The project requires: OpenGL 4.2 or later; 

We provide GLFW in "includes" and "lib". Create "build" dir and use cmake to build the project. 

```

## Parameters 
```
Specify the paths of position and normal of the point cloud. Note that the paths of shaders should be correct.

We provide an example point cloud from Okaland dataset. Note that we pre-compute the normals of the points. 

The program generates supervoxel segmentation results in txt file, including labels and pseudo-color result. 
You can specify the "save_type" of the txt result. Please use CloudCompare software to see the pseudo-color result. 


You can download the point cloud datasets from following websites:
Okaland: http://www.cs.cmu.edu/~vmr/datasets/oakland_3d/cvpr09/doc/
Semantic3D: http://www.semantic3d.net/
NYUV2: https://cs.nyu.edu/~silberman/datasets/nyu_depth_v2.html

```

