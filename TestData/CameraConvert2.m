function CamParamStructA = CameraConvert2(CameraFileName, height, width)

	fid = fopen(CameraFileName, 'rt');

	%% read the angle array 
	radius = 0;
		
    while 1
    	tline = fgetl(fid);
		if ~ischar(tline), break, end
		%% check if the beginning char is Light
		if length(tline) == 0
			continue;
		end
		pos = findstr(tline, '#declare rad');
		if ( length(pos) >= 1)
			%% find the radius
			posstart = findstr(tline, '=');
			posend = findstr(tline, ';');
			
			radius = sscanf(tline((posstart+1):(posend-1)), '%f', 1);
			break
		end				
	end
	
    while 1
    	tline = fgetl(fid);
		if ~ischar(tline), break, end
		%% check if the beginning char is Light
		if length(tline) == 0
			continue;
		end
		pos = findstr(tline, '#declare angleA');
		if ( length(pos) >= 1)
			%% find the angle array
			posstart = findstr(tline, 'array[') + 6;
			posend = findstr(tline, ']');
			camnum = sscanf(tline((posstart):(posend(1)-1)), '%f', 1)
			AngleM = zeros(camnum, 2);
			
			for ii = 1 : camnum
				tline = fgetl(fid);
				posstart = findstr(tline, '{');
				posend = findstr(tline, '}');
				AngleM(ii, :) = (sscanf(tline((posstart+1):(posend(1)-1)), '%f,', 2))';
			end
			break
		end				
	end
	
    while 1
    	tline = fgetl(fid);
		if ~ischar(tline), break, end
		%% check if the beginning char is Light
		if length(tline) == 0
			continue;
		end
		pos = findstr(tline, 'look_at');
		if (length(pos) >= 1)
			posstart = findstr(tline, '<');
			posend = findstr(tline, '>');			
				
			%% is this a column vector?
			TargetV = sscanf(tline((posstart+1):(posend-1)), '%f,', 3);
		end
	end
	
	AngleM = AngleM / 180 * pi;
    
    fclose(fid);	

    CameraPosM = radius * [sin(AngleM(:,1)) .* sin(AngleM(:,2)), cos(AngleM(:,1)), sin(AngleM(:,1)) .* cos(AngleM(:,2))];
            
    %% convert to light/object center coordinate
    CameraPosM = [CameraPosM(:,1), -CameraPosM(:,3), CameraPosM(:,2)];
    TargetV = [TargetV(1); -TargetV(3); TargetV(2)]; 
    
        
    OutVectorM = ones(camnum, 1) * TargetV' - CameraPosM;
    
    CamParamStructA = cell(camnum, 1);
    for ii = 1 : camnum
    	OutV = OutVectorM(ii, :)';
    	OutV = OutV / norm(OutV);
    	
    	if (OutV == [0;0;-1])
    		UpV = [0;-1;0];
    		RightV = [1;0;0];
    	elseif (OutV == [0;0;1])
    		UpV = [0;1;0];
    		RightV = [-1;0;0];
    	else
	    	UpV = [0;0;-1];
	    	UpV = UpV - (UpV' * OutV) * OutV;
	    	UpV = UpV / norm(UpV);
	    	
	    	RightV = cross(UpV, OutV);
	    	RightV = RightV / norm(RightV);

	    end
	    	    	
    	CamParamStructA{ii}.RcM = [RightV,UpV,OutV]';
    	
    	CamParamStructA{ii}.TcV = - CamParamStructA{ii}.RcM * CameraPosM(ii,:)';
    	
    	%%% some modification here, remove the hard coded numbers!!!
    	CamParamStructA{ii}.fcV = (9 / 2.5 * (height -1)/2) * [1;1];
    	
    	CamParamStructA{ii}.ccV = [(width - 1)/2; (height -1)/2];
    
    end