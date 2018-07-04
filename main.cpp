#include <iostream>
#include <cmath>
#include <memory>
#include <map>
#include <string>
#define _USE_MATH_DEFINES

#include <Gl/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "LUTable.h"
#include "mesh.h"
#include "Shader.h"
#include "Noise.h"
#include "cimg.h"

#include "PerlinFunc.h"
#include "SphereFunc.h"
#include "UFGenerator.h"
#include "SurfaceData.h"

struct HKey {
	int a;
	int b;
	int c;
	int d;
	int e;
	int f;
};

bool operator<(HKey const & lhs, HKey const & rhs) {
	return std::tie(lhs.a, lhs.b, lhs.c, lhs.d, lhs.e, lhs.f) < std::tie(rhs.a, rhs.b, rhs.c, rhs.d, rhs.e, rhs.f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
std::vector<GLfloat> genMesh(std::shared_ptr<ImplicitFunc> function, GLfloat cubeSize);
std::vector<GLfloat> genUnion(std::shared_ptr<ImplicitFunc> funcA, std::shared_ptr<ImplicitFunc> funcB, GLfloat cubeSize);
int edgeListIndex(const bool arr[8]);
std::vector<GLfloat> findVertices(int i, int j, int k, int index, GLfloat* vertex[3], GLfloat*** vals);
GLfloat interpolate(GLfloat a, GLfloat aVal, GLfloat b, GLfloat bVal);

std::vector<GLfloat> findVerts(int i, int j, int k, int index,
	GLfloat* vertex[3], std::vector<std::vector<std::vector<GLfloat>>> &vals, std::map<HKey, float> &vert_dic);

const GLint WIDTH = 1000, HEIGHT = 1000;
int screenWidth, screenHeight;
void saveFrame();

Mesh current;
Mesh perlin;

UFGenerator ufg;

float frame_count = 0;
float dr = 2 * M_PI / 360.0;

int main() {
	glfwInit();

	// set the version of openGL to 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// use the modern stuff
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Marching Cubes", nullptr, nullptr);

	// ensures pixels coordinates are mapped to the screen correctly (accounts for pixel density)
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (nullptr == window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		return EXIT_FAILURE;
	}

	// set the current context to the window we just created
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	// use modern approach to obtain function pointers
	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, screenWidth, screenHeight);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// initialize unique file name generator
	ufg = UFGenerator("./frames/frame", "bmp");

	// initialize shader
	Shader ourShader("core.vert", "core.frag");

	// create perlin noise mesh
	float dim = 1.5;
	std::shared_ptr<ImplicitFunc> perlinFunc = (std::shared_ptr<ImplicitFunc>) (new PerlinFunc(0.5, -dim, dim, 0.0, 4));
	std::shared_ptr<ImplicitFunc> sphereFunc = (std::shared_ptr<ImplicitFunc>) (new SphereFunc(1.4));

	//std::vector<GLfloat> perlinVertices = genMesh(perlinFunc, dim);
	std::vector<GLfloat> perlinVertices = genUnion(perlinFunc, sphereFunc, dim);
	perlin = Mesh(0.4f, 0.4f, 0.4f);
	perlin.setVPositions(perlinVertices);
	perlin.genVNormals();
	perlin.genBuffer();

	// generate mesh
	current = perlin;

	// create openGL buffer and attribute objects
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	current.bindBuffer();

	// create projection transformation
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (GLfloat)(screenWidth) / (GLfloat)(screenHeight), 0.1f, 1000.0f);

	// GAME LOOP
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();

		// increment perlin offset
		//perlinFunc->incYoff(0.002);

		// generate new mesh
		/*std::vector<GLfloat> perlinVertices = genUnion(perlinFunc, sphereFunc, dim);
		current = Mesh(0.25f, 0.25f, 0.25f);
		current.setVPositions(perlinVertices);
		current.genVNormals();
		current.genBuffer();
		current.bindBuffer();*/

		// set up MVP matrix
		glm::mat4 model(1.0f);

		//model = glm::rotate(model, (GLfloat)glfwGetTime() * 0.05f, glm::vec3(0.0f, 0.3f, 0.0f));
		model = glm::rotate(model, (GLfloat) frame_count * dr , glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 view = glm::lookAt(
			glm::vec3(3, 3, 3), // Camera is at (3,3,3), in World Space
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
			);
		glm::mat4 MVP = projection * view * model;

		GLint MVPLoc = glGetUniformLocation(ourShader.Program, "MVP");
		glUniformMatrix4fv(MVPLoc, 1, GL_FALSE, glm::value_ptr(MVP));

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, current.vPositions.size() / 3);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		
		if (frame_count < 360) {
			saveFrame();
			frame_count++;
		}
		frame_count++;
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();

	return EXIT_SUCCESS;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		current = perlin;
		current.bindBuffer();
	}
}

