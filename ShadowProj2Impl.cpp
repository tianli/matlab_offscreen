/**
 * New Wish for _ShadowProj.cpp:  Monday Jul. 25 2005  11:24 pm
 *  output unsigned char, unsigned short or unsigned int
 *  use matlab for the final adapation/choice.
 *  use the generic OpenGLCanvas for offscreen rendering
 *
 * Wish 2:	use byte image input and float gradient input to reduce the memory footprint, output double.
 *			This revision is successful, the memory consumption for 6 1024*768 3 band color image reduced from 260MB to 130MB
 *			
 * Switch to ShadowProjX.cpp
 *  
 * Wish 1:	output the area count for each face on the mesh given a set of light source direction
 * 			use orthographic projection 
 *
 * Wish 2:	Add three features:
 * 			1. fractional shadow (0 - 1) instead of the pixel count
 *			2. object maximum axis used as up vector.
 *			3. occlusion verification (use surface orientation)
 * Wish 3:	remove fractional shadow, since this consumes memory as well as the CPU time
 *
 * ShadowProjImpl:
 * Wish 2:  use the uint32 bit output to reduce the memory foot print
 */

#include <mex.h>
#include <math.h>
#include "OffscreenGL.h"


/**
 *	Create the display list to draw the patch multiple times, use color to modulate the triangles
 */ 
GLuint createDisplayList(const double *fM, int fNum, const double *vM, int vNum, unsigned int colorModFactor)
{
	GLuint theShape;
	int i;
	unsigned int channelCapacity, channelCapacity2;
	//double *fp;
	int vIndex, fNum2;
	
	fNum2 = fNum*2;
	
	channelCapacity = 256 / colorModFactor;
	channelCapacity2 = channelCapacity * channelCapacity;
	theShape = glGenLists (1);
	glNewList(theShape, GL_COMPILE);
	
	glBegin (GL_TRIANGLES);
	for (i = 1; i <= fNum; i++) {
		const double *fp = fM + i-1;
		
		glColor3ub(i / channelCapacity2 * colorModFactor, i / channelCapacity % channelCapacity * colorModFactor, i % channelCapacity * colorModFactor);
		vIndex = (int)fp[0] - 1;

		// debug
		//printf("ok\n");		
		//printf("%d %f %f %f\n", vIndex, fp[0], fp[fNum], fp[fNum2]);
		
		glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + 2*vNum] );
		vIndex = (int)fp[fNum] - 1;
		glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + 2*vNum] );
		vIndex = (int)fp[fNum2] - 1;
		glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + 2*vNum] );
		
		// debug
		// printf("%f %f %f %d\n", vM[vIndex], vM[vIndex + vNum], vM[vIndex + 2*vNum], i);
		//printf("color: %d %d %d %d %d\n", colorModFactor, i / channelCapacity2 * colorModFactor, i / channelCapacity % channelCapacity * colorModFactor, i % channelCapacity * colorModFactor, i);
	}
		
	glEnd ();	
	glEndList();
	return theShape;
}

/**
 *	Orthographic projection
 *	Assume that lightDirV is normalized 
 */ 
