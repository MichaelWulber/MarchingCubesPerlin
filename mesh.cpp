#include "Mesh.h"
#include <cmath>
#include <iostream>

Mesh::Mesh() : vPositions(0, 0.f), faceColor(3, 0.f), vBuffer(0, 0.f), vNormals(3, 0.f) {}


Mesh::Mesh(GLfloat R, GLfloat G, GLfloat B) : vPositions(0, 0.f), faceColor(3, 0.f), vBuffer(0, 0.f), vNormals(3, 0.f) {
	this->faceColor[0] = R;
	this->faceColor[1] = G;
	this->faceColor[2] = B;
}

Mesh::Mesh(const Mesh &mesh) {
	this->vPositions = mesh.vPositions;
	this->faceColor = mesh.faceColor;
	this->vBuffer = mesh.vBuffer;
	this->vNormals = mesh.vNormals;
}

void Mesh::setVPositions(std::vector<GLfloat> vPos) {
	this->vPositions = vPos;
}

std::vector<GLfloat> Mesh::calculateVNormals(GLfloat Ax, GLfloat Ay, GLfloat Az, GLfloat Bx, GLfloat By, GLfloat Bz, GLfloat Cx, GLfloat Cy, GLfloat Cz) {

	std::vector<GLfloat> normals(3, 0.f);

	// Vector U: B-A
	GLfloat Ux = Bx - Ax;
	GLfloat Uy = By - Ay;
	GLfloat Uz = Bz - Az;

	// Vector V: C-A
	GLfloat Vx = Cx - Ax;
	GLfloat Vy = Cy - Ay;
	GLfloat Vz = Cz - Az;

	// Calculate Normals
	GLfloat normalX = (Uy*Vz) - (Uz*Vy);
	GLfloat normalY = (Uz*Vx) - (Ux*Vz);
	GLfloat normalZ = (Ux*Vy) - (Uy*Vx);

	GLfloat mag = sqrt(normalX*normalX + normalY*normalY + normalZ*normalZ);

	normals[0] = normalX / mag;
	normals[1] = normalY / mag;
	normals[2] = normalZ / mag;

	return normals;
}

void Mesh::genVNormals() {
	std::vector<GLfloat> normal(3, 0.f);
	std::vector<GLfloat> vNormals(vPositions.size(), 0.f);

	for (int i = 0; i < vPositions.size(); i += 9) {
		normal = calculateVNormals(vPositions[i],
			vPositions[i + 1],
			vPositions[i + 2],
			vPositions[i + 3],
			vPositions[i + 4],
			vPositions[i + 5],
			vPositions[i + 6],
			vPositions[i + 7],
			vPositions[i + 8]);

		vNormals[i] = normal[0];
		vNormals[i + 1] = normal[1];
		vNormals[i + 2] = normal[2];

		vNormals[i + 3] = normal[0];
		vNormals[i + 4] = normal[1];
		vNormals[i + 5] = normal[2];

		vNormals[i + 6] = normal[0];
		vNormals[i + 7] = normal[1];
		vNormals[i + 8] = normal[2];
	}
	this->vNormals = vNormals;
}

void Mesh::addTriangle(std::vector<GLfloat> vPos) {
	vPositions.insert(vPositions.end(), vPos.begin(), vPos.end());
}

void Mesh::genBuffer() {
	std::vector<GLfloat> buffer(vPositions.size() * 3, 0.0f);

	int count = 0;
	for (int i = 0; i < vPositions.size() * 3; i += 9) {
		// vertex position
		buffer[i] = vPositions[count];
		buffer[i + 1] = vPositions[count + 1];
		buffer[i + 2] = vPositions[count + 2];

		// vertex rgb
		buffer[i + 3] = this->faceColor[0];
		buffer[i + 4] = this->faceColor[1];
		buffer[i + 5] = this->faceColor[2];

		// Add facial normal attribute
		buffer[i + 6] = this->vNormals[count];
		buffer[i + 7] = this->vNormals[count + 1];
		buffer[i + 8] = this->vNormals[count + 2];
		count += 3;
	}
	this->vBuffer = buffer;
}

void Mesh::bindBuffer() {
	if (vPositions.size() % 3 != 0) {
		std::cout << "position error" << std::endl;
	}

	if (vNormals.size() != vPositions.size()) {
		std::cout << "normal error" << std::endl;
	}

	glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), &vBuffer[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

std::vector<GLfloat> Mesh::getVBuffer() {
	return this->vBuffer;
}

void Mesh::reset() {

}
