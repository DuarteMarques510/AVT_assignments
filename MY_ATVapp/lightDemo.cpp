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
GLint direc_loc;
	
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
public: float position[3] = { 0.0f, 0.0f, 0.0f };
public: float angle = 0.0f;
public: float angularSpeed = 0.0f;
};

//boat
boat myBoat;

float speedDecay = 0.02f;
float angleDecay = 1.0f;
float deltaT = 0.1f;
float direction = 1.0f;
float forceMultiplier = 1.0f;

//cameras

camera cams[3];
int activeCamera = 0;

bool canChangeDirection = true; //when true the boat can change direction, starts at true because the boat starts at speed 0
bool dayTime = true;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];
float DirectlightPos[4] = {1.0f, 100.0f, 1.0f, 0.0f}; //directonal light



float pointLights[6][4] = { {0.0f, 4.0f, 15.0f, 1.0f}, {-20.0f, 4.0f, -20.0f, 1.0f}, {7.0f, 4.0f, -8.0f, 1.0f}, {1.0f, 4.0f, 8.0f, 1.0f}, {-3.0f, 4.0f, 11.0f, 1.0f}, {5.0f, 4.0f, 7.0f, 1.0f} };
float aux = 0;
float paddleangle[2] = {0.0f, 0.0f};
bool paddlemoving[2] = {false, false};
bool boatmovement = false;

