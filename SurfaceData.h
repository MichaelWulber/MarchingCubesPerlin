#include <vector>;
#define GLEW_STATIC
#include <GL/glew.h>

#ifndef SURFACEDATA_H
#define SURFACEDATA_H

class SurfaceData {
public:
	SurfaceData();
	bool isValid(int i);
	std::vector<GLfloat> getVert(int i);
	void setVert(int i, GLfloat x, GLfloat y, GLfloat z);

private:
	std::vector<std::vector<GLfloat>> verts;
	std::vector<int> is_valid;
};

#endif