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
  %% To my knowledge, OSX only need GLEW library, you can download the source and compile GLEG from glew.sourceforge.net
  GlewPath = '-I/usr/include -I/Users/tianli/Documents/codebase/glew-1.11.0/include -L/Users/tianli/Documents/codebase/glew-1.11.0/lib -lGLEW';
  GlutPath = '-L/opt/X11/lib -I/opt/X11/include -lglut -lgl -lglu ';
  CFlags = '-DNDEBUG'
elseif strcmp(os, 'Linux')
  %% For Ubuntu Linux, just install libglew1.5 and libglew1.5-dev, freeglut
  %% is usually pre-installed
  GlewPath = '-lGLEW';
  GlutPath = '-lglut -lGL -lGLU';
  CFlags = '-DUSE_FREEGLUT';
else
  %% For Windows you could download the GLEW and GLUT binary and put them under
  %% the root of this toolbox, then use the following 
  GlewPath = '-I./glew/include -L./glew/lib -lGLEW';
  GlutPath = '-I./glut -L./glut lglut';
end

%% Compiling the source code
disp(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' MexGlutInit.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' MexGlutInit.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' BatchFaceColorGradImpl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' BatchFaceColorGradImpl.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' ProjectMesh2ImageImpl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' ProjectMesh2ImageImpl.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' RenderColorMeshImpl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' RenderColorMeshImpl.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' RenderDepthMeshImpl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' RenderDepthMeshImpl.cpp']);

disp(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' ShadowProj2Impl.cpp']);
eval(['mex ' GlewPath ' ' GlutPath ' ' CFlags ' ShadowProj2Impl.cpp']);
