%% Test script for Offscreen rendering toolbox.

function OffScreenTest(interactive)

	if (~exist('interactive'))
	    interactive = false;
	end

	cd('TestData');
	load('buddha11hull022.mat');
	loadbuddhaimagecam;
	cd('..');
	
	channelModFactor = 4;
	zoomFactor = 2;

	[FaceColorT, FaceVisibleM] = BatchFaceColorGrad(FM, VM, CamParamStructA, [480; 640], zoomFactor, TestImageA, ImageMaskA, channelModFactor, [10;20]);
	%LabeledImageA = BatchFaceColorGrad(FM, VM, CamParamStructA, [480; 640], zoomFactor, TestImageA, ImageMaskA, channelModFactor, [10;20]);
	%LabeledImageA = ProjectMesh2Image(FM, VM, CamParamStructA, channelModFactor, [480; 640], zoomFactor, [10;20]);
	%keyboard;

    %% Do not call patch in octave as it is very slow and does not have cameratoolbar.
    if (~exist('octave_config_info'))
	%% try to draw the vertex with colors
	figure;
	pat = patch('faces', FM, 'vertices', VM, 'FaceVertexCData', FaceColorT(:, :, 1), 'FaceColor', 'flat', 'EdgeColor', 'none');
	colormap(gray);
	view(3);
	title('Color image projected onto the mesh');
	cameratoolbar('show')
	cameratoolbar('SetMode', 'orbit', 'SetCoordSys', 'z');
	axis equal;


	%% try to draw the vertex with colors
	figure;
	pat = patch('faces', FM, 'vertices', VM, 'FaceVertexCData', FaceVisibleM(:, 1), 'FaceColor', 'flat', 'EdgeColor', 'none');
	colormap(gray);
	view(3);
	title('Visibility map projected onto the mesh');
	cameratoolbar('show')
	cameratoolbar('SetMode', 'orbit', 'SetCoordSys', 'z');
	axis equal;

    end

	%% test for rendering an image
    zoomFactor = 1;

	%ColorImageT = RenderColorMesh(FM, VM, repmat([1 1 1], [size(FM,1), 1]), CamParamStructA{3}, [480; 640], [10; 20], zoomFactor);
	
	ColorImageT = RenderColorMesh(FM, VM, repmat(FaceColorT(:, :, 1), [1 1 3]), CamParamStructA{3}, [480; 640], [10; 20], zoomFactor);
	figure;
	imshow(ColorImageT);
    title('Rendered color image from another view using RenderColorMesh()');

    %% New: Render a depth image from the mesh.
    invertedDepth = true;
    DepthImageM = RenderDepthMesh(FM, VM, CamParamStructA{3}, [480; 640], [10; 20], zoomFactor, invertedDepth);
    figure;
    imshow(DepthImageM);
    title('Rendered depth image from another view using RenderDepthMesh()');

	if (interactive)
	    disp('Please check the variables and type continue to exit the program');
	    keyboard;
	end
