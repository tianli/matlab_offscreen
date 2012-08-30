/**
 */

#include <mex.h>
#include <GL/glut.h>

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[]) {
	int argc = 1;
	char* argv = "test";
	
    static bool glutInitialized = false;
    
    // glut initialization.
    if (!glutInitialized) {
        glutInit(&argc, &argv);
        glutInitialized = true;
    }
}
