/**
 * Wish 2:	use byte image input and float gradient input to reduce the memory footprint, output double.
 *			This revision is successful, the memory consumption for 6 1024*768 3 band color image reduced from 260MB to 130MB
 * Wish 3:	Apply binary masks to mask out points that are outside the object silhouette, normalize the intensity to 0 - 1
 * Wish 4:	Support double size rendering.
 *
 */

/**
 *
 *	LabeledImageT = BatchFaceColorGrad?(FM, VM, CamParamSA, ScreenSizeV, OriginalImageA, MaskImageA, channelModFactor, zNearFarV)
 *	[FaceColorT, FacePixelCountM] = BatchFaceColorGrad?(FM, VM, CamParamSA, ScreenSizeV, OriginalImageA, MaskImageA, channelModFactor, zNearFarV)
 *  [FaceColorT, FacePixelCountM, FaceColorStdT] = BatchFaceColorGrad?(FM, VM, CamParamSA, ScreenSizeV, OriginalImageA, MaskImageA, channelModFactor, zNearFarV)
 *	[FaceColorT, FacePixelCountM, FaceCGradXT, FaceCGradYT] = BatchFaceColorGrad?(FM, VM, CamParamSA, ScreenSizeV, OriginalImageA, MaskImageA, channelModFactor, zNearFarV, GradXImageA, GradYImageA)
 */
 
#include <mex.h>
#include <math.h>
#include "OffscreenGL.h"
#include "OffscreenCommon.h"

void extractFaceColor(GLubyte *imageBuffer, unsigned int channelModFactor, 
	unsigned char *imgData, unsigned char *maskData, 
	unsigned int imgHeight, unsigned int imgWidth, int zoomFactor, unsigned int bandNum, unsigned int fNum, 
	// output
	double *FaceColorM, double *FacePixelCountV)
{
	unsigned int ii, jj, channel1Factor, channel2Factor;
	unsigned int faceIndex, imageIndex, colorImageIndex;
	
	int imgSize;
	unsigned int bi, writeIndex;
	
	imgSize = imgHeight * imgWidth;
	
	channel1Factor = 256 * 256 / channelModFactor / channelModFactor / channelModFactor;
	channel2Factor = 256 / channelModFactor / channelModFactor;
	
	// This is a temporary bug fix for Nvidia's open program
	// seems the width of the pixel has to be a multiple of 4
	// for other width, we have to pad the width and remove it later
	unsigned int paddedWidth = imgWidth * zoomFactor % 4;
	if (paddedWidth != 0) {
		paddedWidth = 4 - paddedWidth + imgWidth * zoomFactor;
	} else {
		paddedWidth = imgWidth * zoomFactor;
	}
	
	unsigned char imgInterpData[2][2][3];
	unsigned char maskInterpData[2][2];
	int zi, zj, baseIndex;
	double deltai, deltaj;
	double t1, t2, t3, t4;
	
	/// debug
	//printf("ExtractFaceColor: \n");
	
	/// Assume the 4 border line will be masked out so in current version do not consider them.	
	/// ii and jj are the matlab index, they should be convert to opengl index
	for (jj = 0, imageIndex = 0; jj < imgWidth - 1; jj++, imageIndex += imgHeight) {
		/// debug
		//printf("Debug %d, zoomFactor %d, imageIndex %d\n", jj, zoomFactor, imageIndex);
		for (ii = 0; ii < imgHeight - 1; ii++) {
			
			/// retrieve the 4 corner image and mask value
			for (bi = 0; bi < bandNum; bi++) {
				baseIndex = imageIndex + ii + bi * imgSize;
				imgInterpData[0][0][bi] = imgData[baseIndex];
				imgInterpData[0][1][bi] = imgData[baseIndex + imgHeight];
				imgInterpData[1][0][bi] = imgData[baseIndex + 1];
				imgInterpData[1][1][bi] = imgData[baseIndex + 1 + imgHeight];
			}
			
			baseIndex = imageIndex + ii;
			maskInterpData[0][0] = maskData[baseIndex];
			maskInterpData[0][1] = maskData[baseIndex + imgHeight];
			maskInterpData[1][0] = maskData[baseIndex + 1];
			maskInterpData[1][1] = maskData[baseIndex + 1 + imgHeight];

			/// fill the inside data using interpolation
			/// now baseIndex is used to index the opengl image
			baseIndex = (imgHeight * zoomFactor - 1 - ii * zoomFactor - zoomFactor / 2) * paddedWidth + (jj * zoomFactor + zoomFactor / 2);
						
			for (zj = 0; zj < zoomFactor; zj++) {
				for (zi = 0; zi < zoomFactor; zi++) {

					if (maskInterpData[zi * 2 >= zoomFactor] [ zj * 2 >= zoomFactor ] > 0) {
						colorImageIndex = (baseIndex - zi * paddedWidth + zj) * 3;
						faceIndex = imageBuffer[colorImageIndex] * channel1Factor + imageBuffer[colorImageIndex + 1] * channel2Factor
													+ imageBuffer[colorImageIndex + 2] / channelModFactor;
													
						if ((faceIndex > 0) && (faceIndex <= fNum)){
							FacePixelCountV[faceIndex-1]++;
							/// compute the offset
							deltai = zi / (double) zoomFactor + ((zoomFactor - 1) % 2) * 0.5 / zoomFactor;
							deltaj = zj / (double) zoomFactor + ((zoomFactor - 1) % 2) * 0.5 / zoomFactor;
							t1 = (1 - deltai) * (1 - deltaj);
							t2 = (1 - deltai) * deltaj;
							t3 = deltai * (1 - deltaj);
							t4 = deltai * deltaj;
							/// bilinear interpolation
							for (bi = 0, writeIndex = faceIndex-1; bi < bandNum; bi++, writeIndex += fNum) {
								FaceColorM[writeIndex] += imgInterpData[0][0][bi] * t1 + imgInterpData[0][1][bi] * t2
														+ imgInterpData[1][0][bi] * t3 + imgInterpData[1][1][bi] * t4 ;
							}
						}
					}
				}
			}			
		}	
	}
	
	// compute the average color
	for (ii = 0; ii < fNum; ii++) {
		if (FacePixelCountV[ii] > 0) {
			for (bi = 0; bi < bandNum; bi++) {
				FaceColorM[ii + bi*fNum] /= FacePixelCountV[ii] * 256.0;
			}
		}
	}
}


