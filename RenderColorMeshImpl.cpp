#include "mex.h"
#include "math.h"
#include "OffscreenGL.h"
#include "OffscreenCommon.h"

void drawPatchAndConvert(GLuint listName, GLubyte *imageBuffer, unsigned int imgHeight, unsigned int imgWidth, unsigned int zoomFactor = 1)
{
	// This is a temporary bug fix for Nvidia's open program
	// seems the width of the pixel has to be a multiple of 4
	// for other width, we have to pad the width and remove it later
	unsigned int paddedWidth = imgWidth * zoomFactor % 4;
	if (paddedWidth != 0) {
		paddedWidth = 4 - paddedWidth + imgWidth * zoomFactor;
	} else {
		paddedWidth = imgWidth * zoomFactor;
	}
	
	unsigned char *paddedImgBuffer = (unsigned char *)mxMalloc(paddedWidth * imgHeight * zoomFactor * MAX_COLOR_CHANNEL * sizeof(GL_UNSIGNED_BYTE));
	drawPatch(listName, paddedImgBuffer, imgHeight, imgWidth, zoomFactor);	

	// reorder the pixel data for the opengl to matlab conversion
	unsigned int imgSize = imgHeight * imgWidth * zoomFactor * zoomFactor;
	unsigned int imgSize2 = imgSize * 2;
	unsigned int matlabImgIndex = 0;
	unsigned int oglImageIndex = 0;
	
	for (int j = 0; j < imgWidth * zoomFactor; j++) {
		for (int i = 0; i < imgHeight * zoomFactor; i++, matlabImgIndex++) {
			oglImageIndex = (j + (imgHeight*zoomFactor -1-i) * paddedWidth) * 3;
		    imageBuffer[matlabImgIndex] = paddedImgBuffer[oglImageIndex];
		    imageBuffer[matlabImgIndex + imgSize] = paddedImgBuffer[oglImageIndex + 1];
		    imageBuffer[matlabImgIndex + imgSize2] = paddedImgBuffer[oglImageIndex + 2];
		}
	}
	
	mxFree(paddedImgBuffer);
}

static void renderColorMesh(double *FM, int fNum, double *VM, int vNum, float *ColorM, int colorNum,
					const mxArray *CamParamS, double *imgSizeV, double *zNearFarV, unsigned int zoomFactor,
					// output
					unsigned char *imgBuffer)
{
	cameraSetup(CamParamS, zNearFarV[0], zNearFarV[1], (unsigned int) imgSizeV[0], (unsigned int) imgSizeV[1], zoomFactor);

#ifndef NDEBUG
	mexPrintf("Start to create the display list: fNum=%d, vNum=%d, colorNum=%d\n", fNum, vNum, colorNum);
#endif

	GLuint list = createDisplayListWithColor(FM, fNum, VM, vNum, ColorM, colorNum);

#ifndef NDEBUG
	mexPrintf("Start to draw the patch\n");
#endif

	drawPatchAndConvert(list, imgBuffer, (int) imgSizeV[0], (int) imgSizeV[1], zoomFactor);
}

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
	// get the vertex array, face array, and color array
	double *FM = mxGetPr(prhs[0]);
	int fNum = mxGetM(prhs[0]);	
	double *VM = mxGetPr(prhs[1]);
	int vNum = mxGetM(prhs[1]);
	float *ColorM = (float *)mxGetData(prhs[2]);
	int colorNum = mxGetM(prhs[2]);
	
	// get the camera parameters
	const mxArray *CamParamS = prhs[3];
	double *imgSizeV = mxGetPr(prhs[4]);
	double *zNearFarV = mxGetPr(prhs[5]);
	double zoomFactor = mxGetScalar(prhs[6]);
	
	OffscreenGL offscreenGL((int)(imgSizeV[0] * zoomFactor), (int) (imgSizeV[1] * zoomFactor));
	int output3Size[3];
	
	unsigned char *imgBuffer;
	
	if (offscreenGL.RGB8Setup()) {
		mexPrintf("OpenGLCanvas setup Successful\n");
		output3Size[0] = (int) (imgSizeV[0] * zoomFactor);
		output3Size[1] = (int) (imgSizeV[1] * zoomFactor);
		output3Size[2] = 3;
		
		plhs[0] = mxCreateNumericArray(3, output3Size, mxUINT8_CLASS, mxREAL);
	    imgBuffer = (unsigned char *) mxGetData(plhs[0]);
		renderColorMesh(FM, fNum, VM, vNum, ColorM, colorNum, CamParamS, imgSizeV,
						zNearFarV, (unsigned int) zoomFactor, imgBuffer);
		
	} else {
		mexPrintf("OpenGLCanvas setup failed\n");		
	}
}