#define SAFE_DIST_FACTOR 1.2
void orthoCameraSetup(const double *objCenterV, const double *lightDirV, double maxRadius, 
	const double *majorAxisV, unsigned int imgHeight, unsigned int imgWidth, float &screenScale)
{


	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);	
	
	glMatrixMode(GL_MODELVIEW);   
	glLoadIdentity();

	double dotP = lightDirV[0] * majorAxisV[0] + lightDirV[1] * majorAxisV[1] + lightDirV[2] * majorAxisV[2];
	
	if (fabs(dotP) < 0.9) {
		// using the major axis as up vector
		gluLookAt(objCenterV[0] + lightDirV[0] * maxRadius * SAFE_DIST_FACTOR, 
					objCenterV[1] + lightDirV[1] * maxRadius * SAFE_DIST_FACTOR, 
					objCenterV[2] + lightDirV[2] * maxRadius * SAFE_DIST_FACTOR
					, objCenterV[0], objCenterV[1], objCenterV[2], majorAxisV[0], majorAxisV[1], majorAxisV[2]);	
	} else {
		// using direction perpendicular to light vector
		gluLookAt(objCenterV[0] + lightDirV[0] * maxRadius * SAFE_DIST_FACTOR, 
					objCenterV[1] + lightDirV[1] * maxRadius * SAFE_DIST_FACTOR, 
					objCenterV[2] + lightDirV[2] * maxRadius * SAFE_DIST_FACTOR
					, objCenterV[0], objCenterV[1], objCenterV[2], majorAxisV[3], majorAxisV[4], majorAxisV[5]);	
	}    

	double left, right, bottom, top, zNear, zFar;
	
	left = - maxRadius * imgWidth / imgHeight;
	bottom = - maxRadius;
	screenScale = imgHeight / 2 / maxRadius;
	right = -left;
	top = -bottom;
	zNear = 0;
	zFar = 2 * maxRadius * SAFE_DIST_FACTOR;

    glMatrixMode(GL_PROJECTION);  
    glLoadIdentity();
    
	glOrtho(left, right, bottom, top, zNear, zFar);
    
    // set the view port 
    glViewport(0, 0, imgWidth, imgHeight);        

}

void drawPatch(GLuint listName, GLubyte *imageBuffer, unsigned int imgHeight, unsigned int imgWidth)
{
	glCallList (listName);
	glFlush ();

	glReadPixels(0, 0, imgWidth, imgHeight, GL_RGB, GL_UNSIGNED_BYTE, imageBuffer);	
}

void extractFaceCount(GLubyte *imageBuffer, unsigned int channelModFactor, 
	unsigned int imgHeight, unsigned int imgWidth, unsigned int fNum, int bitOffset,
	// output
	void *FacePixelCountV)
{
	unsigned int ii, jj, channel1Factor, channel2Factor;
	unsigned int faceIndex, imageIndex, colorImageIndex;
	
	int imgSize;
	
	imgSize = imgHeight * imgWidth;
	
	channel1Factor = 256 * 256 / channelModFactor / channelModFactor / channelModFactor;
	channel2Factor = 256 / channelModFactor / channelModFactor;
	
	unsigned int bitMask = 1 << bitOffset;
	
	for (jj = 0, imageIndex = 0; jj < imgWidth; jj++) {
		for (ii = 0; ii < imgHeight; ii++, imageIndex++) {

			colorImageIndex = (jj + (imgHeight-1-ii) * imgWidth) * 3;
			faceIndex = imageBuffer[colorImageIndex] * channel1Factor + imageBuffer[colorImageIndex + 1] * channel2Factor
											+ imageBuffer[colorImageIndex + 2] / channelModFactor;

			if (faceIndex > 0) {
				((unsigned int *)FacePixelCountV)[faceIndex-1] |= bitMask;
			}
		}
	}
}


void getBatchFaceCount(
	// input
	const double *FM, int fNum, const double *VM, int vNum, const double *objCenterV, double maxRadius,
	const double *majorAxisM, const double *lightDirM, int lightNum, int maxHeight, int maxWidth, 
	unsigned int channelModFactor,
	// output
	void *FacePixelCountM)
{
	int i;
	
	GLubyte *imageBuffer = (GLubyte *) mxMalloc(maxHeight * maxWidth * 3);
	GLuint batchList = createDisplayList(FM, fNum, VM, vNum, channelModFactor);

	
	//memset(FacePixelCountM, 0, sizeof(double) * fNum * lightNum);
	
	printf("Facet Number: %d, Vertex Number: %d\nProjecting images: \n", fNum, vNum);
	
	float screenScale;
	
	// make sure that maxHeight always >= maxWidth
	if (maxHeight < maxWidth) {
		int temp = maxHeight;
		maxHeight = maxWidth;
		maxWidth = temp;	
	}
	
	for (i = 0; i < lightNum; i++) {
		
		printf(" %d ", i);
		if (i % 16 == 15) {
			printf("\n");
		}		
		orthoCameraSetup(objCenterV, lightDirM + 3*i, maxRadius, majorAxisM, maxHeight, maxWidth, screenScale);

		drawPatch(batchList, imageBuffer, maxHeight, maxWidth);		
		
		extractFaceCount(imageBuffer, channelModFactor, maxHeight, maxWidth, fNum, i%(32),
			(unsigned int*) FacePixelCountM + i/32 * fNum);
	}
	printf("\n");
	mxFree(imageBuffer);	
		
}


