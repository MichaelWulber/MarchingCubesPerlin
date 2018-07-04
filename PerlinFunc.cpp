#include <iostream>
#include "PerlinFunc.h"

PerlinFunc::PerlinFunc(GLfloat iso, GLfloat amin, GLfloat amax, GLfloat bmin, GLfloat bmax) {
	this->iso = iso;
	this->amin = amin;
	this->amax = amax;
	this->bmin = bmin;
	this->bmax = bmax;
	this->x_off = 0.0;
	this->y_off = 0.0;
	this->z_off = 0.0;
	this->pn = Noise();
}

bool PerlinFunc::isInside(GLfloat x, GLfloat y, GLfloat z) {
	return pn.noise(map(x) + x_off, map(y) + y_off, map(z) + z_off) - iso < 0;
}

GLfloat PerlinFunc::function(GLfloat x, GLfloat y, GLfloat z) {
	return pn.noise(map(x) + x_off, map(y) + y_off, map(z) + z_off) - iso;
}

GLfloat PerlinFunc::map(GLfloat val) {
	return bmin + (bmax - bmin) * (val - amin) / (amax - amin);
}

void PerlinFunc::incXoff(float inc) {
	this->x_off += inc;
}

void PerlinFunc::incYoff(float inc) {
	//this->y_off += inc;
	if (iso + inc < 1.0) {
		this->iso += inc;
	}
}

void PerlinFunc::incZoff(float inc) {
	this->z_off += inc;
}