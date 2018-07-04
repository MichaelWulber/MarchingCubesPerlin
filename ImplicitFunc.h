#define GLEW_STATIC
#include <Gl/glew.h>

#ifndef IMPLICITFUNC_H
#define IMPLICITFUNC_H

class ImplicitFunc {
public:
	virtual GLfloat function(GLfloat x, GLfloat y, GLfloat z) = 0;
	virtual bool isInside(GLfloat x, GLfloat y, GLfloat z) = 0;
	virtual void incXoff(float inc) = 0;
	virtual void incYoff(float inc) = 0;
	virtual void incZoff(float inc) = 0;
};

#endif
