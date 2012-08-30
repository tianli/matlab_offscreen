function DepthImageM = RenderDepthMesh(FM, VM, CamParamS, ImageSizeV, ...
						zNearFarV, zoomFactor, invertedDepth)
    MexGlutInit;

	if (~isa(FM, 'double'))
		FM = double(FM);
	end

	DepthImageM = RenderDepthMeshImpl(FM, VM, CamParamS, ImageSizeV, ...
						zNearFarV, zoomFactor);

	if (invertedDepth)
        DepthImageM = 1 - DepthImageM;
    end