void updateBoat(int direction) {

	for (uint16_t i = 0; i < 2; i++) {
		if (paddlemoving[i] == true) {
			paddleangle[i] += 6.0f;
		}
	}
		

	if (boatmovement == true) { 
		myBoat.angle += (myBoat.angularSpeed * deltaT) * direction;
		float radians = myBoat.angle * 3.14f / 180.0f;
		myBoat.direction[0] = cos(radians);
		myBoat.direction[2] = sin(radians);
		myBoat.position[0] -= (myBoat.speed * myBoat.direction[0] * deltaT) * direction; //subtração porque assim a combinação do cosseno e seno dá a direção correta
		myBoat.position[1] += (myBoat.speed * myBoat.direction[1] * deltaT) * direction; 
		myBoat.position[2] += (myBoat.speed * myBoat.direction[2] * deltaT) * direction;
		myBoat.speed -= speedDecay;
		if (myBoat.speed < 0.0f) {
			myBoat.speed = 0.0f;
			canChangeDirection = true;
			boatmovement = false;

		}
		else {
			canChangeDirection = false;
		}
		myBoat.angularSpeed -= angleDecay;
		if (myBoat.angularSpeed < 0.001f || myBoat.angularSpeed>-0.001f) {
			myBoat.angularSpeed = 0.0f;
		}
		for (uint16_t i = 0; i < 2; i++) {
			if (paddleangle[i] != 360.0f && paddleangle[i] != 0.0f && paddleangle[i] > 300) {
				paddleangle[i] += 6.0f;
			}
		}
		
		//std::cout << "o paddleangle e: " << paddleangle << "\n";
	}
	glutPostRedisplay();
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
	if(h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	perspective(53.13f, ratio, 0.1f, 1000.0f);
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
				if (paddleangle[0] == 120.0f || paddleangle[1]==120.0f){
					boatmovement = true;
				}
				else if (paddleangle[i] >= 360.0f) {
					paddlemoving[i] = false;
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
			if (i == 1) {
				translate(MODEL, -8.0f, -0.4f, -8.0f);
			}
			else if (i == 2) {
				translate(MODEL, -8.0f, -0.4f, 8.0f);
			}
			else if (i == 3) {
				translate(MODEL, 8.0f, -0.4f, -8.0f);
			}
			else if (i == 4) {
				translate(MODEL, 8.0f, -0.4f, 8.0f);
			}
			else if (i == 5) {
				translate(MODEL, 0.0f, -0.4f, -8.0f);
			}
			else {
				translate(MODEL, 0.0f, -0.4f, 8.0f);
			}
			rotate(MODEL, 90.0f, 1.0f, 0.0f, 0.0f);
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
		cams[activeCamera].camPos[1] = myBoat.position[1] + 5.0f;
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
		ortho(ratio * (-20), ratio * 20, -20, 20, 0.1f, 20); //left, right, bottom, top
	}

	// use our shader
	
	glUseProgram(shader.getProgramIndex());

		//send the light position in eye coordinates
		//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

	float res[4];
	
	//multMatrixPoint(VIEW, lightPos, res);   //lightPos definido em World Coord so is converted to eye space
	//glUniform4fv(lPos_uniformId, 1, res);
	if (dayTime) {
		multMatrixPoint(VIEW, DirectlightPos, res);
		glUniform4fv(direc_loc, 1, res);
	}
	multMatrixPoint(VIEW, pointLights[0], res);
	glUniform4fv(point_loc, 1, pointLights[0]);
	multMatrixPoint(VIEW, pointLights[1], res);
	glUniform4fv(point_loc1, 1, pointLights[1]);
	multMatrixPoint(VIEW, pointLights[2], res);
	glUniform4fv(point_loc2, 1, pointLights[2]);
	multMatrixPoint(VIEW, pointLights[3], res);
	glUniform4fv(point_loc3, 1, pointLights[3]);
	multMatrixPoint(VIEW, pointLights[4], res);
	glUniform4fv(point_loc4, 1, pointLights[4]);
	multMatrixPoint(VIEW, pointLights[5], res);

		

	renderPlain();
	renderBoat();
	renderRedCylinders();
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

	case 'c':
		printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
		break;
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
		myBoat.speed += (0.6f * forceMultiplier);
		myBoat.angularSpeed -= (45.0f * forceMultiplier);
		angleDecay = -0.5f;
		
		break;
	case 'd':
		//mexer cubo segundo um angulo e velocidade para a direita
		paddlemoving[0] = true;
		myBoat.speed += (0.6f * forceMultiplier);
		myBoat.angularSpeed += (45.0f * forceMultiplier);
		angleDecay = 0.5f;
		break;
	case 's':
		//toggle inverter direção frente/trás
		if (canChangeDirection) {
			direction = direction * (-1.0f);
			canChangeDirection = false;
		}
		break;
	case 'o':
		//duplica a força atual das remadas
		forceMultiplier = forceMultiplier * 2.0f;
		break;
	case 'n':
		//ligar ou desligar a luz direcional (dayTime)
		dayTime = !dayTime;
		std::cout << "dayTime: " << dayTime << "\n";
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
	if (state == GLUT_DOWN)  {
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

	deltaX =  - xx + startX;
	deltaY =    yy - startY;

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
		cams[0].camPos[0] =  rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		cams[0].camPos[2] =  rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		cams[0].camPos[1] =  rAux * sin(betaAux * 3.14f / 180.0f);
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
	glBindFragDataLocation(shader.getProgramIndex(), 0,"colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	//glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
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
	direc_loc = glGetUniformLocation(shader.getProgramIndex(), "dirLight.direction");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap0");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	
	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

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
	/*cams[0].camPos[1] = 10.0f;
	cams[0].camTarget[0] = myBoat.position[0];
	cams[0].camTarget[1] = myBoat.position[1];
	cams[0].camTarget[2] = myBoat.position[2];*/
	cams[1].camPos[0] = 0.01f;
	cams[1].camPos[1] = 18.0f;
	cams[1].camPos[2] = 0.0f;
	cams[1].type = 1; //orthogonal camera
	cams[2].camPos[0] = 0.01f;
	cams[2].camPos[1] = 30.0f;
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
	cams[activeCamera].camPos[1] = r *   						     sin(beta * 3.14f / 180.0f);

	


	
	float amb[]= {0.2f, 0.15f, 0.1f, 1.0f};
	float diff[] = {0.8f, 0.6f, 0.4f, 1.0f};
	float spec[] = {0.8f, 0.8f, 0.8f, 1.0f};
	float emissive[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float shininess= 100.0f;
	int texcount = 0;

	float amb2[] = { 0.0f, 0.0f, 0.3f, 1.0f }; //cor ambiente do plano
	float diff2[] = { 0.1f, 0.1f, 0.8f, 1.0f }; //cor difusa do plano  
	float spec2[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	amesh = createQuad(40.0f, 40.0f); // create the plane for the scene
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
	float spec1[] = {0.9f, 0.9f, 0.9f, 1.0f};
	shininess=500.0;

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


int main(int argc, char **argv) {

//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_MULTISAMPLE);

	glutInitContextVersion (4, 3);
	glutInitContextProfile (GLUT_CORE_PROFILE );
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100,100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever
	//glutTimerFunc(0, updateBoat, direction);

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc ( mouseWheel ) ;
	

//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf ("Vendor: %s\n", glGetString (GL_VENDOR));
	printf ("Renderer: %s\n", glGetString (GL_RENDERER));
	printf ("Version: %s\n", glGetString (GL_VERSION));
	printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}