/* The gateway routine */
/**
 *	FacePixelCountM = ShadowProj2(FM, VM, LightDirM, ObjCenterV, maxRadius, majorAxisM, ScreenSizeV, channelModFactor)
 *
 */
 
void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
                 
{
	int output2Size[2];
	
	const double *FM = mxGetPr(prhs[0]);
	int fNum = mxGetM(prhs[0]);	
	const double *VM = mxGetPr(prhs[1]);
	int vNum = mxGetM(prhs[1]);
	
	const double *lightDirM = mxGetPr(prhs[2]);
	int lightNum = mxGetN(prhs[2]);
	
	const double *objCenterV = mxGetPr(prhs[3]);
	double maxRadius = mxGetScalar(prhs[4]);
	
	const double *majorAxisM = mxGetPr(prhs[5]);
	
	const double *maxSizeV = mxGetPr(prhs[6]);
	unsigned int channelModFactor = (unsigned int)mxGetScalar(prhs[7]);

	// remove output type, we will only output uint32 type to simplify things.
	//int outputType = (int) mxGetScalar(prhs[8]);
	
	void *FacePixelCountM;

	output2Size[0] = fNum;
	output2Size[1] = lightNum;

	/*
	if (outputType == sizeof(unsigned char)) {
	    output2Size[1] = (output2Size[1]-1)/(8*sizeof(unsigned char)) + 1;
		plhs[0] = mxCreateNumericArray(2, output2Size, mxUINT8_CLASS, mxREAL);
	} else if (outputType == sizeof(unsigned short)) {
		output2Size[1] = (output2Size[1]-1)/(8*sizeof(unsigned short)) + 1;
		plhs[0] = mxCreateNumericArray(2, output2Size, mxUINT16_CLASS, mxREAL);
	} else if (outputType == sizeof(unsigned int)) {
		output2Size[1] = (output2Size[1]-1)/(8*sizeof(unsigned int)) + 1;
		plhs[0] = mxCreateNumericArray(2, output2Size, mxUINT32_CLASS, mxREAL);
	}*/
	
	// assume the created data will be all zero
	output2Size[1] = (output2Size[1]-1)/32 + 1;
	plhs[0] = mxCreateNumericArray(2, output2Size, mxUINT32_CLASS, mxREAL);


	FacePixelCountM = mxGetData(plhs[0]);
	
	OffscreenGL offscreenGL((int)maxSizeV[0], (int)maxSizeV[1]);

	if (offscreenGL.RGB8Setup()) {
		mexPrintf("OpenGLCanvas setup Successful\n");
						
		/*
		getBatchFaceCount(FM, fNum, VM, vNum, objCenterV, maxRadius, majorAxisM, lightDirM, 
			lightNum, (int)maxSizeV[0], (int)maxSizeV[1], channelModFactor, outputType,
			FacePixelCountM);
		*/
		getBatchFaceCount(FM, fNum, VM, vNum, objCenterV, maxRadius, majorAxisM, lightDirM,
			lightNum, (int)maxSizeV[0], (int)maxSizeV[1], channelModFactor,
			FacePixelCountM);

	} else {
		mexPrintf("OpenGLCanvas setup failed\n");
	}
}