std::vector<GLfloat> genMesh(std::shared_ptr<ImplicitFunc> function, GLfloat cubeSize) {
	//std::cout << "generating mesh..." << std::endl;
	GLfloat minX = -cubeSize;
	GLfloat minY = -cubeSize;
	GLfloat minZ = -cubeSize;
	GLfloat maxX = cubeSize;
	GLfloat maxY = cubeSize;
	GLfloat maxZ = cubeSize;
	GLfloat x, y, z, a;
	bool byteArray[8];

	const GLint dim = 50;

	bool vertices[dim][dim][dim];


	// array of values for x, y, z
	// vertexCoord[0][] = x's, vertexCoord[1][] = y's, vertex Coord[2][] = z's
	GLfloat* vertexCoord[3] = { new GLfloat[dim], new GLfloat[dim], new GLfloat[dim] };
	for (GLint i = 0; i < dim; ++i) {
		a = ((GLfloat)i / ((GLfloat)dim - 1));
		x = maxX * a + minX * (1.0f - a);
		y = maxY * a + minY * (1.0f - a);
		z = maxZ * a + minZ * (1.0f - a);
		vertexCoord[0][i] = x;
		vertexCoord[1][i] = y;
		vertexCoord[2][i] = z;
	}

	// vertices stores 0 or 1 depending on whether vertex is inside sphere or not
	// vertexVals stores the actual value from the implicit function
	GLfloat*** vertexVals = new GLfloat**[dim];
	for (GLint i = 0; i < dim; ++i) {
		vertexVals[i] = new GLfloat*[dim];

		for (GLint j = 0; j < dim; ++j) {
			vertexVals[i][j] = new GLfloat[dim];

			for (GLint k = 0; k < dim; ++k) {
				x = vertexCoord[0][i];
				y = vertexCoord[1][j];
				z = vertexCoord[2][k];
				vertices[i][j][k] = function->isInside(x, y, z);
				vertexVals[i][j][k] = function->function(x, y, z);
			}
		}
	}

	for (GLint i = 0; i < dim; ++i) {
		for (GLint j = 0; j < dim; ++j) {
			vertices[0][i][j] = false;
			vertexVals[0][i][j] = 1000000;

			vertices[dim - 1][i][j] = false;
			vertexVals[dim - 1][i][j] = 1000000;

			vertices[i][0][j] = false;
			vertexVals[i][0][j] = 1000000;

			vertices[i][dim - 1][j] = false;
			vertexVals[i][dim - 1][j] = 1000000;
			
			vertices[i][j][0] = false;
			vertexVals[i][j][0] = 1000000;

			vertices[i][j][dim - 1] = false;
			vertexVals[i][j][dim - 1] = 1000000;
		}
	}


	// Go through every cube and check vertices;
	// triangleVertices stores vertices of facets as {x0, y0, z0, x1, y1, z1, ..., xn, yn, zn}
	std::vector<GLfloat> triangleVertices;
	std::vector<GLfloat> temp;
	for (GLint i = 0; i < dim - 1; ++i) {
		for (GLint j = 0; j < dim - 1; ++j) {
			for (GLint k = 0; k < dim - 1; ++k) {
				byteArray[0] = vertices[i][j][k];
				byteArray[1] = vertices[i + 1][j][k];
				byteArray[2] = vertices[i + 1][j][k + 1];
				byteArray[3] = vertices[i][j][k + 1];
				byteArray[4] = vertices[i][j + 1][k];
				byteArray[5] = vertices[i + 1][j + 1][k];
				byteArray[6] = vertices[i + 1][j + 1][k + 1];
				byteArray[7] = vertices[i][j + 1][k + 1];
				int index = edgeListIndex(byteArray);

				temp = findVertices(i, j, k, index, vertexCoord, vertexVals);
				triangleVertices.insert(triangleVertices.end(), temp.begin(), temp.end());
			}
		}
	}
	//std::cout << "mesh complete" << std::endl;

	return triangleVertices;
}

