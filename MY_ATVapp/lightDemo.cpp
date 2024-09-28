//
// AVT: Phong Shading and Text rendered with FreeType library
// The text rendering was based on https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
// 
// Author: João Madeiras Pereira
//

#include <math.h>
#include <iostream>
#include <sstream>
#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>


// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"

#include "avtFreeType.h"

using namespace std;

#define CAPTION "AVT Demo: Phong Shading and Text rendered with FreeType"
int WindowHandle = 0;
int WinX = 1024, WinY = 768;

unsigned int FrameCount = 0;

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

//Vector with meshes
vector<struct MyMesh> myMeshes;
vector<struct MyMesh> boatMeshes;
vector<struct MyMesh> floats;
vector<struct MyMesh> floatCylinders;
vector<struct MyMesh> islands;
vector<struct MyMesh> islandTrees;
vector<struct MyMesh> islandLeaves;
vector<struct MyMesh> islandHouseBodies;
vector<struct MyMesh> islandHouseRoofs;
vector<struct MyMesh> auxMeshes;

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId;
GLint tex_loc, tex_loc1, tex_loc2;
GLint point_loc, point_loc1, point_loc2, point_loc3, point_loc4, point_loc5;
GLint spot_loc, spot_loc1;
GLint spot_angle_loc, spot_angle_loc1;
GLint spot_dir_loc, spot_dir_loc1;
GLint direc_loc;
GLint directOnOff_loc, pointOnOff_loc, spotOnOff_loc;

// Camera Position
//float cams[activeCamera].camPos[0], cams[activeCamera].camPos[1], cams[activeCamera].camPos[2];

//Camera Class
class camera {
public:	float camPos[3] = { 0.0f, 0.0f, 0.0f };
public: float camTarget[3] = { 0.0f, 0.0f, 0.0f };
public: int type = 0; //0 - perspective, 1 - ortho
};

class boat {
public: float speed = 0.0f;
public: float direction[3] = { 1.0f, 0.0f, 0.0f }; //vetor diretor da direção do barco
public: float position[3] = { 25.0f, 0.0f, 0.0f };
public: float angle = 0.0f;
public: float angularSpeed = 0.0f;
	  //public: float center[3] = { 0.0f, 0.5f, 0.0f }; //como o barco não roda sobre o seu centro é dificil uma esfera envolvente que o envolva completamente, o mais facil é criar uma com raio igual à diagonal do barco ara cobrir o barco e a sua rotação
	  //public: float radius = 0.75f;
public: float center[3] = { 25.0f, 0.5f, 0.0f };
public: float radius = std::sqrt(2);
};

//boat
boat myBoat;

float speedDecay = 0.02f;
float angleDecay = 1.0f;
float deltaT = 0.1f;
float direction = 1.0f;
float forceMultiplier = 1.0f;

float floatPositions[6][3] = {
	{11.0f, 0.0f, 2.0f},
	{-30.0f, 0.0f, -20.0f},
	{5.0f, 0.0f, 13.0f},
	{-18.0f, 0.0f, -10.0f},
	{-24.0f, 0.0f, 18.0f},
	{20.0f, 0.0f, -16.0f}

};

float floatRadius = 0.5f;

float waterCreaturesPositions[6][3] = {
	{10.0f, 0.0f, 10.0f},
	{-14.0f, 0.0f, -12.0f},
	{16.0f, 0.0f, 12.0f},
	{-6.0f, 0.0f, 36.0f},
	{-12.0f, 0.0f, 10.0f},
	{10.0f, 0.0f, -22.0f}

};

float waterCreaturesRotationAngles[6];

float waterCreaturesAngles[6];

float waterCreaturesSpeeds[6];

float waterCreaturesRadius = 0.875f; //metade da altura estipulada para os cilindros que estão a ser usados para representar as piranhas

float islandPositions[4][3] = {
	{27.0f, -17.0f, -33.0f},
	{-34.0f, -17.0f, 30.0f},
	{20.0f, -17.0f, 20.0f},
	{-10.0f, -17.0f, -25.0f}
};

float islandRadius = 5.0f;

//cameras

camera cams[3];
int activeCamera = 0;

bool canChangeDirection = true; //when true the boat can change direction, starts at true because the boat starts at speed 0
bool dayTime = false;
bool pointLightsOn = false;
bool spotLightsOn = false;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];
float DirectlightPos[4] = { 1.0f, 100.0f, 1.0f, 0.0f }; //directonal light



