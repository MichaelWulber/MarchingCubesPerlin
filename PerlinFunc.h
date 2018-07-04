#include "ImplicitFunc.h"
#include "Noise.h"
#define GLEW_STATIC
#include <GL/glew.h>

#ifndef PERLINFUNC_H
#define PERLINFUNC_H


class PerlinFunc: public ImplicitFunc {
private:
	GLfloat iso;

	GLfloat x_off;
	GLfloat y_off;
	GLfloat z_off;

	Noise pn;

	GLfloat map(GLfloat val);

public:
	GLfloat amin;
	GLfloat amax;
	GLfloat bmin;
	GLfloat bmax;

	PerlinFunc(GLfloat iso, GLfloat amin, GLfloat amax, GLfloat bmin, GLfloat bmax);
	bool isInside(GLfloat x, GLfloat y, GLfloat z);
	GLfloat function(GLfloat x, GLfloat y, GLfloat z);

	void incXoff(float inc);
	void incYoff(float inc);
	void incZoff(float inc);
};

#endif