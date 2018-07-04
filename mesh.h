#ifndef MESH_H
#define MESH_H

#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>

class Mesh {
public:
	std::vector<GLfloat> vPositions;
	std::vector<GLfloat> faceColor;
	std::vector<GLfloat> vNormals;

	Mesh();
	Mesh(GLfloat R, GLfloat G, GLfloat B);
	Mesh(const Mesh &mesh);

	void setVPositions(std::vector<GLfloat> vPos);
	std::vector<GLfloat> calculateVNormals(GLfloat Ax, GLfloat Ay, GLfloat Az, GLfloat Bx, GLfloat By, GLfloat Bz, GLfloat Cx, GLfloat Cy, GLfloat Cz);
	void addTriangle(std::vector<GLfloat> vPos);
	void genBuffer();
	void bindBuffer();
	void genVNormals();
	std::vector<GLfloat> getVBuffer();
	void reset();

private:
	std::vector<GLfloat> vBuffer;
};

#endif