float pointLights[6][4] = { {floatPositions[0][0], 4.0f, floatPositions[0][2], 1.0f}, {floatPositions[1][0], 4.0f, floatPositions[1][2], 1.0f}, {floatPositions[2][0], 4.0f, floatPositions[2][2], 1.0f}, {floatPositions[3][0], 4.0f, floatPositions[3][2], 1.0f}, {floatPositions[4][0], 4.0f, floatPositions[4][2], 1.0f}, {floatPositions[5][0], 4.0f, floatPositions[5][2], 1.0f} };
float aux = 0;
float paddleangle[2] = { 0.0f, 0.0f };
bool paddlemoving[2] = { false, false };
int paddlesMoving = 0;

//void createBoundingSphere() {
//	myBoat.center[0] = myBoat.position[0] - (0.5f * myBoat.direction[0]);
//	myBoat.center[1] = myBoat.position[1] + (0.5f); //não é preciso direção porque a altura do barco é constante
//	myBoat.center[2] = myBoat.position[2] + (0.5f * myBoat.direction[2]);
//
//	//myBoat.radius = std::sqrt(3 * std::pow(0.5f, 2));
//}

float distanceFromBoatToObject(float objectPosition[3], float nextBoatCenter[3]) {
	float distance = std::sqrt(std::pow((objectPosition[0] - nextBoatCenter[0]), 2)
		+ std::pow((objectPosition[1] - nextBoatCenter[1]), 2)
		+ std::pow((objectPosition[2] - nextBoatCenter[2]), 2));

	return distance;
}

int checkCollision(float nextBoatCenter[3]) {
	for (uint16_t i = 0; i < 6; i++) {
		if (distanceFromBoatToObject(floatPositions[i], nextBoatCenter) < myBoat.radius + floatRadius) {
			return 1; //code for collision with float
		}
		if (distanceFromBoatToObject(waterCreaturesPositions[i], nextBoatCenter) < myBoat.radius + waterCreaturesRadius) {
			return -1; //code for collision with water creature, code for failure
		}
	}
	for (uint16_t i = 0; i < 4; i++) {
		if (distanceFromBoatToObject(islandPositions[i], nextBoatCenter) < myBoat.radius + islandRadius) {
			return 2; //code for collision with island
		}
	}
	return 0; //no collision
}


void updateBoat(int direction) {

	for (uint16_t i = 0; i < 2; i++) {
		if (paddlemoving[i] == true) {
			paddleangle[i] += 6.0f;
		}
	}
	if (paddlemoving[1]) {
		myBoat.speed = (0.6f * forceMultiplier);
		myBoat.angularSpeed -= (20.0f * forceMultiplier);
	}
	if (paddlemoving[0]) {
		myBoat.speed = (0.6f * forceMultiplier);
		myBoat.angularSpeed += (20.0f * forceMultiplier);
	}

	if (!paddlemoving[1] || !paddlemoving[0]) { // se um dos dois remos não mexer atualizar a direção
		myBoat.angle += (myBoat.angularSpeed * deltaT) * direction;
		float radians = myBoat.angle * 3.14f / 180.0f;
		myBoat.direction[0] = cos(radians);
		myBoat.direction[2] = sin(radians);
	}
	float nextBoatCenter[3] = { myBoat.center[0] - (myBoat.speed * myBoat.direction[0] * deltaT) * direction,
		myBoat.center[1],
		myBoat.center[2] + (myBoat.speed * myBoat.direction[2] * deltaT) * direction };

	int collision = checkCollision(nextBoatCenter);
	if (collision == 0) { //if no colision move the boat and its center
		myBoat.position[0] -= (myBoat.speed * myBoat.direction[0] * deltaT) * direction; //subtração porque assim a combinação do cosseno e seno dá a direção correta
		myBoat.center[0] -= (myBoat.speed * myBoat.direction[0] * deltaT) * direction;
		//myBoat.position[1] += (myBoat.speed * myBoat.direction[1] * deltaT) * direction;
		//myBoat.center[1] += (myBoat.speed * myBoat.direction[1] * deltaT) * direction;
		myBoat.position[2] += (myBoat.speed * myBoat.direction[2] * deltaT) * direction;
		myBoat.center[2] += (myBoat.speed * myBoat.direction[2] * deltaT) * direction;
	}
	else if (collision == -1) { //colision with piranha, failure, reset boat
		myBoat.speed = 0.0f;
		myBoat.angularSpeed = 0.0f;
		myBoat.position[0] = 25.0f;
		myBoat.position[1] = 0.0f;
		myBoat.position[2] = 0.0f;
		myBoat.angle = 0.0f;
		myBoat.direction[0] = 1.0f;
		myBoat.direction[1] = 0.0f;
		myBoat.direction[2] = 0.0f;
		myBoat.center[0] = 25.0f;
		myBoat.center[1] = 0.5f;
		myBoat.center[2] = 0.0f;
		//exit(0);
	}//if the colision code is not 0 or -1, its a collsion that makes the position update not happen

	myBoat.speed -= speedDecay;
	if (myBoat.speed < 0.0f) {
		myBoat.speed = 0.0f;
		canChangeDirection = true;
	}
	else {
		canChangeDirection = false;
	}
	if (myBoat.angularSpeed < 0.001f || myBoat.angularSpeed>-0.001f) {
		myBoat.angularSpeed = 0.0f;
	}
	for (uint16_t i = 0; i < 2; i++) {
		if (paddleangle[i] != 360.0f && paddleangle[i] != 0.0f) {
			paddleangle[i] += 3.0f;
		}
	}

	//createBoundingSphere(); //atualizar a bounding sphere do barco

	glutPostRedisplay();
}