void extractFaceColorStd(GLubyte *imageBuffer, unsigned int channelModFactor, 
	unsigned char *imgData, unsigned char *maskData, 
	unsigned int imgHeight, unsigned int imgWidth, int zoomFactor, unsigned int bandNum, unsigned int fNum, 
	// output
	double *FaceColorM, double *FacePixelCountV, double *FaceColorStdM)
{
	unsigned int ii, jj, channel1Factor, channel2Factor;
	unsigned int faceIndex, imageIndex, colorImageIndex;
	
	int imgSize;
	unsigned int bi, writeIndex;
	
	imgSize = imgHeight * imgWidth;
	
	channel1Factor = 256 * 256 / channelModFactor / channelModFactor / channelModFactor;
	channel2Factor = 256 / channelModFactor / channelModFactor;
	
	// This is a temporary bug fix for Nvidia's open program
	// seems the width of the pixel has to be a multiple of 4
	// for other width, we have to pad the width and remove it later
	unsigned int paddedWidth = imgWidth * zoomFactor % 4;
	if (paddedWidth != 0) {
		paddedWidth = 4 - paddedWidth + imgWidth * zoomFactor;
	} else {
		paddedWidth = imgWidth * zoomFactor;
	}
	
	unsigned char imgInterpData[2][2][3];
	unsigned char maskInterpData[2][2];
	int zi, zj, baseIndex;
	double deltai, deltaj;
	double t1, t2, t3, t4;
	
	double normedColor;
	
	/// debug
	//printf("ExtractFaceColor: \n");
	
	/// Assume the 4 border line will be masked out so in current version do not consider them.	
	/// ii and jj are the matlab index, they should be convert to opengl index
	for (jj = 0, imageIndex = 0; jj < imgWidth - 1; jj++, imageIndex += imgHeight) {
		/// debug
		//printf("Debug %d, zoomFactor %d, imageIndex %d\n", jj, zoomFactor, imageIndex);
		for (ii = 0; ii < imgHeight - 1; ii++) {
			
			/// retrieve the 4 corner image and mask value
			for (bi = 0; bi < bandNum; bi++) {
				baseIndex = imageIndex + ii + bi * imgSize;
				imgInterpData[0][0][bi] = imgData[baseIndex];
				imgInterpData[0][1][bi] = imgData[baseIndex + imgHeight];
				imgInterpData[1][0][bi] = imgData[baseIndex + 1];
				imgInterpData[1][1][bi] = imgData[baseIndex + 1 + imgHeight];
			}
			
			baseIndex = imageIndex + ii;
			maskInterpData[0][0] = maskData[baseIndex];
			maskInterpData[0][1] = maskData[baseIndex + imgHeight];
			maskInterpData[1][0] = maskData[baseIndex + 1];
			maskInterpData[1][1] = maskData[baseIndex + 1 + imgHeight];

			/// fill the inside data using interpolation
			/// now baseIndex is used to index the opengl image
			baseIndex = (imgHeight * zoomFactor - 1 - ii * zoomFactor - zoomFactor / 2) * paddedWidth + (jj * zoomFactor + zoomFactor / 2);
						
			for (zj = 0; zj < zoomFactor; zj++) {
				for (zi = 0; zi < zoomFactor; zi++) {

					if (maskInterpData[zi * 2 >= zoomFactor] [ zj * 2 >= zoomFactor ] > 0) {
						colorImageIndex = (baseIndex - zi * paddedWidth + zj) * 3;
						faceIndex = imageBuffer[colorImageIndex] * channel1Factor + imageBuffer[colorImageIndex + 1] * channel2Factor
													+ imageBuffer[colorImageIndex + 2] / channelModFactor;
													
						if ((faceIndex > 0) && (faceIndex <= fNum)){
							FacePixelCountV[faceIndex-1]++;
							/// compute the offset
							deltai = zi / (double) zoomFactor + ((zoomFactor - 1) % 2) * 0.5 / zoomFactor;
							deltaj = zj / (double) zoomFactor + ((zoomFactor - 1) % 2) * 0.5 / zoomFactor;
							t1 = (1 - deltai) * (1 - deltaj);
							t2 = (1 - deltai) * deltaj;
							t3 = deltai * (1 - deltaj);
							t4 = deltai * deltaj;
							/// bilinear interpolation
							for (bi = 0, writeIndex = faceIndex-1; bi < bandNum; bi++, writeIndex += fNum) {
								normedColor = (imgInterpData[0][0][bi] * t1 + imgInterpData[0][1][bi] * t2
												+ imgInterpData[1][0][bi] * t3 + imgInterpData[1][1][bi] * t4) / 256.0;
								FaceColorM[writeIndex] += normedColor;
								FaceColorStdM[writeIndex] += normedColor * normedColor;
							}
						}
					}
				}
			}			
		}	
	}
	
	// compute the average color
	for (ii = 0; ii < fNum; ii++) {
		if (FacePixelCountV[ii] > 0) {
			for (bi = 0; bi < bandNum; bi++) {
				FaceColorM[ii + bi*fNum] /= FacePixelCountV[ii];
				FaceColorStdM[ii + bi*fNum] = sqrt(FaceColorStdM[ii + bi*fNum] / FacePixelCountV[ii] - FaceColorM[ii + bi*fNum] * FaceColorM[ii + bi*fNum]);
			}
		}
	}
}


