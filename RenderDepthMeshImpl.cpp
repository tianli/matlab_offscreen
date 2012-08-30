#include <mex.h>
#include <math.h>
#include "OffscreenGL.h"
#include "OffscreenCommon.h"

void drawPatchToDepthBuffer(GLuint listName, float *imageBuffer, unsigned int imgHeight, unsigned int imgWidth, unsigned int zoomFactor = 1)
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
    
#ifndef NDEBUG
mexPrintf("paddedWidth = %d, imgHeight * zoomFactor=%d\n", paddedWidth, imgHeight * zoomFactor);
#endif

    // Read off of the depth buffer
    float *dataBuffer = (float *)mxMalloc(paddedWidth * imgHeight * zoomFactor * sizeof(GL_FLOAT));
    glReadPixels(0,0,paddedWidth, imgHeight * zoomFactor, GL_DEPTH_COMPONENT,GL_FLOAT, dataBuffer);

    // reorder the pixel data for the opengl to matlab conversion
    unsigned int imgSize = imgHeight * imgWidth * zoomFactor * zoomFactor;
    unsigned int imgSize2 = imgSize * 2;
    unsigned int matlabImgIndex = 0;
    unsigned int oglImageIndex = 0;

    for (int j = 0; j < imgWidth * zoomFactor; j++) {
        for (int i = 0; i < imgHeight * zoomFactor; i++, matlabImgIndex++) {
            oglImageIndex = (j + (imgHeight*zoomFactor -1-i) * paddedWidth);
            imageBuffer[matlabImgIndex] = dataBuffer[oglImageIndex];
        }
    }

    mxFree(dataBuffer);
}

static void renderDepthMesh(double *FM, int fNum, double *VM, int vNum,
        const mxArray *CamParamS, double *imgSizeV, double *zNearFarV, unsigned int zoomFactor,
        // output
        float *imgBuffer)
{
    cameraSetup(CamParamS, zNearFarV[0], zNearFarV[1], (unsigned int) imgSizeV[0], (unsigned int) imgSizeV[1], zoomFactor);
    
#ifndef NDEBUG
mexPrintf("Start to create the display list: fNum=%d, vNum=%d\n", fNum, vNum);
#endif

    GLuint list = createDisplayList(FM, fNum, VM, vNum, 1);

#ifndef NDEBUG
mexPrintf("Start to draw the patch\n");
#endif

    drawPatchToDepthBuffer(list, imgBuffer, (int) imgSizeV[0], (int) imgSizeV[1], zoomFactor);
}

void mexFunction(int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[])
{
    // get the vertex array, face array, and color array
    double *FM = mxGetPr(prhs[0]);
    int fNum = mxGetM(prhs[0]);
    double *VM = mxGetPr(prhs[1]);
    int vNum = mxGetM(prhs[1]);
    
    // get the camera parameters
    const mxArray *CamParamS = prhs[2];
    double *imgSizeV = mxGetPr(prhs[3]);
    double *zNearFarV = mxGetPr(prhs[4]);
    double zoomFactor = mxGetScalar(prhs[5]);
    
    //printf("in RCMI: imgSizeV[0] = %f, imgSizeV[1] = %f\n", imgSizeV[0], imgSizeV[1]);
    
    int output2Size[2];
    output2Size[0] = (int)0;
    output2Size[1] = (int)1;
    
    /*
     * printf("In RCMI: imgSizeV[0]*zoomFactor = %d, imgSizeV[1]*zoomFactor = %d\n",
     * (int)(imgSizeV[0] * zoomFactor), (int)(imgSizeV[1] * zoomFactor));
     */
    
    int maxHeight = (int)(imgSizeV[0] * zoomFactor);
    int maxWidth = (int)(imgSizeV[1] * zoomFactor);
    
    OffscreenGL offscreenGL(maxHeight, maxWidth);
    
    float *imgBuffer;
    
    if (offscreenGL.RGB8Setup()){
        mexPrintf("OpenGLCanvas setup Successful\n");
        output2Size[0] = (int) (imgSizeV[0] * zoomFactor);
        output2Size[1] = (int) (imgSizeV[1] * zoomFactor);
        
        plhs[0] = mxCreateNumericArray(2, output2Size, mxSINGLE_CLASS, mxREAL);
        imgBuffer = (float *) mxGetData(plhs[0]);
        
        
        renderDepthMesh(FM, fNum, VM, vNum, CamParamS, imgSizeV,
                zNearFarV, (unsigned int) zoomFactor, imgBuffer);
        
        
    } else {
        mexPrintf("OpenGLCanvas setup failed\n");
    }
}