void accelerateCreatures(int value) {
	for (uint16_t i = 0; i < 6; i++) {
		waterCreaturesSpeeds[i] = waterCreaturesSpeeds[i] * 2;
	}
	glutPostRedisplay();
	glutTimerFunc(30000, accelerateCreatures, 0);
}

void changeCreaturesAngle(int value) {
	for (uint16_t i = 0; i < 6; i++) {
		float angle = float((rand() % 181) + (waterCreaturesAngles[i] - 90));
		waterCreaturesAngles[i] = angle;
	}
	glutPostRedisplay();
	glutTimerFunc(5000, changeCreaturesAngle, 0);
}

void animateWaterCreatures(int value) {
	for (uint16_t i = 0; i < 6; i++) {
		waterCreaturesRotationAngles[i] += 2.0f;
		if (waterCreaturesRotationAngles[i] >= 360.0f) {
			waterCreaturesRotationAngles[i] = 0.0f;
		}
		float radians = waterCreaturesAngles[i] * 3.14f / 180.0f;
		if (waterCreaturesPositions[i][0] + (waterCreaturesSpeeds[i] * cos(radians) * deltaT) > 50 || waterCreaturesPositions[i][2] + (waterCreaturesSpeeds[i] * cos(radians) * deltaT) > 50 || waterCreaturesPositions[i][0] + (waterCreaturesSpeeds[i] * cos(radians) * deltaT) < -50 || waterCreaturesPositions[i][2] + (waterCreaturesSpeeds[i] * cos(radians) * deltaT) < -50) {
			waterCreaturesPositions[i][0] = (rand() % 101) - 50;
			waterCreaturesPositions[i][2] = (rand() % 101) - 50;
		}
		waterCreaturesPositions[i][0] += (waterCreaturesSpeeds[i] * cos(radians) * deltaT);
		waterCreaturesPositions[i][2] += (waterCreaturesSpeeds[i] * sin(radians) * deltaT);
	}

	glutPostRedisplay();
	glutTimerFunc(1000 / 60, animateWaterCreatures, 0);
}

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	updateBoat(direction);
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	if (cams[activeCamera].type == 0) {
		//perspective camera
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
	else {
		//orthogonal camera
		ortho(ratio * (-55), ratio * 55, -55, 55, 0.1f, 55); //left, right, bottom, top
	}
}


// ------------------------------------------------------------
//
// Render stufff
//

