function [DepthImageM, CameraCoordT] = RenderDepthMesh(FM, VM, CamParamS, ImageSizeV, ...
						zNearFarV, zoomFactor, invertedDepth)
    MexGlutInit;

	if (~isa(FM, 'double'))
		FM = double(FM);
	end

	DepthImageM = RenderDepthMeshImpl(FM, VM, CamParamS, ImageSizeV, ...
						zNearFarV, zoomFactor);

  %% converts the z buffer to real 3D value for easier reading.
  % z algorithm from http://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
  NormalizedZM = DepthImageM * 2 - 1.0;
  RealZM = 2 * zNearFarV(1) * zNearFarV(2) ./ (zNearFarV(2) + zNearFarV(1)
          - NormalizedZM * (zNearFarV(2) - zNearFarV(1)));

  %% calculates the 3d coordinates from the real depth value. for the screen
  
  [XM, YM] = meshgrid(1:ImageSizeV(1), 1:ImageSizeV(2));

  RealXM = (XM - CamParamS.ccV(1)) / CamParamS.fcV(1) .* RealZM;
  RealYM = (YM - CamParamS.ccV(2)) / CamParamS.fcV(2) .* RealZM;
  
  CameraCoordT = cat(3, RealXM, RealYM, RealZM);
  
  if (invertedDepth)
    DepthImageM = 1 - DepthImageM;
  end