void extractFaceColorGrad(GLubyte *imageBuffer, unsigned int channelModFactor, 
	unsigned char *imgData, unsigned char *maskData, float *gradXData, float *gradYData, 
	unsigned int imgHeight, unsigned int imgWidth, int zoomFactor, unsigned int bandNum, unsigned int fNum, 
	// output
	double *FaceColorM, double *FaceGradXM, double *FaceGradYM, double *FacePixelCountV)
{
	unsigned int ii, jj, channel1Factor, channel2Factor;
	unsigned int faceIndex, imageIndex, colorImageIndex;
	
	int imgSize;
	unsigned int bi, writeIndex;
	
	imgSize = imgHeight * imgWidth;
	
	channel1Factor = 256 * 256 / channelModFactor / channelModFactor / channelModFactor;
	channel2Factor = 256 / channelModFactor / channelModFactor;
	
	// This is a temporary bug fix for Nvidia's open program
	// seems the width of the pixel has to be a multiple of 4
	// for other width, we have to pad the width and remove it later
	unsigned int paddedWidth = imgWidth * zoomFactor % 4;
	if (paddedWidth != 0) {
		paddedWidth = 4 - paddedWidth + imgWidth * zoomFactor;
	} else {
		paddedWidth = imgWidth * zoomFactor;
	}
	
	unsigned char imgInterpData[2][2][MAX_COLOR_CHANNEL];
	float gradXInterpData[2][2][MAX_COLOR_CHANNEL];
	float gradYInterpData[2][2][MAX_COLOR_CHANNEL];
	unsigned char maskInterpData[2][2];
	int zi, zj, baseIndex;
	double deltai, deltaj;
	double t1, t2, t3, t4;
	
	/// debug
	//printf("ExtractFaceColor: \n");
	
	/// Assume the 4 border line will be masked out so in current version do not consider them.	
	/// ii and jj are the matlab index, they should be convert to opengl index
	for (jj = 0, imageIndex = 0; jj < imgWidth - 1; jj++, imageIndex += imgHeight) {
		/// debug
		//printf("Debug %d, zoomFactor %d, imageIndex %d\n", jj, zoomFactor, imageIndex);
		for (ii = 0; ii < imgHeight - 1; ii++) {
			
			/// retrieve the 4 corner image and mask value
			for (bi = 0; bi < bandNum; bi++) {
				baseIndex = imageIndex + ii + bi * imgSize;
				imgInterpData[0][0][bi] = imgData[baseIndex];
				imgInterpData[0][1][bi] = imgData[baseIndex + imgHeight];
				imgInterpData[1][0][bi] = imgData[baseIndex + 1];
				imgInterpData[1][1][bi] = imgData[baseIndex + 1 + imgHeight];
				
				gradXInterpData[0][0][bi] = gradXData[baseIndex];
				gradXInterpData[0][1][bi] = gradXData[baseIndex + imgHeight];
				gradXInterpData[1][0][bi] = gradXData[baseIndex + 1];
				gradXInterpData[1][1][bi] = gradXData[baseIndex + 1 + imgHeight];
				
				gradYInterpData[0][0][bi] = gradYData[baseIndex];
				gradYInterpData[0][1][bi] = gradYData[baseIndex + imgHeight];
				gradYInterpData[1][0][bi] = gradYData[baseIndex + 1];
				gradYInterpData[1][1][bi] = gradYData[baseIndex + 1 + imgHeight];
			}
			
			baseIndex = imageIndex + ii;
			maskInterpData[0][0] = maskData[baseIndex];
			maskInterpData[0][1] = maskData[baseIndex + imgHeight];
			maskInterpData[1][0] = maskData[baseIndex + 1];
			maskInterpData[1][1] = maskData[baseIndex + 1 + imgHeight];

			/// fill the inside data using interpolation
			/// now baseIndex is used to index the opengl image
			baseIndex = (imgHeight * zoomFactor - 1 - ii * zoomFactor - zoomFactor / 2) * paddedWidth + (jj * zoomFactor + zoomFactor / 2);
						
			for (zj = 0; zj < zoomFactor; zj++) {
				for (zi = 0; zi < zoomFactor; zi++) {

					if (maskInterpData[zi * 2 >= zoomFactor] [ zj * 2 >= zoomFactor ] > 0) {
						colorImageIndex = (baseIndex - zi * paddedWidth + zj) * 3;
						faceIndex = imageBuffer[colorImageIndex] * channel1Factor + imageBuffer[colorImageIndex + 1] * channel2Factor
													+ imageBuffer[colorImageIndex + 2] / channelModFactor;
													
						if ((faceIndex > 0) && (faceIndex <= fNum)){
							FacePixelCountV[faceIndex-1]++;
							/// compute the offset
							deltai = zi / (double) zoomFactor + ((zoomFactor - 1) % 2) * 0.5 / zoomFactor;
							deltaj = zj / (double) zoomFactor + ((zoomFactor - 1) % 2) * 0.5 / zoomFactor;
							t1 = (1 - deltai) * (1 - deltaj);
							t2 = (1 - deltai) * deltaj;
							t3 = deltai * (1 - deltaj);
							t4 = deltai * deltaj;
							/// bilinear interpolation
							for (bi = 0, writeIndex = faceIndex-1; bi < bandNum; bi++, writeIndex += fNum) {
								FaceColorM[writeIndex] += imgInterpData[0][0][bi] * t1 + imgInterpData[0][1][bi] * t2
														+ imgInterpData[1][0][bi] * t3 + imgInterpData[1][1][bi] * t4 ;
								FaceGradXM[writeIndex] += gradXInterpData[0][0][bi] * t1 + gradXInterpData[0][1][bi] * t2
														+ gradXInterpData[1][0][bi] * t3 + gradXInterpData[1][1][bi] * t4 ;														
								FaceGradYM[writeIndex] += gradYInterpData[0][0][bi] * t1 + gradYInterpData[0][1][bi] * t2
														+ gradYInterpData[1][0][bi] * t3 + gradYInterpData[1][1][bi] * t4 ;								
							}
						}
					}
				}
			}			
		}	
	}
	
	// compute the average color
	double fCount;
	
	for (ii = 0; ii < fNum; ii++) {
		fCount = FacePixelCountV[ii] * 256.0;
		if (FacePixelCountV[ii] > 0) {
			for (bi = 0; bi < bandNum; bi++) {
				writeIndex = ii + bi*fNum;				
				FaceColorM[writeIndex] /= fCount;
				FaceGradXM[writeIndex] /= fCount;
				FaceGradYM[writeIndex] /= fCount;
			}
		}
	}
}