void renderBoat(void) {
	//render the boat
	GLint loc;
	pushMatrix(MODEL);
	{
		//translate the boat to the position
		translate(MODEL, myBoat.position[0], myBoat.position[1], myBoat.position[2]);
		rotate(MODEL, myBoat.angle, 0.0f, 1.0f, 0.0f);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, boatMeshes[0].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, boatMeshes[0].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, boatMeshes[0].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, boatMeshes[0].mat.shininess);

		//compute and send the matrices to the shader
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		//render the parent mesh
		glBindVertexArray(boatMeshes[0].vao);
		glDrawElements(boatMeshes[0].type, boatMeshes[0].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//render the pawn mesh attached to the cube
		pushMatrix(MODEL);
		{
			translate(MODEL, 0.5f, 1.0f, 0.5f); //translate to the top of the parent mesh
			scale(MODEL, 0.3f, 0.3f, 0.3f); //scale the pawn
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, boatMeshes[1].mat.ambient);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, boatMeshes[1].mat.diffuse);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, boatMeshes[1].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, boatMeshes[1].mat.shininess);

			//compute and send the matrices to the shader
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			//render the child mesh
			glBindVertexArray(boatMeshes[1].vao);
			glDrawElements(boatMeshes[1].type, boatMeshes[1].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		popMatrix(MODEL);

		//render the paddles sticks
		for (uint16_t i = 0; i < 2; i++) {
			pushMatrix(MODEL);
			{
				if (i == 0) { // paddle da direita em relação à posição inicial da camara
					translate(MODEL, 0.5f, 0.35f, -0.25f);
					rotate(MODEL, 60.0f, 1.0f, 0.0f, 0.0f); // com estas duas rotações a pá fica na posição correta e com a rotação correta
					rotate(MODEL, 90.0f, 0.0f, 1.0f, 0.0f);
					rotate(MODEL, paddleangle[i], 0.075f * direction, 0.2f * direction, 0.0f);
				}
				else { // paddle da esquerda em relação à posição inicial da camara
					translate(MODEL, 0.5f, 0.35f, 1.25f);
					rotate(MODEL, 300.0f, 1.0f, 0.0f, 0.0f);
					rotate(MODEL, 270.0f, 0.0f, 1.0f, 0.0f);
					rotate(MODEL, paddleangle[i], -0.075f * direction, -0.2f * direction, 0.0f);
				}
				if (paddleangle[i] >= 360.0f) {
					//paddlemoving[i] = false;
					paddleangle[i] = 0.0f;
				}

				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
				glUniform4fv(loc, 1, boatMeshes[2 + i].mat.ambient);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, boatMeshes[2 + i].mat.diffuse);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
				glUniform4fv(loc, 1, boatMeshes[2 + i].mat.specular);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
				glUniform1f(loc, boatMeshes[2 + i].mat.shininess);

				//compute and send the matrices to the shader
				computeDerivedMatrix(PROJ_VIEW_MODEL);
				glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
				glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
				computeNormalMatrix3x3();
				glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

				//render the child mesh
				glBindVertexArray(boatMeshes[2 + i].vao);
				glDrawElements(boatMeshes[2 + i].type, boatMeshes[2 + i].numIndexes, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				//render the paddle ends
				pushMatrix(MODEL);
				{
					scale(MODEL, 1.0f, 1.0f, 0.25f);
					translate(MODEL, -0.5, -1.65f, -0.5f);// aqui não é preciso fazer translações diferentes para cada paddle end porque a matriz MODEL já está no sistema de coordenadas do paddle stick
					loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
					glUniform4fv(loc, 1, boatMeshes[4 + i].mat.ambient);
					loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
					glUniform4fv(loc, 1, boatMeshes[4 + i].mat.diffuse);
					loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
					glUniform4fv(loc, 1, boatMeshes[4 + i].mat.specular);
					loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
					glUniform1f(loc, boatMeshes[4 + i].mat.shininess);

					//compute and send the matrices to the shader
					computeDerivedMatrix(PROJ_VIEW_MODEL);
					glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
					glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
					computeNormalMatrix3x3();
					glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

					//render the child mesh
					glBindVertexArray(boatMeshes[4 + i].vao);
					glDrawElements(boatMeshes[4 + i].type, boatMeshes[4 + i].numIndexes, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
				}
				popMatrix(MODEL);
			}
			popMatrix(MODEL);
		}
	}
	popMatrix(MODEL);
}
void renderPlain(void) {
	GLint loc;
	pushMatrix(MODEL);
	{
		translate(MODEL, 0.0f, 0.0f, 0.0f);
		rotate(MODEL, 270.0f, 1.0f, 0.0f, 0.0f);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[0].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[0].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[0].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[0].mat.shininess);

		//compute and send the matrices to the shader
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		//render the parent mesh
		glBindVertexArray(myMeshes[0].vao);
		glDrawElements(myMeshes[0].type, myMeshes[0].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	popMatrix(MODEL);
}
void renderRedCylinders(void) {
	GLint loc;
	for (uint16_t i = 1; i < 7; i++) {
		pushMatrix(MODEL);
		{
			translate(MODEL, waterCreaturesPositions[i - 1][0], waterCreaturesPositions[i - 1][1] - 0.4f, waterCreaturesPositions[i - 1][2]);
			rotate(MODEL, 90.0f, 1.0f, 0.0f, 0.0f);
			rotate(MODEL, waterCreaturesRotationAngles[i - 1], 0.0f, 0.0f, 1.0f);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, myMeshes[i].mat.ambient);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, myMeshes[i].mat.diffuse);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, myMeshes[i].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, myMeshes[i].mat.shininess);

			//compute and send the matrices to the shader
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			//render the parent mesh
			glBindVertexArray(myMeshes[i].vao);
			glDrawElements(myMeshes[i].type, myMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		popMatrix(MODEL);
	}
}

