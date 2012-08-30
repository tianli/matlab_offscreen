#ifndef OFFSCREEN_COMMON_H
#define OFFSCREEN_COMMON_H

#define COLOR_MODULATE_CHANNEL 3
#define MAX_COLOR_CHANNEL 3

/**
 *	Create the display list to draw the patch multiple times, use color to modulate the triangles
 */ 
GLuint createDisplayList(double *fM, int fNum, double *vM, int vNum, unsigned int colorModFactor)
{
	GLuint theShape;
	int i;
	unsigned int channelCapacity, channelCapacity2;
	double *fp;
	int vIndex, fNum2;
	
	fNum2 = fNum*2;
	
	channelCapacity = 256 / colorModFactor;
	channelCapacity2 = channelCapacity * channelCapacity;
	
	// display list code starts
	theShape = glGenLists (1);
	glNewList(theShape, GL_COMPILE);
	
	glBegin (GL_TRIANGLES);
	for (i = 1; i <= fNum; i++) {
		fp = fM + i-1;
		
		glColor3ub(i / channelCapacity2 * colorModFactor, i / channelCapacity % channelCapacity * colorModFactor, i % channelCapacity * colorModFactor);
		vIndex = (int)fp[0] - 1;
		glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + 2*vNum] );
		vIndex = (int)fp[fNum] - 1;
		glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + 2*vNum] );
		vIndex = (int)fp[fNum2] - 1;
		glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + 2*vNum] );
		
		// debug
		//printf("%f %f %f %d\n", vM[vIndex], vM[vIndex + vNum], vM[vIndex + 2*vNum], i);
		//printf("color: %d %d %d %d %d\n", colorModFactor, i / channelCapacity2 * colorModFactor, i / channelCapacity % channelCapacity * colorModFactor, i % channelCapacity * colorModFactor, i);
	}
	
	glEnd ();	
	glEndList();
	return theShape;
}


/**
 *	Create the display list to draw the patch multiple times, use color to modulate the triangles
 */
static GLuint createDisplayListWithColor(double *fM, int fNum, double *vM, int vNum, GLfloat *ColorM, int colorNum)
{
	GLuint theShape;
	int i;
	double *fp;
	int vIndex, fNum2, vNum2;
	
	fNum2 = fNum * 2;
	vNum2 = vNum * 2;
	
	theShape = glGenLists (1);
	glNewList(theShape, GL_COMPILE);
	glBegin (GL_TRIANGLES);
#ifndef NDEBUG
	mexPrintf("Drawing triangles: fNum=%d, vNum=%d, colorNum=%d\n", fNum, vNum, colorNum);
#endif
	if (colorNum == vNum) { // vertex color
		for (i = 1; i <= fNum; i++) {
			fp = fM + i-1;
			vIndex = (int)fp[0] - 1;
			glColor3f(ColorM[vIndex], ColorM[vIndex + vNum], ColorM[vIndex + vNum2]);
			glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + vNum2] );
			
			vIndex = (int)fp[fNum] - 1;
			glColor3f(ColorM[vIndex], ColorM[vIndex + vNum], ColorM[vIndex + vNum2]);
			glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + vNum2] );
			
			vIndex = (int)fp[fNum2] - 1;
			glColor3f(ColorM[vIndex], ColorM[vIndex + vNum], ColorM[vIndex + vNum2]);
			glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + vNum2] );
			
		}
	} else { // face color
		for (i = 1; i <= fNum; i++) {
#ifndef NDEBUG
  			mexPrintf("Drawing face %d, ColorM[0]=%f\n", i, ColorM[i-1]);
#endif
			fp = fM + i-1;
			glColor3f(ColorM[i-1], ColorM[i-1 + colorNum], ColorM[i-1 + colorNum * 2]);
			
			vIndex = (int)(fp[0] - 1);
			glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + vNum2] );
			vIndex = (int)(fp[fNum] - 1);
			glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + vNum2] );
			vIndex = (int)(fp[fNum2] - 1);
			glVertex3d(vM[vIndex], vM[vIndex + vNum], vM[vIndex + vNum2] );
		}
	}
	
	glEnd ();
	glEndList();
	return theShape;
}