void getBatchFaceColorStd(
	// input
	double *FM, int fNum, double *VM, int vNum, const mxArray *CamParamSA, int maxHeight, int maxWidth, int zoomFactor,
	const mxArray *OriginalImageA, const mxArray *MaskImageA,
	unsigned int channelModFactor, double zNear, double zFar,
	// output
	double *FaceColorT, double *FacePixelCountM, double *FaceColorStdT
	)
{
	int imageNum, i;
	GLuint batchList;
	mxArray *currentCamParamS;
	GLubyte *imageBuffer;
	mxArray *currentImage, *currentMask;
	unsigned char *currentImageData, *currentMaskData;
	int imgHeight, imgWidth;
	int dimNum, bandNum;
	const int *dims;
	
	imageBuffer = (GLubyte *) malloc(maxHeight * maxWidth * COLOR_MODULATE_CHANNEL * zoomFactor * zoomFactor);
	
	imageNum = mxGetM(CamParamSA);
	
	batchList = createDisplayList(FM, fNum, VM, vNum, channelModFactor);

	currentImage = mxGetCell(OriginalImageA, 0);
	//currentImageData = mxGetPr(currentImage);
	
	dimNum = mxGetNumberOfDimensions(currentImage);
	dims = mxGetDimensions(currentImage);
	imgHeight = dims[0];
	imgWidth = dims[1];
	if (dimNum == 2) {
		bandNum = 1;	
	} else {
		bandNum = dims[2];
	}
				
	//memset(FaceColorT, 0, sizeof(double) * fNum * bandNum * imageNum);
	//memset(FacePixelCountM, 0, sizeof(double) * fNum * imageNum);
	
	printf("Facet Number: %d, Vertex Number: %d\nProjecting images: \n", fNum, vNum);
		
	for (i = 0; i < imageNum; i++) {
		
		printf(" %d ", i);
		if (i % 8 == 7) {
			printf("\n");
		}		
		
		currentCamParamS = mxGetCell(CamParamSA, i);
		
		currentImage = mxGetCell(OriginalImageA, i);
		currentImageData = (unsigned char *)mxGetData(currentImage);
		
		currentMask = mxGetCell(MaskImageA, i);
		currentMaskData = (unsigned char *)mxGetData(currentMask);
		
		dims = mxGetDimensions(currentImage);
		imgHeight = dims[0];
		imgWidth = dims[1];

		cameraSetup(currentCamParamS, zNear, zFar, imgHeight, imgWidth, zoomFactor);
		drawPatch(batchList, imageBuffer, imgHeight, imgWidth, zoomFactor);		
		
		if (FaceColorStdT == NULL) {
			extractFaceColor(imageBuffer, channelModFactor, currentImageData, currentMaskData, imgHeight, imgWidth, zoomFactor, bandNum, fNum, 
				FaceColorT + i*fNum * bandNum, FacePixelCountM + i*fNum);	
		} else {
			extractFaceColorStd(imageBuffer, channelModFactor, currentImageData, currentMaskData, imgHeight, imgWidth, zoomFactor, bandNum, fNum, 
				FaceColorT + i*fNum * bandNum, FacePixelCountM + i*fNum, FaceColorStdT + i*fNum*bandNum);
		}		
	}
	
	free(imageBuffer);	
}


