imagebase = 'buddha12/';

CameraV = [1:24]';

imagenum = length(CameraV);

TestImageA = cell(imagenum, 1);
ImageMaskA = cell(imagenum, 1);
CamParamStructA = cell(imagenum, 1);

for ii = 1 : imagenum
	indexname = int2str(1200+CameraV(ii));
	
	TestImageA{ii} = imread([imagebase,'buddha', indexname, '.png']);
	
	ImageMaskA{ii} = imread([imagebase,'mask_', indexname, '.png']);

end

[height, width] = size(TestImageA{1});

CamA = CameraConvert2('cam5.inc', height, width);

for ii = 1 : imagenum
	CamParamStructA{ii} = CamA{CameraV(ii)};
end