std::vector<GLfloat> genUnion(std::shared_ptr<ImplicitFunc> funcA, std::shared_ptr<ImplicitFunc> funcB, GLfloat cubeSize) {
	//std::cout << "generating mesh..." << std::endl;
	GLfloat minX = -cubeSize;
	GLfloat minY = -cubeSize;
	GLfloat minZ = -cubeSize;
	GLfloat maxX = cubeSize;
	GLfloat maxY = cubeSize;
	GLfloat maxZ = cubeSize;
	GLfloat x, y, z, a;
	bool byteArray[8];

	const GLint dim = 100;

	bool vertices[dim][dim][dim];
	std::vector<std::vector<std::vector<GLfloat>>> vertexVals(dim, std::vector<std::vector<GLfloat>>(dim, std::vector<GLfloat>(dim, 0.0)));

	std::map<HKey, GLfloat> vert_dic;

	std::vector<GLfloat> triangleVertices;
	std::vector<GLfloat> temp;
	
	// array of values for x, y, z
	// vertexCoord[0][] = x's, vertexCoord[1][] = y's, vertex Coord[2][] = z's
	GLfloat* vertexCoord[3] = { new GLfloat[dim], new GLfloat[dim], new GLfloat[dim] };
	for (GLint i = 0; i < dim; ++i) {
		a = ((GLfloat)i / ((GLfloat)dim - 1));
		x = maxX * a + minX * (1.0f - a);
		y = maxY * a + minY * (1.0f - a);
		z = maxZ * a + minZ * (1.0f - a);
		vertexCoord[0][i] = x;
		vertexCoord[1][i] = y;
		vertexCoord[2][i] = z;
	}

	// calculate container data
	for (GLint i = 0; i < dim; ++i) {
		for (GLint j = 0; j < dim; ++j) {
			for (GLint k = 0; k < dim; ++k) {
				x = vertexCoord[0][i];
				y = vertexCoord[1][j];
				z = vertexCoord[2][k];
				vertices[i][j][k] = funcB->isInside(x, y, z);
				vertexVals[i][j][k] = funcB->function(x, y, z);
			}
		}
	}

	// determine outer surface
	for (GLint i = 0; i < dim - 1; ++i) {
		for (GLint j = 0; j < dim - 1; ++j) {
			for (GLint k = 0; k < dim - 1; ++k) {
				byteArray[0] = vertices[i][j][k];
				byteArray[1] = vertices[i + 1][j][k];
				byteArray[2] = vertices[i + 1][j][k + 1];
				byteArray[3] = vertices[i][j][k + 1];
				byteArray[4] = vertices[i][j + 1][k];
				byteArray[5] = vertices[i + 1][j + 1][k];
				byteArray[6] = vertices[i + 1][j + 1][k + 1];
				byteArray[7] = vertices[i][j + 1][k + 1];

				int index = edgeListIndex(byteArray);
				findVerts(i, j, k, index, vertexCoord, vertexVals, vert_dic);
				//triangleVertices.insert(triangleVertices.end(), temp.begin(), temp.end());
			}
		}
	}

	// calculate intersection
	for (GLint i = 0; i < dim; ++i) {
		for (GLint j = 0; j < dim; ++j) {
			for (GLint k = 0; k < dim; ++k) {
				x = vertexCoord[0][i];
				y = vertexCoord[1][j];
				z = vertexCoord[2][k];
				vertices[i][j][k] = funcA->isInside(x, y, z) && funcB->isInside(x, y, z);
				vertexVals[i][j][k] = funcA->function(x, y, z);
			}
		}
	}

	// Go through every cube and check vertices;
	// triangleVertices stores vertices of facets as {x0, y0, z0, x1, y1, z1, ..., xn, yn, zn}
	for (GLint i = 0; i < dim - 1; ++i) {
		for (GLint j = 0; j < dim - 1; ++j) {
			for (GLint k = 0; k < dim - 1; ++k) {
				byteArray[0] = vertices[i][j][k];
				byteArray[1] = vertices[i + 1][j][k];
				byteArray[2] = vertices[i + 1][j][k + 1];
				byteArray[3] = vertices[i][j][k + 1];
				byteArray[4] = vertices[i][j + 1][k];
				byteArray[5] = vertices[i + 1][j + 1][k];
				byteArray[6] = vertices[i + 1][j + 1][k + 1];
				byteArray[7] = vertices[i][j + 1][k + 1];
				int index = edgeListIndex(byteArray);

				temp = findVerts(i, j, k, index, vertexCoord, vertexVals, vert_dic);
				triangleVertices.insert(triangleVertices.end(), temp.begin(), temp.end());
			}
		}
	}

	std::cout << "mesh complete" << std::endl;

	return triangleVertices;
}