void getBatchFaceColorGrad(
	// input
	double *FM, int fNum, double *VM, int vNum, const mxArray *CamParamSA, int maxHeight, int maxWidth, int zoomFactor,
	const mxArray *OriginalImageA, const mxArray *MaskImageA,
	const mxArray *GradXImageA, const mxArray *GradYImageA, 
	unsigned int channelModFactor, double zNear, double zFar, 
	// output
	double *FaceColorT, double *FaceColorGradXT, double *FaceColorGradYT, double *FacePixelCountM
	)
{
	int imageNum, i;
	GLuint batchList;
	mxArray *currentCamParamS;
	GLubyte *imageBuffer;
	mxArray *currentImage, *currentMask, *currentGradX, *currentGradY;
	unsigned char *currentImageData, *currentMaskData;
	float *currentGradXData, *currentGradYData;
	int imgHeight, imgWidth;
	int dimNum, bandNum;
	const int *dims;
	
	imageBuffer = (GLubyte *) malloc(maxHeight * maxWidth * COLOR_MODULATE_CHANNEL * zoomFactor * zoomFactor);
	
	imageNum = mxGetM(CamParamSA);

	
	currentImage = mxGetCell(OriginalImageA, 0);
	//currentImageData = mxGetPr(currentImage);
	
	batchList = createDisplayList(FM, fNum, VM, vNum, channelModFactor);
	
	dimNum = mxGetNumberOfDimensions(currentImage);
	dims = mxGetDimensions(currentImage);
	imgHeight = dims[0];
	imgWidth = dims[1];
	if (dimNum == 2) {
		bandNum = 1;	
	} else {
		bandNum = dims[2];
	}
				
	//memset(FaceColorT, 0, sizeof(double) * fNum * bandNum * imageNum);
	//memset(FaceColorGradXT, 0, sizeof(double) * fNum * bandNum * imageNum);
	//memset(FaceColorGradYT, 0, sizeof(double) * fNum * bandNum * imageNum);	
	//memset(FacePixelCountM, 0, sizeof(double) * fNum * imageNum);
	
	
	printf("Facet Number: %d, Vertex Number: %d\nProjecting images: \n", fNum, vNum);
		
	for (i = 0; i < imageNum; i++) {
		
		printf(" %d ", i);
		if (i % 8 == 7) {
			printf("\n");
		}		
		
		currentCamParamS = mxGetCell(CamParamSA, i);
		
		currentImage = mxGetCell(OriginalImageA, i);
		currentImageData = (unsigned char *)mxGetData(currentImage);
		
		currentMask = mxGetCell(MaskImageA, i);
		currentMaskData = (unsigned char *)mxGetData(currentMask);
		
		dims = mxGetDimensions(currentImage);
		imgHeight = dims[0];
		imgWidth = dims[1];
		
		currentGradX = mxGetCell(GradXImageA, i);
		currentGradXData = (float *) mxGetData(currentGradX);
		
		currentGradY = mxGetCell(GradYImageA, i);
		currentGradYData = (float *) mxGetData(currentGradY);		

		cameraSetup(currentCamParamS, zNear, zFar, imgHeight, imgWidth, zoomFactor);
		drawPatch(batchList, imageBuffer, imgHeight, imgWidth, zoomFactor);		
		
		extractFaceColorGrad(imageBuffer, channelModFactor, currentImageData, currentMaskData, currentGradXData, currentGradYData, imgHeight, imgWidth, zoomFactor, bandNum, fNum, 
			FaceColorT + i*fNum * bandNum, FaceColorGradXT + i*fNum*bandNum, FaceColorGradYT + i*fNum*bandNum, FacePixelCountM + i*fNum);		
		
	}
	
	free(imageBuffer);	
}