void cameraSetup(const mxArray *camStruct, double zNear, double zFar, unsigned int imgHeight, unsigned int imgWidth, unsigned int zoomFactor = 1)
{
	
	double viewMat[16];
	int ii;
	double *tcv, *rcm, *fcv, *ccv;
	double left, right, bottom, top;
	
	tcv = mxGetPr(mxGetField(camStruct, 0, "TcV"));
	rcm = mxGetPr(mxGetField(camStruct, 0, "RcM"));
	fcv = mxGetPr(mxGetField(camStruct, 0, "fcV"));
	ccv = mxGetPr(mxGetField(camStruct, 0, "ccV"));
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);	
	
	// the calibration tool box and opengl use different cooridinate system, need additional transform
	
	for (ii = 0; ii < 3; ii++) {
		viewMat[ii*4] = rcm[ii*3];
		viewMat[ii*4+1] = -rcm[ii*3+1];			
		viewMat[ii*4+2] = -rcm[ii*3+2];			
		
	}
	viewMat[12] = tcv[0];
	viewMat[13] = -tcv[1];
	viewMat[14] = -tcv[2];
	
	viewMat[3] = 0;
	viewMat[7] = 0;
	viewMat[11] = 0;
	viewMat[15] = 1;
	
	// debug
	/*
	 for (ii = 0; ii < 16; ii++) {
	 printf("%f ", 	viewMat[ii]);
	 }
	 printf("\n");
	 */
	
	glMatrixMode(GL_MODELVIEW);   
    glLoadMatrixd(viewMat);	
    
    // debug
    //glLoadIdentity();
    //gluLookAt(0, 0, -15, 0, 0, 0, 0, 1, 0);
	
	left = - ccv[0] / fcv[0] * zNear;
	bottom = (ccv[1] - (imgHeight-1)) / fcv[1] * zNear;
	right = (imgWidth - 1 - ccv[0]) / fcv[0] * zNear;
	top = ccv[1] / fcv[1] * zNear;	
	
	
    glMatrixMode(GL_PROJECTION);  
    glLoadIdentity();
    
	// use glFrustum to better control the window position
	glFrustum(left, right, bottom, top, zNear, zFar);
	
	// debug
	//gluPerspective(60, 1, zNear, zFar);
	//printf("ZNear %f, zFar %f, width %d, height, %d\n", zNear, zFar, imgWidth, imgHeight);
    
    // set the view port 
    glViewport(0, 0, imgWidth * zoomFactor, imgHeight * zoomFactor);
	
}

void drawPatch(GLuint listName, GLubyte *imageBuffer, unsigned int imgHeight, unsigned int imgWidth, unsigned int zoomFactor = 1)
{
	glCallList (listName);
	glFlush ();
	
	// This is a temporary bug fix for Nvidia's open program
	// seems the width of the pixel has to be a multiple of 4
	// for other width, we have to pad the width and remove it later
	unsigned int paddedWidth = imgWidth * zoomFactor % 4;
	
	if (paddedWidth != 0) {
		paddedWidth = 4 - paddedWidth + imgWidth * zoomFactor;
	} else {
		paddedWidth = imgWidth * zoomFactor;
	}
	
	glReadPixels(0, 0, paddedWidth, imgHeight * zoomFactor, GL_RGB, GL_UNSIGNED_BYTE, imageBuffer);	
}


void colorDemodulation(GLubyte *imageBuffer, unsigned int channelModFactor, 
					   unsigned char *maskData, unsigned int imgHeight, unsigned int imgWidth, int zoomFactor,
					   // output
					   unsigned int *LabeledImage)

{
	unsigned int channel1Factor = 256 * 256 / channelModFactor / channelModFactor / channelModFactor;
	unsigned int channel2Factor = 256 / channelModFactor / channelModFactor;
	
	unsigned int colorImageIndex;
	
	// This is a temporary bug fix for Nvidia's open program
	// seems the width of the pixel has to be a multiple of 4
	// for other width, we have to pad the width and remove it later
	unsigned int paddedWidth = imgWidth * zoomFactor % 4;
	if (paddedWidth != 0) {
		paddedWidth = 4 - paddedWidth + imgWidth * zoomFactor;
	} else {
		paddedWidth = imgWidth * zoomFactor;
	}
	
	for (unsigned int jj = 0, imageIndex = 0; jj < imgWidth * zoomFactor; jj++) {
		for (unsigned int ii = 0; ii < imgHeight * zoomFactor; ii++, imageIndex++) {			
			if (!maskData || maskData[jj/zoomFactor * imgHeight + ii/zoomFactor] > 0) {
				// need to map the openGL image pixel order to matlab's order
				colorImageIndex = (jj + (imgHeight*zoomFactor -1-ii) * paddedWidth) * 3;
				LabeledImage[imageIndex] = imageBuffer[colorImageIndex] * channel1Factor + imageBuffer[colorImageIndex + 1] * channel2Factor
				+ imageBuffer[colorImageIndex + 2] / channelModFactor;
			}
		}	
	}	
}



#endif  // OFFSCREEN_COMMON_H