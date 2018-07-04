#include "ImplicitFunc.h"

#ifndef SPHEREFUNC_H_
#define SPHEREFUNC_H_

class SphereFunc : public ImplicitFunc {
private:
	GLfloat r;
public:
	SphereFunc(GLfloat r);
	bool isInside(GLfloat x, GLfloat y, GLfloat z);
	GLfloat function(GLfloat x, GLfloat y, GLfloat z);
	
	void incXoff(float inc);
	void incYoff(float inc);
	void incZoff(float inc);
};

#endif /* SPHEREFUNC_H_ */
