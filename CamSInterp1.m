%%
% function CamParamS = CamSInterp1(CamParam1S, CamParam2S, lamda)
% return a camera paramter that is an interpolated version of the two
% CamParamS = lamda * CamParam1S + (1-lamda) * CamParam2S

function CamParamS = CamSInterp1(CamParam1S, CamParam2S, lamda)

	[ViewPoint1V, LookAt1V, Up1V, fc1V, cc1V] = CamS2Vector1(CamParam1S);
	[ViewPoint2V, LookAt2V, Up2V, fc2V, cc2V] = CamS2Vector1(CamParam2S);
	
	ViewPointV = lamda * ViewPoint1V + (1-lamda) * ViewPoint2V;
	LookAtV = lamda * LookAt1V + (1-lamda) * LookAt2V;
	LookAtV = LookAtV / norm(LookAtV);
	UpV = lamda * Up1V + (1-lamda) * Up2V;
	UpV = UpV / norm(UpV);
	
	fcV = lamda * fc1V + (1-lamda) * fc2V;
	ccV = lamda * cc1V + (1-lamda) * cc2V;
	
	CamParamS = Vector2CamS1(ViewPointV, LookAtV, UpV, fcV, ccV);
	
	
