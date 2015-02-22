%% function [FaceColorT, FaceVisibleM] = BatchFaceColorGrad(FM, VM, CamParamSA, ...
%			ScreenSizeV, zoomFactor, TestImageA, ImageMaskA, channelModFactor, zNearFarV)
%
%	This function will use OpenGL to render a set of triangular faces (defined by FM and VM arrays)
%
function [FaceColorT, FaceVisibleM] = BatchFaceColorGrad(FM, VM, CamParamSA, ...
			ScreenSizeV, zoomFactor, TestImageA, ImageMaskA, channelModFactor, zNearFarV)

  MexGlutInit;

	%% Check the input data type
	imageNum = length(TestImageA);
	for ii = 1 : imageNum
	    if (~isa(TestImageA{ii}, 'uint8'))
	        error('Offscreen:BatchFaceColorGrad:Wrong Input Type', 'Input image should be uint8');
	    end

		if (~isa(ImageMaskA{ii}, 'uint8'))
		    error('Offscreen:BatchFaceColorGrad:Wrong Input Type', 'Mask image should be uint8');
		end
	end
	
	if (~isa(FM, 'double'))
		FM = double(FM);
	end
	
	[FaceColorT, FaceVisibleM] = BatchFaceColorGradImpl(FM, VM, CamParamSA, ...
			ScreenSizeV, zoomFactor, TestImageA, ImageMaskA, channelModFactor, zNearFarV);