void getLabeledImages(
					  // input
					  double *FM, int fNum, double *VM, int vNum, const mxArray *CamParamSA, int maxHeight, int maxWidth, int zoomFactor,
					  const mxArray *OriginalImageA, const mxArray *MaskImageA,
					  unsigned int channelModFactor, double zNear, double zFar,
					  // output
					  mxArray *LabeledImageA
					  )
{
	int imageNum, i;
	GLuint batchList;
	mxArray *currentCamParamS;
	GLubyte *imageBuffer;
	mxArray *currentImage, *currentMask;
	unsigned char *currentImageData, *currentMaskData;
	int imgHeight, imgWidth;
	int dimNum, bandNum;
	const int *dims;
	int bufferDims[3];
	
	unsigned int *LabeledImage;
	
	imageBuffer = (GLubyte *) mxMalloc(maxHeight * maxWidth * COLOR_MODULATE_CHANNEL * zoomFactor * zoomFactor);
	
	imageNum = mxGetM(CamParamSA);
	
	batchList = createDisplayList(FM, fNum, VM, vNum, channelModFactor);
	printf("Facet Number: %d, Vertex Number: %d\nProjecting images: \n", fNum, vNum);
	
	for (i = 0; i < imageNum; i++) {
		
		printf(" %d ", i);
		if (i % 8 == 7) {
			printf("\n");
		}
		
		currentCamParamS = mxGetCell(CamParamSA, i);
		
		currentImage = mxGetCell(OriginalImageA, i);
		currentImageData = (unsigned char *)mxGetData(currentImage);
		
		currentMask = mxGetCell(MaskImageA, i);
		currentMaskData = (unsigned char *)mxGetData(currentMask);
		
		dimNum = mxGetNumberOfDimensions(currentImage);
		dims = mxGetDimensions(currentImage);
		imgHeight = dims[0];
		imgWidth = dims[1];
		if (dimNum == 2) {
			bandNum = 1;	
		} else {
			bandNum = dims[2];
		}		
		
		cameraSetup(currentCamParamS, zNear, zFar, imgHeight, imgWidth, zoomFactor);
		drawPatch(batchList, imageBuffer, imgHeight, imgWidth, zoomFactor);		
		
		bufferDims[0] = dims[0] * zoomFactor;
		bufferDims[1] = dims[1] * zoomFactor;		
		mxSetCell(LabeledImageA, i, mxCreateNumericArray(2, bufferDims, mxUINT32_CLASS, mxREAL));
		LabeledImage = (unsigned int *) mxGetData(mxGetCell(LabeledImageA, i));
		
		colorDemodulation(imageBuffer, channelModFactor, currentMaskData, 
						  imgHeight, imgWidth, zoomFactor, LabeledImage);
	}
	printf("\n");
	mxFree(imageBuffer);
}