int edgeListIndex(const bool arr[8]) {
	int index = 0;
	int factor = 1;
	for (int i = 0; i < 8; ++i) {
		index += arr[i] * factor;
		factor = 2 * factor;
	}
	return index;
}

std::string genKey(int a, int b, int c, int d, int e, int f) {
	return std::to_string(a) + std::to_string(b) + std::to_string(c) + std::to_string(d) + std::to_string(e) + std::to_string(f);
}

std::vector<GLfloat> findVertices(int i, int j, int k, int index,
	GLfloat* vertex[3], GLfloat*** vals) {
	std::vector<GLfloat> triangleVertices;
	int edgeNum;
	GLfloat intersection;
	GLfloat aVal, bVal;
	GLfloat a, b;
	GLfloat x, y, z;
	std::string key;

	for (int e = 0; e < 13; ++e) {
		edgeNum = aCases[index][e];
		switch (edgeNum) {
		case -1:
			return triangleVertices;
		case 0:
			y = vertex[1][j];
			z = vertex[2][k];

			a = vertex[0][i];
			aVal = vals[i][j][k];
			b = vertex[0][i + 1];
			bVal = vals[i + 1][j][k];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(intersection);
			triangleVertices.push_back(y);
			triangleVertices.push_back(z);
			break;
		case 1:
			x = vertex[0][i + 1];
			y = vertex[1][j];

			a = vertex[2][k];
			aVal = vals[i + 1][j][k];
			b = vertex[2][k + 1];
			bVal = vals[i + 1][j][k + 1];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(x);
			triangleVertices.push_back(y);
			triangleVertices.push_back(intersection);
			break;
		case 2:
			y = vertex[1][j];
			z = vertex[2][k + 1];

			a = vertex[0][i];
			aVal = vals[i][j][k + 1];
			b = vertex[0][i + 1];
			bVal = vals[i + 1][j][k + 1];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(intersection);
			triangleVertices.push_back(y);
			triangleVertices.push_back(z);
			break;
		case 3:
			x = vertex[0][i];
			y = vertex[1][j];

			a = vertex[2][k];
			aVal = vals[i][j][k];
			b = vertex[2][k + 1];
			bVal = vals[i][j][k + 1];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(x);
			triangleVertices.push_back(y);
			triangleVertices.push_back(intersection);
			break;
		case 4:
			y = vertex[1][j + 1];
			z = vertex[2][k];

			a = vertex[0][i];
			aVal = vals[i][j + 1][k];
			b = vertex[0][i + 1];
			bVal = vals[i + 1][j + 1][k];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(intersection);
			triangleVertices.push_back(y);
			triangleVertices.push_back(z);
			break;
		case 5:
			x = vertex[0][i + 1];
			y = vertex[1][j + 1];

			a = vertex[2][k];
			aVal = vals[i + 1][j + 1][k];
			b = vertex[2][k + 1];
			bVal = vals[i + 1][j + 1][k + 1];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(x);
			triangleVertices.push_back(y);
			triangleVertices.push_back(intersection);
			break;
		case 6:
			y = vertex[1][j + 1];
			z = vertex[2][k + 1];

			a = vertex[0][i];
			aVal = vals[i][j + 1][k + 1];
			b = vertex[0][i + 1];
			bVal = vals[i + 1][j + 1][k + 1];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(intersection);
			triangleVertices.push_back(y);
			triangleVertices.push_back(z);
			break;
		case 7:
			x = vertex[0][i];
			y = vertex[1][j + 1];

			a = vertex[2][k];
			aVal = vals[i][j + 1][k];
			b = vertex[2][k + 1];
			bVal = vals[i][j + 1][k + 1];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(x);
			triangleVertices.push_back(y);
			triangleVertices.push_back(intersection);
			break;
		case 8:
			x = vertex[0][i];
			z = vertex[2][k];

			a = vertex[1][j];
			aVal = vals[i][j][k];
			b = vertex[1][j + 1];
			bVal = vals[i][j + 1][k];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(x);
			triangleVertices.push_back(intersection);
			triangleVertices.push_back(z);
			break;
		case 9:
			x = vertex[0][i + 1];
			z = vertex[2][k];

			a = vertex[1][j];
			aVal = vals[i + 1][j][k];
			b = vertex[1][j + 1];
			bVal = vals[i + 1][j + 1][k];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(x);
			triangleVertices.push_back(intersection);
			triangleVertices.push_back(z);
			break;
		case 10:
			x = vertex[0][i + 1];
			z = vertex[2][k + 1];

			a = vertex[1][j];
			aVal = vals[i + 1][j][k + 1];
			b = vertex[1][j + 1];
			bVal = vals[i + 1][j + 1][k + 1];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(x);
			triangleVertices.push_back(intersection);
			triangleVertices.push_back(z);
			break;
		case 11:
			x = vertex[0][i];
			z = vertex[2][k + 1];

			a = vertex[1][j];
			aVal = vals[i][j][k + 1];
			b = vertex[1][j + 1];
			bVal = vals[i][j + 1][k + 1];
			intersection = interpolate(a, aVal, b, bVal);

			triangleVertices.push_back(x);
			triangleVertices.push_back(intersection);
			triangleVertices.push_back(z);
			break;
		}
	}

	return triangleVertices;
}