void renderFloats() {
	GLint loc;
	for (uint16_t i = 0; i < 6; i++) {
		pushMatrix(MODEL);
		{
			translate(MODEL, floatPositions[i][0], floatPositions[i][1], floatPositions[i][2]);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, floats[i].mat.ambient);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, floats[i].mat.diffuse);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, floats[i].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, floats[i].mat.shininess);

			//compute and send the matrices to the shader
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			//render the parent mesh
			glBindVertexArray(floats[i].vao);
			glDrawElements(floats[i].type, floats[i].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			pushMatrix(MODEL);
			{
				translate(MODEL, 0.0f, 1.0f, 0.0f);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
				glUniform4fv(loc, 1, floatCylinders[i].mat.ambient);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, floatCylinders[i].mat.diffuse);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
				glUniform4fv(loc, 1, floatCylinders[i].mat.specular);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
				glUniform1f(loc, floatCylinders[i].mat.shininess);

				//compute and send the matrices to the shader
				computeDerivedMatrix(PROJ_VIEW_MODEL);
				glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
				glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
				computeNormalMatrix3x3();
				glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

				//render the parent mesh
				glBindVertexArray(floatCylinders[i].vao);
				glDrawElements(floatCylinders[i].type, floatCylinders[i].numIndexes, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
			}
			popMatrix(MODEL);
		}
		popMatrix(MODEL);
	}

}

void renderIslandsAndTrees() {
	GLint loc;
	for (uint16_t i = 0; i < islands.size(); i++) {
		pushMatrix(MODEL);
		{
			translate(MODEL, islandPositions[i][0], islandPositions[i][1], islandPositions[i][2]);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
			glUniform4fv(loc, 1, islands[i].mat.ambient);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, islands[i].mat.diffuse);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, islands[i].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, islands[i].mat.shininess);

			//compute and send the matrices to the shader
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			//render the parent mesh
			glBindVertexArray(islands[i].vao);
			glDrawElements(islands[i].type, islands[i].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			pushMatrix(MODEL);
			{
				translate(MODEL, 0.0f, 20.0f, 0.0f);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
				glUniform4fv(loc, 1, islandTrees[i].mat.ambient);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, islandTrees[i].mat.diffuse);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
				glUniform4fv(loc, 1, islandTrees[i].mat.specular);
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
				glUniform1f(loc, islandTrees[i].mat.shininess);

				//compute and send the matrices to the shader
				computeDerivedMatrix(PROJ_VIEW_MODEL);
				glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
				glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
				computeNormalMatrix3x3();
				glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

				//render the parent mesh
				glBindVertexArray(islandTrees[i].vao);
				glDrawElements(islandTrees[i].type, islandTrees[i].numIndexes, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);

				pushMatrix(MODEL);
				{
					translate(MODEL, 0.0f, 1.0f, 0.0f);
					loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
					glUniform4fv(loc, 1, islandLeaves[i].mat.ambient);
					loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
					glUniform4fv(loc, 1, islandLeaves[i].mat.diffuse);
					loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
					glUniform4fv(loc, 1, islandLeaves[i].mat.specular);
					loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
					glUniform1f(loc, islandLeaves[i].mat.shininess);

					//compute and send the matrices to the shader
					computeDerivedMatrix(PROJ_VIEW_MODEL);
					glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
					glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
					computeNormalMatrix3x3();
					glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

					//render the parent mesh
					glBindVertexArray(islandLeaves[i].vao);
					glDrawElements(islandLeaves[i].type, islandLeaves[i].numIndexes, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
				}
				popMatrix(MODEL);
				// Render two houses on each island
				for (uint16_t j = 0; j < 2; j++) {
					pushMatrix(MODEL);
					{
						scale(MODEL, 2.5, 2.5, 2.5);
						// Position the house bodies
						if (j == 0) {
							translate(MODEL, 1.5f, -0.8f, 1.5f);  // Position for the first house
						}
						else {
							translate(MODEL, -2.5f, -0.8f, -2.5f); // Position for the second house
						}

						// Render house body
						loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
						glUniform4fv(loc, 1, islandHouseBodies[i].mat.ambient);
						loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
						glUniform4fv(loc, 1, islandHouseBodies[i].mat.diffuse);
						loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
						glUniform4fv(loc, 1, islandHouseBodies[i].mat.specular);
						loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
						glUniform1f(loc, islandHouseBodies[i].mat.shininess);

						computeDerivedMatrix(PROJ_VIEW_MODEL);
						glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
						glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
						computeNormalMatrix3x3();
						glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

						glBindVertexArray(islandHouseBodies[i].vao);
						glDrawElements(islandHouseBodies[i].type, islandHouseBodies[i].numIndexes, GL_UNSIGNED_INT, 0);
						glBindVertexArray(0);

						// Render house roof
						translate(MODEL, 0.5f, 1.0f, 0.5f); // Move up to place the roof on top
						loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
						glUniform4fv(loc, 1, islandHouseRoofs[i].mat.ambient);
						loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
						glUniform4fv(loc, 1, islandHouseRoofs[i].mat.diffuse);
						loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
						glUniform4fv(loc, 1, islandHouseRoofs[i].mat.specular);
						loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
						glUniform1f(loc, islandHouseRoofs[i].mat.shininess);

						computeDerivedMatrix(PROJ_VIEW_MODEL);
						glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
						glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
						computeNormalMatrix3x3();
						glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

						glBindVertexArray(islandHouseRoofs[i].vao);
						glDrawElements(islandHouseRoofs[i].type, islandHouseRoofs[i].numIndexes, GL_UNSIGNED_INT, 0);
						glBindVertexArray(0);
					}
					popMatrix(MODEL);
				}

			}
			popMatrix(MODEL);
		}
		popMatrix(MODEL);

	}

}

