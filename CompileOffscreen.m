%%
% This is a matlab compilation script that will compile the Offscreen
% library
% Author: Tianli Yu, tianli@ieee.org, Mar 30, 2009

%%
% Please Modify these two path strings to point to the GLUT and GLEW library
% -I is the header file include path, -L is the binary lib path

[status, os] = system('uname');

% removes \n
os = os(1:end-1);

if strcmp(os, 'Darwin')
  %% Mac OS X
  GlewPath = '-I/opt/local/include -L/opt/local/lib -lGLEW';
  GlutPath = '-I/opt/local/include -L/opt/local/lib -lglut';
elseif strcmp(os, 'Linux')
  %% For Ubuntu Linux, just install libglew1.5 and libglew1.5-dev, freeglut
  %% is usually pre-installed
  GlewPath = '-lGLEW';
  GlutPath = '-lglut -lGL -lGLU';
else

  %% For Windows you could download the GLEW and GLUT binary and put them under
  %% the root of this toolbox, then use the following 
  GlewPath = '-I./glew/include -L./glew/lib -lGLEW';
  GlutPath = '-I./glut -L./glut lglut';
end

%% Compiling the source code
disp(['mex ' GlewPath ' ' GlutPath ' MexGlutInit.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' MexGlutInit.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' BatchFaceColorGradImpl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' BatchFaceColorGradImpl.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' ProjectMesh2ImageImpl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' ProjectMesh2ImageImpl.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' RenderColorMeshImpl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' RenderColorMeshImpl.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' RenderDepthMeshImpl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' RenderDepthMeshImpl.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' ShadowProj2Impl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' ShadowProj2Impl.cpp']);