void setHKey(struct HKey *key, int a, int b, int c, int d, int e, int f) {
	key->a = a;
	key->b = b;
	key->c = c;
	key->d = d;
	key->e = e;
	key->f = f;
}

std::vector<GLfloat> findVerts(int i, int j, int k, int index,
	GLfloat* vertex[3], std::vector<std::vector<std::vector<GLfloat>>> &vals, std::map<HKey, GLfloat> &vert_dic) {
	std::vector<GLfloat> triangleVertices;
	int edgeNum;
	GLfloat intersection;
	GLfloat aVal, bVal;
	GLfloat a, b;
	GLfloat x, y, z;
	struct HKey key;

	for (int e = 0; e < 13; ++e) {
		edgeNum = aCases[index][e];
		switch (edgeNum) {
		case -1:
			return triangleVertices;
		case 0:
			y = vertex[1][j];
			z = vertex[2][k];
			setHKey(&key, i, j, k, i + 1, j, k );

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[0][i];
				aVal = vals[i][j][k];
				b = vertex[0][i + 1];
				bVal = vals[i + 1][j][k];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(intersection);
			triangleVertices.push_back(y);
			triangleVertices.push_back(z);
			break;
		case 1:
			x = vertex[0][i + 1];
			y = vertex[1][j];
			setHKey(&key, i + 1, j, k, i + 1, j, k + 1);
			
			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[2][k];
				aVal = vals[i + 1][j][k];
				b = vertex[2][k + 1];
				bVal = vals[i + 1][j][k + 1];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(x);
			triangleVertices.push_back(y);
			triangleVertices.push_back(intersection);
			break;
		case 2:
			y = vertex[1][j];
			z = vertex[2][k + 1];
			setHKey(&key, i, j, k + 1, i + 1, j, k + 1 );

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[0][i];
				aVal = vals[i][j][k + 1];
				b = vertex[0][i + 1];
				bVal = vals[i + 1][j][k + 1];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(intersection);
			triangleVertices.push_back(y);
			triangleVertices.push_back(z);
			break;
		case 3:
			x = vertex[0][i];
			y = vertex[1][j];
			setHKey(&key, i, j, k, i, j, k + 1);

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[2][k];
				aVal = vals[i][j][k];
				b = vertex[2][k + 1];
				bVal = vals[i][j][k + 1];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(x);
			triangleVertices.push_back(y);
			triangleVertices.push_back(intersection);
			break;
		case 4:
			y = vertex[1][j + 1];
			z = vertex[2][k];
			setHKey(&key, i, j + 1, k, i + 1, j + 1, k);

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[0][i];
				aVal = vals[i][j + 1][k];
				b = vertex[0][i + 1];
				bVal = vals[i + 1][j + 1][k];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(intersection);
			triangleVertices.push_back(y);
			triangleVertices.push_back(z);
			break;
		case 5:
			x = vertex[0][i + 1];
			y = vertex[1][j + 1];
			setHKey(&key, i + 1, j + 1, k, i + 1, j + 1, k + 1);

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[2][k];
				aVal = vals[i + 1][j + 1][k];
				b = vertex[2][k + 1];
				bVal = vals[i + 1][j + 1][k + 1];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(x);
			triangleVertices.push_back(y);
			triangleVertices.push_back(intersection);
			break;
		case 6:
			y = vertex[1][j + 1];
			z = vertex[2][k + 1];
			setHKey(&key, i, j + 1, k + 1, i + 1, j + 1, k + 1);

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[0][i];
				aVal = vals[i][j + 1][k + 1];
				b = vertex[0][i + 1];
				bVal = vals[i + 1][j + 1][k + 1];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(intersection);
			triangleVertices.push_back(y);
			triangleVertices.push_back(z);
			break;
		case 7:
			x = vertex[0][i];
			y = vertex[1][j + 1];
			setHKey(&key, i, j + 1, k, i, j + 1, k + 1 );

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[2][k];
				aVal = vals[i][j + 1][k];
				b = vertex[2][k + 1];
				bVal = vals[i][j + 1][k + 1];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(x);
			triangleVertices.push_back(y);
			triangleVertices.push_back(intersection);
			break;
		case 8:
			x = vertex[0][i];
			z = vertex[2][k];
			setHKey(&key, i, j, k, i, j + 1, k );

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[1][j];
				aVal = vals[i][j][k];
				b = vertex[1][j + 1];
				bVal = vals[i][j + 1][k];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(x);
			triangleVertices.push_back(intersection);
			triangleVertices.push_back(z);
			break;
		case 9:
			x = vertex[0][i + 1];
			z = vertex[2][k];
			setHKey(&key, i + 1, j, k, i + 1, j + 1, k);

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[1][j];
				aVal = vals[i + 1][j][k];
				b = vertex[1][j + 1];
				bVal = vals[i + 1][j + 1][k];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(x);
			triangleVertices.push_back(intersection);
			triangleVertices.push_back(z);
			break;
		case 10:
			x = vertex[0][i + 1];
			z = vertex[2][k + 1];
			setHKey(&key, i + 1, j, k + 1, i + 1, j + 1, k + 1);

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[1][j];
				aVal = vals[i + 1][j][k + 1];
				b = vertex[1][j + 1];
				bVal = vals[i + 1][j + 1][k + 1];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(x);
			triangleVertices.push_back(intersection);
			triangleVertices.push_back(z);
			break;
		case 11:
			x = vertex[0][i];
			z = vertex[2][k + 1];
			setHKey(&key, i, j, k + 1, i, j + 1, k + 1);

			if (vert_dic.count(key) != 0) {
				intersection = vert_dic.at(key);
			}
			else {
				a = vertex[1][j];
				aVal = vals[i][j][k + 1];
				b = vertex[1][j + 1];
				bVal = vals[i][j + 1][k + 1];
				intersection = interpolate(a, aVal, b, bVal);
				vert_dic.insert(std::pair<HKey, GLfloat>(key, intersection));
			}

			triangleVertices.push_back(x);
			triangleVertices.push_back(intersection);
			triangleVertices.push_back(z);
			break;
		}
	}

	return triangleVertices;
}

bool isBetween(GLfloat val, GLfloat a, GLfloat b) {
	if (a > b) {
		return val >= b && val <= a;
	}
	else {
		return val >= a && val <= b;
	}
}

GLfloat interpolate(GLfloat a, GLfloat aVal, GLfloat b, GLfloat bVal) {
	GLfloat val = a + ((0 - aVal) * (b - a) / (bVal - aVal));
	if (isBetween(val, a, b)) {
		return val;
	}
	else {
		return a;
	}
}

void saveFrame() {
	char *pixel_data = new char[3 * WIDTH * HEIGHT];
	
	// obtain pixel data from frame buffer
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);

	// write pixel data to an img obj
	cimg_library::CImg<unsigned char> image(WIDTH, HEIGHT, 1, 3, 0);
	unsigned char color[3];
	int index = 0;

	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < HEIGHT; j++) {
			index = (i + j * WIDTH) * 3;
			color[0] = pixel_data[index];
			color[1] = pixel_data[index + 1];
			color[2] = pixel_data[index + 2];
			image.draw_point(i, HEIGHT - 1 - j, color);
		}
	}

	delete pixel_data;
	
	// save img to file
	image.save(ufg.getUniqueName().c_str());
}