void renderBoundingSphere() {
	GLint loc;
	pushMatrix(MODEL);
	{
		translate(MODEL, myBoat.center[0], myBoat.center[1], myBoat.center[2]);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, auxMeshes[0].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, auxMeshes[0].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, auxMeshes[0].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, auxMeshes[0].mat.shininess);

		//compute and send the matrices to the shader
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		//render the parent mesh
		glBindVertexArray(auxMeshes[0].vao);
		glDrawElements(auxMeshes[0].type, auxMeshes[0].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	popMatrix(MODEL);
}

void renderScene(void) {

	GLint loc;
	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	if (activeCamera == 0) {
		cams[activeCamera].camPos[0] = myBoat.position[0] + 10.0f;
		cams[activeCamera].camPos[2] = myBoat.position[2];
		cams[activeCamera].camPos[1] = myBoat.position[1] + 3.0f;
		cams[activeCamera].camTarget[0] = myBoat.position[0];
		cams[activeCamera].camTarget[1] = myBoat.position[1];
		cams[activeCamera].camTarget[2] = myBoat.position[2];

	}
	// set the camera using a function similar to gluLookAt
	lookAt(cams[activeCamera].camPos[0], cams[activeCamera].camPos[1], cams[activeCamera].camPos[2], cams[activeCamera].camTarget[0], cams[activeCamera].camTarget[1], cams[activeCamera].camTarget[2], 0, 1, 0);

	float ratio = (float)(m_viewport[2] - m_viewport[0]) / (float)(m_viewport[3] - m_viewport[1]);
	loadIdentity(PROJECTION);
	if (cams[activeCamera].type == 0) {
		//perspective camera
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
	else {
		//orthogonal camera
		ortho(ratio * (-55), ratio * 55, -55, 55, 0.1f, 55); //left, right, bottom, top
	}

	// use our shader

	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates
	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

	float res[4];

	//multMatrixPoint(VIEW, lightPos, res);   //lightPos definido em World Coord so is converted to eye space
	//glUniform4fv(lPos_uniformId, 1, res);

	glUniform1i(directOnOff_loc, dayTime); //send boolean variables to shader
	glUniform1i(pointOnOff_loc, pointLightsOn);
	glUniform1i(spotOnOff_loc, spotLightsOn);

	if (dayTime) {
		multMatrixPoint(VIEW, DirectlightPos, res);
		glUniform4fv(direc_loc, 1, res);
	}
	if (pointLightsOn) {
		multMatrixPoint(VIEW, pointLights[0], res);
		glUniform4fv(point_loc, 1, res);
		multMatrixPoint(VIEW, pointLights[1], res);
		glUniform4fv(point_loc1, 1, res);
		multMatrixPoint(VIEW, pointLights[2], res);
		glUniform4fv(point_loc2, 1, res);
		multMatrixPoint(VIEW, pointLights[3], res);
		glUniform4fv(point_loc3, 1, res);
		multMatrixPoint(VIEW, pointLights[4], res);
		glUniform4fv(point_loc4, 1, res);
		multMatrixPoint(VIEW, pointLights[5], res);
		glUniform4fv(point_loc5, 1, res);
	}




	renderPlain();
	renderBoat();
	renderRedCylinders();
	renderFloats();
	renderIslandsAndTrees();
	//renderBoundingSphere();
	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	//the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	//RenderText(shaderText, "This is a sample text", 25.0f, 25.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	//RenderText(shaderText, "AVT Light and Text Rendering Demo", 440.0f, 570.0f, 0.5f, 0.3, 0.7f, 0.9f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch (key) {

	case 27:
		glutLeaveMainLoop();
		break;

		/*case 'c':
			printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
			break;*/
	case 'm': glEnable(GL_MULTISAMPLE); break;
	case '1':
		activeCamera = 0;
		break;
	case '2':
		activeCamera = 1;
		break;
	case '3':
		activeCamera = 2;
		break;
	case 'a':
		//mexer cubo segundo um angulo e velocidade para a esquerda
		paddlemoving[1] = true;
		break;
	case 'd':
		//mexer cubo segundo um angulo e velocidade para a direita
		paddlemoving[0] = true;
		break;
	case 's':
		//toggle inverter direção frente/trás
		if (canChangeDirection) {
			direction = direction * (-1.0f);
		}
		break;
	case 'o':
		//duplica a força atual das remadas
		forceMultiplier = forceMultiplier * 2.0f;
		break;
	case 'n':
		//ligar ou desligar a luz direcional (dayTime)
		dayTime = !dayTime;
		//std::cout << "dayTime: " << dayTime << "\n";
		break;
	case 'c':
		//toggle on/off the point lights
		pointLightsOn = !pointLightsOn;
		std::cout << "pointLightsOn: " << pointLightsOn << "\n";
		break;
	}
}

void processKeysUp(unsigned char key, int xx, int yy)
{
	switch (key) {
	case 'a':
		paddlemoving[1] = false;
		break;
	case 'd':
		paddlemoving[0] = false;
		break;
	}
}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}
	if (activeCamera == 0) {
		cams[0].camPos[0] = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		cams[0].camPos[2] = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		cams[0].camPos[1] = rAux * sin(betaAux * 3.14f / 180.0f);
	}


	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}



