%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ProjectMesh2Image: adaptor for type check and other pre-processing 
%
% FaceM: the nx3 matrix to hold the vertex indices
% VertexM: the mx3 matrix to hold the 3D vertex coordinates
% CamParamSA: the cell array of camera parameters
%				each cell contains a camera structure with these fields:
%		tcV: 3x1 vector, the translation vector
%		RcM: 3x3 matrix, the rotation matrix
%		fcV: 2x1 vector, the focal length in x and y direction
%		ccV: 2x1 vector, the camera center
%		imSizeV: 2x1 vector, the image size of the current camera [height; width]
% channelModFactor: the factor to create distinctive face color, usually 2 or 4
% ScreenSizeV: the maximum size of the screen needed, (note the the rendered image size
%				can be smaller, but no bigger)
% zoomFactor: set to non zero to zoom the rendered image, must be integer.
% ZNearFarV: the closest and furthest distance to generate the view frustum.
% 
% Copyright (c) 2009  Tianli Yu (yu_tianli@hotmail.com)
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function LabeledImageA = ProjectMesh2Image(FaceM, VertexM, CamParamSA, channelModFactor, ... 
					ScreenSizeV, zoomFactor, ZNearFarV)

    MexGlutInit;

	%% Check the input data type
	
	if (~isa(FaceM, 'double'))
		FaceM = double(FaceM);
	end
	
	if (~isa(VertexM, 'double'))
		VertexM = double(VertexM);
	end
	
	LabeledImageA = ProjectMesh2ImageImpl(FaceM, VertexM, CamParamSA, channelModFactor, ... 
					ScreenSizeV, zoomFactor, ZNearFarV);
