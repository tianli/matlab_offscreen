function ColorImageT = RenderColorMesh(FM, VM, ColorM, CamParamS, ImageSizeV, ...
						zNearFarV, zoomFactor)
    MexGlutInit;

	if (~isa(FM, 'double'))
		FM = double(FM);
	end

	if (~isa(ColorM, 'single'))
	    ColorM = single(ColorM);
	end
	ColorImageT = RenderColorMeshImpl(FM, VM, ColorM, CamParamS, ImageSizeV, ...
						zNearFarV, zoomFactor);