void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;
	if (activeCamera == 0) {
		cams[activeCamera].camPos[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
		cams[activeCamera].camPos[2] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
		cams[activeCamera].camPos[1] = r * sin(beta * 3.14f / 180.0f);
	}


	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight_phong.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight_phong.frag");
	//shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight_Gouraud.vert");
	//shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight_Gouraud.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	//glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
		exit(1);
	}

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	point_loc = glGetUniformLocation(shader.getProgramIndex(), "pointLights[0].position");
	point_loc1 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[1].position");
	point_loc2 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[2].position");
	point_loc3 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[3].position");
	point_loc4 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[4].position");
	point_loc5 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[5].position");
	spot_loc = glGetUniformLocation(shader.getProgramIndex(), "spotLight[0].position");
	spot_loc1 = glGetUniformLocation(shader.getProgramIndex(), "spotLight[1].position");
	spot_angle_loc = glGetUniformLocation(shader.getProgramIndex(), "spotLight[0].angle");
	spot_angle_loc1 = glGetUniformLocation(shader.getProgramIndex(), "spotLight[1].angle");
	spot_dir_loc = glGetUniformLocation(shader.getProgramIndex(), "spotLight[0].direction");
	spot_dir_loc1 = glGetUniformLocation(shader.getProgramIndex(), "spotLight[1].direction");
	direc_loc = glGetUniformLocation(shader.getProgramIndex(), "dirLight.direction");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap0");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	directOnOff_loc = glGetUniformLocation(shader.getProgramIndex(), "dayTime");
	pointOnOff_loc = glGetUniformLocation(shader.getProgramIndex(), "pointLightsOn");
	spotOnOff_loc = glGetUniformLocation(shader.getProgramIndex(), "spotLightsOn");


	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shaderText.isProgramValid()) {
		printf("GLSL Text Program Not Valid!\n");
		exit(1);
	}

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//


