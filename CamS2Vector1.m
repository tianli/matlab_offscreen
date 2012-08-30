%% Wish 1:	Decompose matlab camera parameter to different vectors

function [ViewPointV, LookAtV, UpV, fcV, ccV] = CamS2Vector1(CamParamS)

	InvRcM = inv(CamParamS.RcM);
	%% compute the viewpoint
	ViewPointV = InvRcM * (-CamParamS.TcV);
	
	%% the lookat vector is the camera's z axis
	LookAtV = InvRcM * [0; 0; 1];
	
	%% the Up vector is the camera's -y axis
	UpV = InvRcM * [0; -1; 0];
	
	fcV = CamParamS.fcV;
	ccV = CamParamS.ccV;
