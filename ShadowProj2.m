%% function FacePixelCountM = ShadowProj2(FM, VM, LightDirM, ObjCenterV, maxRadius,
%% 							majorAxisM, ScreenSizeV, channelModFactor)
%% 

function BitFaceShadowM = ShadowProj2(FM, VM, LightDirM, ObjCenterV, maxRadius, majorAxisM, ScreenSizeV, channelModFactor)

    MexGlutInit;

	if (~isa(FM, 'double'))
		FM = double(FM);
	end
	
	BitFaceShadowM = ShadowProj2Impl(FM, VM, LightDirM, ObjCenterV, maxRadius, majorAxisM, ScreenSizeV, channelModFactor);