/* The gateway routine */

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
                 
{

	int output1Size[1];
	int output2Size[2];
	int output3Size[3];
	
	double *FM = mxGetPr(prhs[0]);
	int fNum = mxGetM(prhs[0]);	
	double *VM = mxGetPr(prhs[1]);
	int vNum = mxGetM(prhs[1]);
	const mxArray *CamParamSA = prhs[2];
	double *maxSizeV = mxGetPr(prhs[3]);
	double zoomFactor = mxGetScalar(prhs[4]);
	
	const mxArray *OriginalImageA = prhs[5];
	const mxArray *MaskImageA = prhs[6];
	unsigned int channelModFactor = (unsigned int)mxGetScalar(prhs[7]);
	double *zNearFarV = mxGetPr(prhs[8]);
	
	int bandNum;
	const int *dims;
	mxArray *LabeledImageA;
	mxArray *firstImage;

	double *FaceColorT, *FacePixelCountM, *FaceColorStdT;
	double *FaceCGradXT, *FaceCGradYT;
	const mxArray *GradXImageA, *GradYImageA;
	
	OffscreenGL offscreenGL((int)maxSizeV[0] * zoomFactor, (int)maxSizeV[1] * zoomFactor);
					
	if (nlhs == 1) {
		output1Size[0] = mxGetM(CamParamSA);
		plhs[0] = mxCreateCellArray(1, output1Size);		
		LabeledImageA = plhs[0];

		if (offscreenGL.RGB8Setup()) {
			mexPrintf("OpenGLCanvas setup Successful\n");
						
			getLabeledImages(FM, fNum, VM, vNum, CamParamSA, (int)maxSizeV[0], (int)maxSizeV[1], zoomFactor, OriginalImageA, MaskImageA,
				channelModFactor, zNearFarV[0], zNearFarV[1], LabeledImageA);
		
		} else {
			mexPrintf("OpenGLCanvas setup failed\n");
		}
		
	} else if ((nlhs == 2) || (nlhs == 3)) {
		
		firstImage = mxGetCell(OriginalImageA, 0);
		if (mxGetNumberOfDimensions(firstImage) == 2) {
			bandNum = 1;
		} else {
			dims = mxGetDimensions(firstImage);
			bandNum = dims[2];
		}
		
		output3Size[0] = fNum;
		output3Size[1] = bandNum;
		output3Size[2] = mxGetM(CamParamSA);			
		plhs[0] = mxCreateNumericArray(3, output3Size, mxDOUBLE_CLASS, mxREAL);
		FaceColorT = mxGetPr(plhs[0]);
		
		if (nlhs == 3) {		
			plhs[2] = mxCreateNumericArray(3, output3Size, mxDOUBLE_CLASS, mxREAL);
			FaceColorStdT = mxGetPr(plhs[2]);
		} else {
			FaceColorStdT = NULL;
		}
		
		output2Size[0] = fNum;
		output2Size[1] = mxGetM(CamParamSA);
		plhs[1] = mxCreateNumericArray(2, output2Size, mxDOUBLE_CLASS, mxREAL);
		FacePixelCountM = mxGetPr(plhs[1]);

		if (offscreenGL.RGB8Setup()) {
			mexPrintf("OpenGLCanvas setup Successful\n");
					
			getBatchFaceColorStd(FM, fNum, VM, vNum, CamParamSA, (int)maxSizeV[0], (int)maxSizeV[1], zoomFactor, OriginalImageA, MaskImageA,
				channelModFactor, zNearFarV[0], zNearFarV[1], FaceColorT, FacePixelCountM, FaceColorStdT);	
			
		} else {
			mexPrintf("OpenGLCanvas setup failed\n");
		}		
				
	} else if (nlhs == 4) {
		firstImage = mxGetCell(OriginalImageA, 0);
		if (mxGetNumberOfDimensions(firstImage) == 2) {
			bandNum = 1;
		} else {
			dims = mxGetDimensions(firstImage);
			bandNum = dims[2];
		}
		
		output3Size[0] = fNum;
		output3Size[1] = bandNum;
		output3Size[2] = mxGetM(CamParamSA);			
		plhs[0] = mxCreateNumericArray(3, output3Size, mxDOUBLE_CLASS, mxREAL);
		FaceColorT = mxGetPr(plhs[0]);
		
		GradXImageA = prhs[8];
		plhs[2] = mxCreateNumericArray(3, output3Size, mxDOUBLE_CLASS, mxREAL);
		FaceCGradXT = mxGetPr(plhs[2]);
		
		GradYImageA = prhs[9];		
		plhs[3] = mxCreateNumericArray(3, output3Size, mxDOUBLE_CLASS, mxREAL);
		FaceCGradYT = mxGetPr(plhs[3]);
				
		output2Size[0] = fNum;
		output2Size[1] = mxGetM(CamParamSA);
		plhs[1] = mxCreateNumericArray(2, output2Size, mxDOUBLE_CLASS, mxREAL);
		FacePixelCountM = mxGetPr(plhs[1]);

		if (offscreenGL.RGB8Setup()) {
			mexPrintf("OpenGLCanvas setup Successful\n");
					
			getBatchFaceColorGrad(FM, fNum, VM, vNum, CamParamSA, (int)maxSizeV[0], (int)maxSizeV[1], zoomFactor, OriginalImageA, MaskImageA,
				GradXImageA, GradYImageA, channelModFactor, zNearFarV[0], zNearFarV[1], FaceColorT, FaceCGradXT, FaceCGradYT, FacePixelCountM);	
	
		} else {
			mexPrintf("OpenGLCanvas setup failed\n");
		}		
	}
}


