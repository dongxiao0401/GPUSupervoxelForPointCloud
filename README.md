We provide the source code that tested on Windows for the paper: 
> **GPU-based Supervoxel Segmentation for 3D Point Clouds**


## Runtime Environment 
```
The project requires: OpenGL 4.2 or later; 

The glad and GLFW3 are provided in the "dependency" dir. Please set the include and lib paths in your project.

```

## Parameters 
```
Specify the paths of position and normal of the point cloud. 

The program generates supervoxel segmentation results in txt file, including labels and pseudo-color images. 
You can use CloudCompare software to see the pseudo-color result. 

We provide an example point cloud from Okaland dataset. Note that we pre-compute the normals of the points. 

You can download the point cloud datasets from following websites:
Okaland: http://www.cs.cmu.edu/~vmr/datasets/oakland_3d/cvpr09/doc/
Semantic3D: http://www.semantic3d.net/
NYUV2: https://cs.nyu.edu/~silberman/datasets/nyu_depth_v2.html

```

