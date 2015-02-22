Matlab Offscreen Rendering Toolbox (version 2)

[Revision]
8/21/2012  Version 2, fix a few glut crashing bugs (mostly in Ubuntu). Add render to depth image function.
4/3/2009   Version 1.

[Introduction]

This toolbox provides support to use OpenGL's rendering capability to project 3D triangular meshes onto an image plane. This has a few applications including extract the color of each face on the mesh given an observed image, determine the shadow area on the mesh given a light source direction and visualizing a colored mesh from a camera viewpoint. Version 2 also added the support of rendering a depth image.

Directly using OpenGL calls has the advantage of using more advanced graphics hardware capabilities, which results in faster rendering, and better control of anti-alliasing and other GL options.

[Features]

1. Use OpenGL for offscreen rendering, make use of your graphics hardware acceleration without showing anything on the screen.
2. Directly input/output using matlab image arrays, no need to deal with matlab figures.
3. Render any colored mesh into an image, calculate visibility of each triangle.
4. Render from light source to calculate shadowed regions,
4. (New) Render any mesh into depth image (for the kinect lovers). 

[Installation]

1. You will need GLUT and GLEW library to compile and run this toolbox. Mathworks now requires all the binaries being removed from the uploaded package, so I can no longer include any of these libraries. To install these library on ubuntu, you can try "sudo apt-get install freeglut3-dev libglew-dev". 
2. Compile the mex source code
	After install GLUT and GLEW, you could run the compilation script "CompileOffscreen.m" to compile all the mex source code. Be sure to change the path at the beginning of the script to point to the correct location.
3. Run the test script "OffscrenTest.m" to test if everything works fine.

[Contents]

1. Main functions
BatchFaceColorGrad.m
	Matlab function to project a set of triangles to a set of images and extract the triangle color
BatchFaceColorImpl.cpp
	The underlying c++ implementation of the BatchFaceColor function. It also provide additional functions to compute the gradient inside each triangle.

ProjectMesh2Image.m
	Matlab function to project a triangular mesh onto a set of images and get a labeled image.
ProjectMesh2ImageImpl.cpp
	The underlying c++ implementation of the ProjectMesh2Image function.

RenderColorMesh.m
	Matlab function to render a colored triangular mesh to an image
RenderColorMeshImpl.cpp
	The underlying c++ implementation of the RenderColorMesh function.

ShadowProj2.m
	Matlab function to perform a orthographic project for a mesh to calculate which triangle is in shadow
ShadowProj2Impl.cpp
	The underlying c++ implementation for ShadowProj2 function.

-- New in version 2, thanks to the help of Jose Padial (jpadial@stanford.edu)
RenderDepthMesh.m
        Matlab function to render a depth image from a triangular mesh
RenderDepthMeshImpl.cpp
        The underlying c++ implementation for RenderDepthMesh function.

2. Supporting files

OffscreenGL.h
	The opengl wrapper class to set up an openGL context for graphics rendering in Matlab.

MexGlutInit.cpp
        The matlab mex function to initialize the glut only once (to fix the crashing of freeGLUT on Ubuntu).

CamS2Vector1.m
	Converting the camera parameter structure CamParamS to the OpenGL camera parameters (ViewPoint vector, LookAt vector, Up vector, the focal length vector and the principle point vecor)

Vector2CamS1.m
	Converting the OpenGL camera parameters back to the CamParamS structure used in this package.

CamSInerp1.m
	A function to interpolate between two CamParamS to create a smooth camera transition.

CompileOffscreen.m
	A matlab script to compile all the c++ mex functions

[Notations]
	The variable names in this toolbox is suffixed with upper case letters indicating its type/shape:
	'M'  -- 2D matrix
	'T'  -- 3D Tensor
	'V'  -- 1D vector (usually column vector)
	'RV' -- row vector
	'A'  -- cell array
	'S'  -- structure
	'SA' -- structure array

[Examples]

1. The CamParamS structure
	The CamParamS is a structure to pack all the camera parameters of a standard pin-hole camera. It includes four fields:
	CamParamS.TcV -- The translation vector (3x1) from the world coordinates to the camera coordinates
	CamParamS.RcM -- The rotation matrix (3x3) from the world coordinates to the camera coordinates
	CamParamS.fcV -- The focal length vector (2x1)
	CamParamS.ccV -- The principle point vector (2x1)
	
	Assuming the world coordinates of a 3D point is Xw (3x1 vector), it's camera coordinates is Xc (3x1 vector), it's position on the final image is Ip = (Ix, Iy), then Ip can be compute by:

	Xc = RcM * Xw + TcV;
	Ix = Xc(1)/Xc(3) * fcV(1) + ccV(1);
	Iy = Xc(2)/Xc(3) * fcV(2) + ccV(2);

	Additionaly, zNearFarV = [zNear; zFar] is used to set the zNear and zFar parameter of openGL, and zoomFactor is used to provide basic software antialiasing (zoomFactor = 2 gives the best compromise of quality and speed)
	
2. The mesh representation
	According to Matlab's convention, the triangular mesh is represented as a list of facet (FM, N x 3 matrix) and a list of vertices (VM, M x 3 matrix), where N is the number of triangular facets and M is the number of vertices. Each row of VM is the 3D world coordinates of a vertex. Each Row of FM contains the indices of the 3 vertices of that facet.
	The order of the three vertices is important as it is used to calculate the normal of the facet and therefore used in shadow and visibility calculations. If a facet is (V1, V2, V3), then the normal is calculate by (V2 - V1) x (V3 - V2). If the normal is pointing opposite to the viewpoint, it is considered not visible even if no object occludes it.

3. Input images:
	The input images are assumed o be RGB image of UINT8, if the image size is h x w, then the dimension of the input images is h x w x 3. All the input images are stored in a cell array (because they could be of different sizes). The cell array indices corresponds to those in the CamParamSA cell array and the image mask cell array.

4. Image masks:
	Each input image could have one corresponding segmentation mask to filter out background. The image mask is of type UINT8, has the same size of the image (h x w) and only one color component. Any nonzero value in the mask means the pixel is foreground, otherwise it is in the background. If no segmentation mask is available, a mask image of all 255 could be used.

4. Output
	The output of BatchFaceColorGrad is a set of Facet Color Tensor and as well as the visibility matrix. Each row corresponds to a facet, each column correspond to one camera position in CamParamSA and the third dimension corresponds to color bands if the input is color image.

	The output of ProjectMesh2Image is a set of labeled images stored in a cell array. A labeled image is a image whose pixels are assigned labels. The label of a pixel indicates which triangle this pixel belongs to when the mesh is projected onto the image. 

	The output of ShadowProj2 is a array of UINT32 bit field, where each bit correspond to whether a facet is in shadow of the light in LightDirM

	The output of RenderColorMesh is an image of size h x w x 3, where [h; w] = ImageSizeV. It renders the mesh using the specified facet color (ColorM) and camera parameters (CamParaS)

        The output of RenderDepthMesh is an image of size h x w, where [h; w] = ImageSizeV. It renders the mesh using the camera parameters and generate a depth image.

[To Do]
1. Need to add some debug features

[Acknowledgement]
This toolbox is developed during my Ph.D. study in University of Illinois at Urbana-Champaign. Fundings from National Science Foundation under grant ECS 02-25523 and Beckman Institute Graduate Fellowship provide partial support for the research.

[Contact]

If you have any questions about this toolbox, please contact:

Tianli Yu (tianli@ieee.org), http://tianliresearch.blogspot.com
Researcher & Software Engineer at Google


