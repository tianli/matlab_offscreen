/*******************************************************************************
 ProjectMesh2ImageImpl.cpp: implementation of the offscreen projection of a mesh
 onto an image.

 example:
  
 LabeledImageA = ProjectMesh2ImageImpl(FaceM, VertexM, CamParamSA, channelModFactor, ... 
					ScreenSizeV, zoomFactor, ZNearFarV)

*******************************************************************************/



#include <mex.h>
#include <math.h>

#include "OffscreenGL.h"
#include "OffscreenCommon.h"

void getLabeledImages(
	// input
	double *FM, int fNum, double *VM, int vNum, const mxArray *CamParamSA, double *ScreenSizeV, int zoomFactor,
	unsigned int channelModFactor, double *ZNearFarV,
	// output
	mxArray *LabeledImageA
	)
{
	int imageNum, i;
	GLuint batchList;
	mxArray *currentCamParamS;
	GLubyte *imageBuffer;
	int bufferDims[3];
	
	unsigned int *LabeledImage;
	
	imageBuffer = (GLubyte *) mxMalloc(ScreenSizeV[0] * ScreenSizeV[1] * COLOR_MODULATE_CHANNEL * zoomFactor * zoomFactor);
	
	imageNum = mxGetM(CamParamSA);
	
	batchList = createDisplayList(FM, fNum, VM, vNum, channelModFactor);
	printf("Facet Number: %d, Vertex Number: %d\nProjecting images: \n", fNum, vNum);
	
	for (i = 0; i < imageNum; i++) {
		
		printf(" %d ", i);
		if (i % 8 == 7) {
			printf("\n");
		}
		
		currentCamParamS = mxGetCell(CamParamSA, i);
	
		double *imSizeV = mxGetPr(mxGetField(currentCamParamS, 0, "imSizeV"));
		
		cameraSetup(currentCamParamS, ZNearFarV[0], ZNearFarV[1], imSizeV[0], imSizeV[1], zoomFactor);
		drawPatch(batchList, imageBuffer, imSizeV[0], imSizeV[1], zoomFactor);		
		
		bufferDims[0] = imSizeV[0] * zoomFactor;
		bufferDims[1] = imSizeV[1] * zoomFactor;		
		mxSetCell(LabeledImageA, i, mxCreateNumericArray(2, bufferDims, mxUINT32_CLASS, mxREAL));
		LabeledImage = (unsigned int *) mxGetData(mxGetCell(LabeledImageA, i));
		
		colorDemodulation(imageBuffer, channelModFactor, NULL, imSizeV[0], imSizeV[1], zoomFactor, LabeledImage);
	
	}
	mxFree(imageBuffer);
	// Relase the display list.
	if (list) {
		glDeleteLists(list, 1);
		list = 0;
	}
	printf("Done\n");
}


/* The gateway routine */

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
                 
{

	int output1Size[1];
	
	double *FM = mxGetPr(prhs[0]);
	int fNum = mxGetM(prhs[0]);	
	double *VM = mxGetPr(prhs[1]);
	int vNum = mxGetM(prhs[1]);
	const mxArray *CamParamSA = prhs[2];
	unsigned int channelModFactor = (unsigned int)mxGetScalar(prhs[3]);	
	double *ScreenSizeV = mxGetPr(prhs[4]);
	double zoomFactor = mxGetScalar(prhs[5]);
	double *zNearFarV = mxGetPr(prhs[6]);
	
	const int *dims;
	mxArray *LabeledImageA;
	
	OffscreenGL offscreenGL((int)(ScreenSizeV[0] * zoomFactor), (int)(ScreenSizeV[1] * zoomFactor));
			
	if (nlhs == 1) {
		output1Size[0] = mxGetM(CamParamSA);
		plhs[0] = mxCreateCellArray(1, output1Size);		
		LabeledImageA = plhs[0];

		if (offscreenGL.RGB8Setup()) {	
			mexPrintf("OpenGLCanvas setup Successful\n");
						
			getLabeledImages(FM, fNum, VM, vNum, CamParamSA, ScreenSizeV, zoomFactor, 
				channelModFactor, zNearFarV, LabeledImageA);
		
		} else {
			mexPrintf("OpenGLCanvas setup failed\n");	
		}
		
	}
}
