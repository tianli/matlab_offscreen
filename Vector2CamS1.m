%% Wish 1:	construct the matlab camera parameter from different vectors

function CamParamS = Vector2CamS1(ViewPointV, LookAtV, UpV, fcV, ccV)

	CamParamS.fcV = fcV;
	CamParamS.ccV = ccV;
	
	%% construct the rotation matrix
  %% we will normalize LookAtV and UpV are normalized
  LookAtV = LookAtV / norm(LookAtV);
  UpV = UpV / norm(UpV);
	CamParamS.RcM = zeros(3, 3);
	CamParamS.RcM(3, :) = LookAtV';
	CamParamS.RcM(1, :) = cross(LookAtV, UpV)';
	CamParamS.RcM(1, :) = CamParamS.RcM(1, :) / norm(CamParamS.RcM(1, :));
	
	CamParamS.RcM(2, :) = cross(LookAtV, CamParamS.RcM(1, :)')';
	CamParamS.RcM(2, :) = CamParamS.RcM(2, :) / norm(CamParamS.RcM(1, :));

	%% the TcV
	CamParamS.TcV = - CamParamS.RcM * ViewPointV;
