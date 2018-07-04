#include "SurfaceData.h"

SurfaceData::SurfaceData() : verts(12, std::vector<GLfloat>(3, 0.0)), is_valid(12, 0) {}

bool SurfaceData::isValid(int i) {
	return is_valid[i] == 1;
}

std::vector<GLfloat> SurfaceData::getVert(int i) {
	return verts[i];
}

void SurfaceData::setVert(int i, GLfloat x, GLfloat y, GLfloat z) {
	verts[i][0] = x;
	verts[i][1] = y;
	verts[i][2] = z;
	is_valid[i] = 1;
}