void init()
{
	MyMesh amesh;
	cams[1].camPos[0] = 0.01f;
	cams[1].camPos[1] = 50.0f;
	cams[1].camPos[2] = 0.0f;
	cams[1].type = 1; //orthogonal camera
	cams[2].camPos[0] = 0.01f;
	cams[2].camPos[1] = 100.0f;
	cams[2].camPos[2] = 0.0f;

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	// set the camera position based on its spherical coordinates
	cams[activeCamera].camPos[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	cams[activeCamera].camPos[2] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	cams[activeCamera].camPos[1] = r * sin(beta * 3.14f / 180.0f);


	for (uint16_t i = 0; i < 6; i++) {
		float random = 0.3 + ((float)rand() / RAND_MAX) * (0.7 - 0.5);
		waterCreaturesSpeeds[i] = random / 4;
		random = 1 + ((float)rand() / RAND_MAX) * (360 - 1);
		waterCreaturesRotationAngles[i] = random;
		for (uint16_t j = 0; j < 2; j++) {
			if (j == 1) { continue; }
			random = (rand() % 101) - 50;
			waterCreaturesPositions[i][j] = float(random);
		}
	}


	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	float shininess2 = 128.0f;
	int texcount = 0;

	float amb2[] = { 0.0f, 0.0f, 0.3f, 1.0f }; //cor ambiente do plano
	float diff2[] = { 0.1f, 0.1f, 0.8f, 1.0f }; //cor difusa do plano  
	float spec2[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	amesh = createQuad(100.0f, 100.0f); // create the plane for the scene
	memcpy(amesh.mat.ambient, amb2, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff2, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec2, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);


	// create geometry and VAO of the pawn
	/*amesh = createPawn();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);*/


	// create geometry and VAO of the sphere
	/*amesh = createSphere(1.0f, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);*/

	float amb1[] = { 0.3f, 0.0f, 0.0f, 1.0f }; //cor ambiente do cilindro
	float diff1[] = { 0.8f, 0.1f, 0.1f, 1.0f }; //cor difusa do cilindro ou cor do material 
	float spec1[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	float amb3[] = { 0.0f, 0.3f, 0.0, 1.0f }; //green
	float diff3[] = { 0.1f, 0.8f, 0.1f, 1.0f };
	float spec3[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	float amb4[] = { 0.55f, 0.45f, 0.3f, 1.0f };  // Sand
	float diff4[] = { 0.65f, 0.55f, 0.35f, 1.0f };
	float spec4[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	float amb5[] = { 0.5f, 0.5f, 0.5f, 1.0f };  // house body
	float diff5[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float spec5[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	float amb6[] = { 0.5f, 0.0f, 0.0f, 1.0f };  // house roof
	float diff6[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float spec6[] = { 1.0f, 0.0f, 0.0f, 1.0f };

	shininess = 500.0;

	//create geometry and VAO of the cube
	amesh = createCube();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	boatMeshes.push_back(amesh);

	// create geometry and VAO of the pawn on top of the boat
	amesh = createPawn();
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	boatMeshes.push_back(amesh);


	//create paddles sticks as cylinders
	for (uint16_t i = 0; i < 2; i++) {
		amesh = createCylinder(1.75f, 0.2f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		boatMeshes.push_back(amesh);
	}

	//create the paddle ends as cubes
	for (uint16_t i = 0; i < 2; i++) {
		amesh = createCube();
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		boatMeshes.push_back(amesh);
	}


	// create geometry and VAO of 6 cylinders that will be in the water
	for (uint16_t i = 0; i < 6; i++) {
		amesh = createCylinder(1.5f, 0.5f, 30);
		memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);
	}
	//create geometry for floating spheres with poles as poles
	for (uint16_t i = 0; i < 6; i++) {
		amesh = createSphere(0.5f, 20);
		memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		floats.push_back(amesh);

		amesh = createCylinder(2.0f, 0.25f, 20);
		memcpy(amesh.mat.ambient, amb1, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff1, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec1, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		floatCylinders.push_back(amesh);
	}

	//create the islands with trees
	for (uint16_t i = 0; i < 4; i++) {
		amesh = createSphere(20.0f, 20);
		memcpy(amesh.mat.ambient, amb4, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff4, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec4, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		islands.push_back(amesh);

		amesh = createCylinder(2.0f, 0.5f, 20);
		memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		islandTrees.push_back(amesh);

		amesh = createCone(2.0f, 1.0f, 20);
		memcpy(amesh.mat.ambient, amb3, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff3, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec3, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		islandLeaves.push_back(amesh);

	}

	// House body (cube)
	for (uint16_t i = 0; i < 4; i++) {
		amesh = createCube();
		memcpy(amesh.mat.ambient, amb5, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff5, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec5, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess2;
		amesh.mat.texCount = texcount;
		islandHouseBodies.push_back(amesh);

		// House roof (octagon)
		amesh = createCone(1.0f, 1.5f, 8);
		memcpy(amesh.mat.ambient, amb6, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, diff6, 4 * sizeof(float));
		memcpy(amesh.mat.specular, spec6, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess2;
		amesh.mat.texCount = texcount;
		islandHouseRoofs.push_back(amesh);
	}


	//create the aux sphere to show the bounding sphere, comment to not create it
	amesh = createSphere(myBoat.radius, 20);
	memcpy(amesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	auxMeshes.push_back(amesh);


	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char** argv) {


	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever
	//glutTimerFunc(0, updateBoat, direction);
	glutTimerFunc(0, accelerateCreatures, 0);
	glutTimerFunc(0, changeCreaturesAngle, 0);
	glutTimerFunc(0, animateWaterCreatures, 0);

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutKeyboardUpFunc(processKeysUp);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);


	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}
