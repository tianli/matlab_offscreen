/*******************************************************************************
 * OffscreenGL.h: class to wrap the opengl rendering to texture using framebuffer
 * object
 *
 * Copyright (c) 2009  Tianli Yu (yu_tianli@hotmail.com)
 *
 *******************************************************************************/

#ifndef OFFSCREEN_GL_H
#define OFFSCREEN_GL_H

#include "GL/glew.h"
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"
#include <stdio.h>

class OffscreenGL
{
public:
    OffscreenGL(int maxHeight, int maxWidth);
    ~OffscreenGL();
    
    bool RGB8Setup();
    
private:
    static int glutWin;
    GLuint fb;
    GLuint renderTex;
    GLuint depthTex;
    int maxHeight, maxWidth;
    
};

int OffscreenGL::glutWin = -1;

OffscreenGL::OffscreenGL(int maxHeight, int maxWidth) {
	if (glutWin < 0) {
        glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
        glutInitWindowPosition(100,100);
        glutInitWindowSize(maxWidth, maxHeight);
        glutWin = glutCreateWindow("Offscreen rendering test");
	} else {
        glutSetWindow(glutWin);
	}
        
	// We also hide the window so that you don't see it.
	glutHideWindow();
    glewInit();
    
    // create FBO (off-screen framebuffer)
    glGenFramebuffersEXT(1, &fb);
    
    this->maxHeight = maxHeight;
    this->maxWidth = maxWidth;
}

OffscreenGL::~OffscreenGL()
{
  // detach the texture & FBO
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
      GL_TEXTURE_RECTANGLE_ARB, 0, 0);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDeleteFramebuffersEXT(1, &fb);
    
  glDeleteTextures(1, &renderTex);
  glDeleteTextures(1, &depthTex);
    
	// We do not destroy the window as freeGLUT in Ubuntu never really destroy
	// the window, so we will keep reusing the window to work around the problem.
#ifndef USE_FREEGLUT
  glutDestroyWindow(glutWin);
  glutWin = -1;
#endif
}

bool checkFramebufferStatus();
void AllocateTexRect(GLenum texUnit, GLuint texName, GLint interpMethod,
        GLint internalFormat, GLsizei width, GLsizei height);
void AllocateDepthTexRect(GLenum texUnit, GLuint texName, GLint interpMethod,
        GLint internalFormat, GLsizei width, GLsizei height);


// setup an offscreen opengl context with RGB 8bit-color and a specific max size
// using FBO
bool OffscreenGL::RGB8Setup()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
        
    glGenTextures(1, &renderTex);
    AllocateTexRect(GL_TEXTURE0, renderTex, GL_NEAREST, GL_RGB, maxWidth, maxHeight);
    
    glGenTextures(1, &depthTex);
    AllocateDepthTexRect(GL_TEXTURE1, depthTex, GL_NEAREST, GL_DEPTH_COMPONENT,
            maxWidth, maxHeight);
    
    glGenFramebuffersEXT(1, &fb);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    
    //Attach 2D texture to this FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, renderTex, 0);
    //-------------------------
    //Attach depth texture to FBO
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_RECTANGLE_ARB, depthTex, 0);
    //-------------------------
    
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT | GL_DEPTH_ATTACHMENT_EXT);
    
    checkFramebufferStatus();
    
    //glEnable(GL_DEPTH_TEST);
    return true;
}

void AllocateTexRect(GLenum texUnit, GLuint texName, GLint interpMethod,
        GLint internalFormat, GLsizei width, GLsizei height)
{
    glActiveTexture(texUnit);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texName);
    
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, interpMethod);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, interpMethod);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, internalFormat, width, height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}


void AllocateDepthTexRect(GLenum texUnit, GLuint texName, GLint interpMethod,
        GLint internalFormat, GLsizei width, GLsizei height)
{
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texName);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, interpMethod);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, interpMethod);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    
    //NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
}


/**
 * Checks framebuffer status.
 * Copied directly out of the spec, modified to deliver a return value.
 */
bool checkFramebufferStatus() {
    GLenum status;
    status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            return true;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, incomplete attachment\n");
            return false;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            printf("Unsupported framebuffer format\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            printf("Framebuffer incomplete, missing attachment\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            printf("Framebuffer incomplete, attached images must have same dimensions\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            printf("Framebuffer incomplete, attached images must have same format\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            printf("Framebuffer incomplete, missing draw buffer\n");
            return false;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            printf("Framebuffer incomplete, missing read buffer\n");
            return false;
    }
    return false;
}

#endif // OFFSCREEN_GL
