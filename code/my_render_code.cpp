
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <cstdio>
#include <cassert>
#include <iostream>
#include "GL_framework.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

#define NUM_ROWS 70
#define NUM_COLS 70

#define OFFSET 1.1

struct matrixBeh {

	int speed;
	int initPos;
	int num_elements;


} matrix[NUM_COLS];


int rotAxisMatrix[NUM_COLS][NUM_ROWS];

float zPos[NUM_ROWS][NUM_COLS] = { 0 };


#define MAX_CUBES 4
#define EXERCISE_NUM 10
glm::vec4 randomPositions[MAX_CUBES] = {glm::vec4(0,0,0,1),glm::vec4(2,2,0,1), glm::vec4(4,0,0,1), glm::vec4(2,-2,0,1) };
int rotAxis[MAX_CUBES];
int ex = 0;
bool update[EXERCISE_NUM];

int w, h;
int aux = 50;
float angle = 0.f;
float paramSlide = 0.f;
glm::mat4 rot;
glm::vec3 rotVal = glm::vec3(0, 0, 0);

namespace MyFirstShader {

	void myInitCode(void);
	GLuint myShaderCompile(void);

	void myCleanupCode(void);
	void myRenderCode(double currentTime, glm::vec4 pos);
	void myRenderCode(double currentTime, glm::vec4 pos, int axis);

	GLuint myRenderProgram;
	GLuint myVAO;
}

namespace MyOctahedronShader {

	void myInitCode(void);
	GLuint myShaderCompile(void);

	void myCleanupCode(void);
	void myRenderCode(double currentTime, glm::vec4 pos);
	void myRenderCode(double currentTime, glm::vec4 pos, int axis);
	void myRenderCodeMatrix(double currentTime, glm::vec4 pos, int axis, int speed, bool first); 
	void myRenderCode(double currentTime, glm::vec4 pos, int axis, bool checkAlign);

	GLuint myRenderProgram;
	GLuint myVAO;
}


namespace MyOctahedronShaderWireframe {

	void myInitCode(void);
	GLuint myShaderCompile(void);

	void myCleanupCode(void);
	void myRenderCode(double currentTime, glm::vec4 pos);
	void myRenderCode(double currentTime, glm::vec4 pos, int axis);
	void myRenderCodeMatrix(double currentTime, glm::vec4 pos, int axis, int speed);

	GLuint myRenderProgram;
	GLuint myVAO;
}



namespace ImGui {
	void Render();
}



//PENSAR SI SE INCLUYEN TODAS O NO
namespace RenderVars {
	const float FOV = glm::radians(65.f);
	const float zNear = 1.f;
	const float zFar = 1000.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _targetViewPoint;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -5.f, -15.f };
	float rota[2] = { 0.f, 0.f };
}

namespace RV = RenderVars;

void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	if (height != 0) RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(RV::FOV, 0.f, RV::zNear, RV::zFar);
}

void GLmousecb(MouseEvent ev) {
	if (RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch (ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	}
	else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}
void myInitCode(int width, int height);
void myGUI() {
	bool show = true;
	ImGui::Begin("Simulation Parameters", &show, 0);

	// Do your GUI code here....
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		ImGui::RadioButton("Exercise 1", &ex, 0);
		ImGui::RadioButton("Exercise 2", &ex, 1);
		ImGui::RadioButton("Exercise 3", &ex, 2);
		ImGui::RadioButton("Exercise 4", &ex, 3);
		ImGui::RadioButton("Exercise 5", &ex, 4);
		ImGui::RadioButton("Exercise 6.1", &ex, 5);
		ImGui::RadioButton("Exercise 6.2", &ex, 6);
		ImGui::Text("   Exercise 6.3");
		ImGui::Text("   Exercise 6.4");

	}
	// .........................

	ImGui::End();

}


glm::vec4 GetRandomPoint() {
	return glm::vec4(rand()%20, rand() % 20, rand() % 20, 1.0);
}

void myInitCode(int width, int height) {

	w = width;
	h = height;

	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	/*for (int i = 0; i < MAX_CUBES; i++) {
		randomPositions[i] = GetRandomPoint();
	}*/

	for (int i = 0; i < EXERCISE_NUM; i++) {

		update[i] = true;
	}


	for (int i = 0; i < NUM_ROWS / 2; i++) {
		for (int j = 0; j < NUM_COLS / 2; j++) {
			zPos[i][j] = 2 * sqrt(2)*(rand() % 4);
		}
	}

	//int aux = 200;
	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar); 
	rot = glm::mat4(1);

	MyFirstShader::myInitCode();
	MyOctahedronShader::myInitCode();
	MyOctahedronShaderWireframe::myInitCode();
}


void myRenderCode(double currentTime) {

	//std::cout<<cos(currentTime) << std::endl;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RV::_modelView = glm::mat4(1.f);

	glm::mat4 auxMatrix = glm::mat4(1);



	//SWITCH STATEMENT: to check exercises
	switch (ex) {

	//EXERCISE 1
	/*Generate a geometry shader that, given a set of random points in a 3D volume, converts each point into a cube, 
	with the segments defining the cube aligned with the axis. Make sure the random points are stored as seeds, and 
	that each seed is placed at the center of the cube. The combination of the side and the cube is called a 
	body-centered cube.*/

	case 0:
		//Reset of exercises
		if (update[0]) {

			RV::rota[0] = 0.f;
			RV::rota[1] = 0.f;

			RV::panv[0] = 0.f;
			RV::panv[1] = 0.f;
			RV::panv[2] = -20.f;

			RV::_projection = glm::perspective(RV::FOV, (float)w / (float)h, RV::zNear, RV::zFar);


			for (int i = 0; i < MAX_CUBES; i++) {
				randomPositions[i] = GetRandomPoint();
			}

			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[0] = false;
		}
		for (int i = 0; i < MAX_CUBES; i++) {
			MyFirstShader::myRenderCode(currentTime, randomPositions[i]);
		}
		break;


	//EXERCISE 2
	/*Arrange the seeds in a body centered cubic lattice.
	Make a shader that, for each cube generated, generate a truncated octocahedron, as defined by the voronoi cell 
	of the body-centered cube. A Voronoi cell is the 3D equivalent of a Voronoi space, as 
	defined here: http://mathworld.wolfram.com/VoronoiDiagram.html*/
	case 1:
		//Reset of exercises 
		if (update[1]){
				
			RV::rota[0] = -45.f;
			RV::rota[1] = 45.f;

			RV::panv[0] = 0.f;
			RV::panv[1] = 0.f;
			RV::panv[2] = -15.f;

			RV::_projection = glm::perspective(RV::FOV, (float)w / (float)h, RV::zNear, RV::zFar);

			randomPositions[0] = glm::vec4(-2*sqrt(2), 0, 0, 1);
			randomPositions[1] = glm::vec4(2*sqrt(2), 0, 0, 1);
			randomPositions[2] = glm::vec4(0, 2*sqrt(2), -2*sqrt(2), 1);
			randomPositions[3] = glm::vec4(0, 2*sqrt(2), 2*sqrt(2), 1);
			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[1] = false;
		}
		for (int i = 0; i < MAX_CUBES; i++) {
			MyOctahedronShader::myRenderCode(currentTime, randomPositions[i]);				
		}
		break;


	//EXERCISE 3
	/*Pick the result of task 1, and organize your points in such a way that  the each cube rotates in a different 
	direction as it falls. Visualize it with an orthonormal perspective.*/
	case 2: 
		//Reset of exercises
		if (update[2]) {

			RV::rota[0] = -45.f;
			RV::rota[1] = 0.f;

			RV::panv[0] = 0.f;
			RV::panv[1] = 0.f;
			RV::panv[2] = -15.f;

			RV::_projection = glm::ortho((float)(-w / aux), (float)(w / aux), (float)(-h / aux), (float)(h / aux), 0.01f, 100.f);

			for (int i = 0; i < MAX_CUBES; i++) {
				randomPositions[i] = GetRandomPoint();
				rotAxis[i] = rand() % 3;
			}
			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[2] = false;
		}
		for (int i = 0; i < MAX_CUBES; i++) {
			MyFirstShader::myRenderCode(currentTime, randomPositions[i], rotAxis[i]);
		}
		
		break;


	//EXERCISE 4
	/*Reproduce the effect of task 3, but replacing the cubes with the truncated octocahedrons*/
	case 3:
		//Reset of exercises
		if (update[3]) {

			RV::rota[0] = -45.f;
			RV::rota[1] = 45.f;

			RV::panv[0] = 0.f;
			RV::panv[1] = 0.f;
			RV::panv[2] = -25.f;

			RV::_projection = glm::perspective(RV::FOV, (float)w / (float)h, RV::zNear, RV::zFar);

			for (int i = 0; i < MAX_CUBES; i++) {
				randomPositions[i] = GetRandomPoint();
				rotAxis[i] = rand() % 3;
			}
			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[3] = false;
		}
		for (int i = 0; i < MAX_CUBES; i++) {
			MyOctahedronShader::myRenderCode(currentTime, randomPositions[i], rotAxis[i]);			//Replace shader: use octahedron
		}

		break;


	//EXERCISE 5
	/* Make the cubes/octocahedrons of task 3 and/or 4 replace the random characters in the matrix effect, preserving 
	the same changes in color than the letters show.*/
	case 4:
		//Reset of exercises
		if (update[4]) {

			RV::_projection = glm::perspective(RV::FOV, (float)w / (float)h, RV::zNear, RV::zFar);

			RV::rota[0] = 0.f;
			RV::rota[1] = 0.f;

			RV::panv[0] = -50.f;
			RV::panv[1] = 0.f;
			RV::panv[2] = -100.f;

			for (int i = 0; i < NUM_COLS; i++) {
				matrix[i].speed = rand() % 20 + 5;
				matrix[i].initPos = rand() % 20;
				matrix[i].num_elements = rand() % NUM_ROWS + 10;
				
				if (matrix[i].num_elements > NUM_ROWS) {
					matrix[i].num_elements = 2*matrix[i].num_elements - NUM_ROWS;
				}
			}
			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[4] = false;
		}
		for (int i = 0; i < NUM_COLS; i++) {
			for (int j = 0; j < matrix[i].num_elements; j++) {

				rotAxisMatrix[i][j] = rand() % 3;

				if (j == 0) {
					MyOctahedronShader::myRenderCodeMatrix(currentTime, glm::vec4(i * 4 * sqrt(2)*OFFSET, j * 4 * sqrt(2) * OFFSET + matrix[i].initPos, matrix[i].initPos * 2, 1.f), rotAxisMatrix[i][j], matrix[i].speed, true);
				}
				else {
					MyOctahedronShader::myRenderCodeMatrix(currentTime, glm::vec4(i * 4 * sqrt(2)*OFFSET, j * 4 * sqrt(2) * OFFSET + matrix[i].initPos, matrix[i].initPos * 2, 1.f), rotAxisMatrix[i][j], matrix[i].speed, false);
				}
			}
		}

		break;


	//EXERCISE 6
	/* Consider again the structure of task 1. you generated a cube from a point. The point was in the center of the cube.
	This means that the result was a body-centered cube, from which a truncated octocahedron was generated.build a wireframe
	visualization of a bitruncated honeycomb from the truncated octocahedron structure. */

	/* 6.1. build a wireframe visualization of a bitruncated honeycomb from the truncated octocahedron structure.*/
	case 5:
		//Reset of exercises
		if (update[5]) {

			RV::rota[0] = 0.f;
			RV::rota[1] = 0.f;

			RV::panv[0] = 0.f;
			RV::panv[1] = 0.f;
			RV::panv[2] = -15.f;

			RV::_projection = glm::perspective(RV::FOV, (float)w / (float)h, RV::zNear, RV::zFar);

			RV::rota[0] = 0.f;
			RV::rota[1] = 0.f;

			randomPositions[0] = glm::vec4(-2 * sqrt(2), 0, 0, 1);
			randomPositions[1] = glm::vec4(2 * sqrt(2), 0, 0, 1);
			randomPositions[2] = glm::vec4(0, 2 * sqrt(2), -2 * sqrt(2), 1);
			randomPositions[3] = glm::vec4(0, 2 * sqrt(2), 2 * sqrt(2), 1);
			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[5] = false;
		}
		for (int i = 0; i < MAX_CUBES; i++) {
			MyOctahedronShaderWireframe::myRenderCode(currentTime, randomPositions[i]);
		}
		break;


	/* 6.2. Take again the falling octocahedrons of task 4.Impose that, when the scene is seen from the x-y plane (orthonormal perspective), 
	impose the falling octocahedrons to be aligned with the elements forming bitruncated honeycomb, and to rotate in a way that makes them
	often have the same orientation that the elements forming the  wireframe. When the falling octocahedrons meet an orientation that corresponds 
	to the wireframe of the honeycomb, make them turn white, like the characters in the matrix effect.*/

	case 6:
		if (update[6]) {

			auxMatrix = glm::mat4(1);
			angle = 0;

			RV::_projection = glm::ortho((float)(-w / aux), (float)(w / aux), (float)(-h / aux), (float)(h / aux), 0.01f, 100.f);

			RV::rota[0] = 0.f;
			RV::rota[1] = 0.f;

			RV::panv[0] = 0.f;
			RV::panv[1] = -10.f;
			RV::panv[2] = -15.f;

			//INIT STUFF
			for (int i = 0; i < MAX_CUBES; i++) {
				randomPositions[i].x = (2 * sqrt(2)) * (rand()%10);
				randomPositions[i].y = (2) * sqrt(2) * (rand() % 10);
				randomPositions[i].z = rand() % 10;
				rotAxis[i] = rand() % 3;
			}

			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[6] = false;
		}
		RV::_projection = glm::ortho((float)(-w / aux), (float)(w / aux), (float)(-h / aux), (float)(h / aux), 0.01f, 100.f);


		for (int i = 0; i < NUM_ROWS/2; i++) {
			for (int j = 0; j < NUM_COLS/2; j++) {

				if (i % 2 == 0) {
					if (j % 2 == 0) {
						MyOctahedronShaderWireframe::myRenderCode(currentTime, glm::vec4(j * 2 * sqrt(2), i * 2 * sqrt(2), 0 - 20, 1));
					}
				}
				else {
					if (j % 2 == 1) {
						MyOctahedronShaderWireframe::myRenderCode(currentTime, glm::vec4(j * 2 * sqrt(2), i * 2 * sqrt(2), 2 * sqrt(2)-20, 1));
						//MyOctahedronShaderWireframe::myRenderCode(currentTime, glm::vec4(j * 2 * sqrt(2), i * 2 * sqrt(2), -2 * sqrt(2), 1));
					}
				}
			}
		}

		//MyOctahedronShader::myRenderCode(currentTime, glm::vec4(0,4*sqrt(2),0,1), true);
		for (int i = 0; i < MAX_CUBES; i++) {
			MyOctahedronShader::myRenderCode(currentTime, randomPositions[i], rotAxis[i], true);			//Replace shader: use octahedron
		}

		angle += 0.01f;
		angle = fmod(angle, 1.55f);

		auxMatrix = glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0));

		break;
	default:
		break;
	}


	
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_targetViewPoint = RV::_projection *RV::_modelView;
	
	RV::_MVP = RV::_projection * RV::_modelView * rot*auxMatrix;
	ImGui::Render();
}

void myCleanupCode(void) {
	MyFirstShader::myCleanupCode();
	MyOctahedronShader::myCleanupCode();
	MyOctahedronShaderWireframe::myCleanupCode();


}


//////////////////////////////////// COMPILE AND LINK
GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}
void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char *buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}


/*
*
*
*
*
*		SHADER CUBO
*
*
*
*
*
*/

namespace MyFirstShader {
	void myCleanupCode() {
		glDeleteVertexArrays(1, &myVAO);
		glDeleteProgram(myRenderProgram);
	}


	GLuint myShaderCompile(void) {

		static const GLchar * vertex_shader_source[] =
		{
			"#version 330										\n\
			\n\
			uniform vec4 inOne;\n\
			void main() {\n\
			gl_Position = inOne; \n\
			}" };


		static const GLchar * geom_shader_source[] = {
			"#version 330 \n\
			uniform mat4 rotation;\n\
			uniform mat4 selfRot;\n\
			layout(triangles) in;\n\
			layout(triangle_strip, max_vertices = 120) out;\n\
			void main()\n\
			{	\n\
				//CARA 1\n\
				const vec4 vertices[4] = vec4[4](vec4(1, -1, 1, 1.0),\n\
										vec4(1, 1, 1, 1.0),\n\
										vec4(-1, -1, 1, 1.0),\n\
										vec4(-1, 1, 1, 1.0));\n\
				//CARA 2\n\
				const vec4 vertices2[4]= vec4[4](vec4(1, 1, 1, 1.0),\n\
										vec4(1, 1, -1, 1.0),\n\
										vec4(-1, 1, 1, 1.0),\n\
										vec4(-1, 1, -1, 1.0));\n\
				//CARA 3\n\
				const vec4 vertices3[4]= vec4[4](vec4(-1, -1, 1, 1.0),\n\
										vec4(-1, 1, 1, 1.0),\n\
										vec4(-1, -1, -1, 1.0),\n\
										vec4(-1, 1, -1, 1.0));\n\
				//CARA 4\n\
				const vec4 vertices4[4]= vec4[4](vec4(-1, -1, -1, 1.0),\n\
										vec4(-1, 1, -1, 1.0),\n\
										vec4(1, -1, -1, 1.0),\n\
										vec4(1, 1, -1, 1.0));\n\
				//CARA 5\n\
				const vec4 vertices5[4]= vec4[4](vec4(-1, -1, 1, 1.0),\n\
										vec4(-1, -1, -1, 1.0),\n\
										vec4(1, -1, 1, 1.0),\n\
										vec4(1, -1, -1, 1.0));\n\
				//CARA 6\n\
				const vec4 vertices6[4]= vec4[4](vec4(1, -1, -1, 1.0),\n\
										vec4(1, 1, -1, 1.0),\n\
										vec4(1, -1, 1, 1.0),\n\
										vec4(1, 1, 1, 1.0));\n\
				\n\
				\n\
				\n\
				//CARA 1\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*vertices[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 0;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA 2\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*vertices2[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 1;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 3\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*vertices3[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 2;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 4\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*vertices4[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 3;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 5\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*vertices5[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 4;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 6\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*vertices6[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
			}"
		};


		static const GLchar * fragment_shader_source[] =
		{
			"#version 330\n\
			\n\
			out vec4 color;\n\
			\n\
			void main() {\n\
			const vec4 colors[6] = vec4[6](vec4( 0, 1, 0, 1.0),\n\
											vec4(0, 0.9, 0, 1.0),\n\
											vec4( 0, 0.8, 0, 1.0),\n\
											vec4(0, 0.87, 0, 1.0),\n\
											vec4( 0, 0.95, 0, 1.0),\n\
											vec4( 0, 0.85, 0, 1.0));\n\
			color = colors[gl_PrimitiveID];\n\
			}" };


		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint geom_shader;
		GLuint program;

		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		geom_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geom_shader, 1, geom_shader_source, NULL);
		glCompileShader(geom_shader);

		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glAttachShader(program, geom_shader);
		glLinkProgram(program);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}





	void  myInitCode(void) {

		myRenderProgram = myShaderCompile();
		//glCreateVertexArrays(1, &myVAO); //use this one on class pc
		glGenVertexArrays(1, &myVAO);		//Use this one on home pc
		glBindVertexArray(myVAO);

		glUseProgram(myRenderProgram);


	}



	glm::mat4 myMVP;

	//CODE THAT RENDERS STATTIONARY OBJECTS
	void myRenderCode(double currentTime, glm::vec4 pos) {
	

		glUseProgram(myRenderProgram);
		glm::mat4 rotation = glm::mat4(1);

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);


		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	//CODE THAT RENDERS ROTATIONAL AND TRANSLATIONAL OBJECTS
	void myRenderCode(double currentTime, glm::vec4 pos, int axis) {

		glUseProgram(myRenderProgram);
		glm::mat4 rotation; 
		glm::mat4 translation;	

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);

		
		switch (axis) {

		//Rotation with X axis
		case 0:
			rotation = { 1.0f, 0.f, 0.f, 0.f,
							0.f, cos(currentTime), -sin(currentTime), 0.f,
							0.f, sin(currentTime), cos(currentTime), 0.f,
							0.f, 0.f, 0.f, 1.f };
			break;
			
		//Rotation with Y axis
		case 1:
			rotation = { cos(currentTime), 0.f, -sin(currentTime), 0.f,
				0.f, 1.f, 0.f, 0.f,
				sin(currentTime), 0.f, cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;
			
		//Rotation with Z axis
		case 2:
			rotation = { cos(currentTime), -sin(currentTime), 0.f, 0.f,
					        sin(currentTime), cos(currentTime), 0.f, 0.f,
						0.f, 0.f, 1.f, 0.f,
						0.f, 0.f, 0.f, 1.f };
			break;
		}
			

		//Translate the cubes
		translation = glm::translate(glm::mat4(1), glm::vec3(0.f, fmod(-currentTime, 10.f)+5.f, 0.f));
		
		

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP*translation));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}



	

}


/*
*
*
*
*
*		SHADER OCTAHEDRON
*
*
*
*
*
*/

namespace MyOctahedronShader {
	void myCleanupCode() {
		glDeleteVertexArrays(1, &myVAO);
		glDeleteProgram(myRenderProgram);
	}


	GLuint myShaderCompile(void) {

		static const GLchar * vertex_shader_source[] =
		{
			"#version 330										\n\
			\n\
			uniform vec4 inOne;\n\
			void main() {\n\
			gl_Position = inOne; \n\
			}" };



#pragma region "Octahedron"
		static const GLchar * geom_shader_source[] = {
			"\#version 330 \n\
			uniform mat4 rotation;\n\
			uniform mat4 selfRot;\n\
			uniform int IDAdd;\n\
			uniform float param;\n\
			layout(triangles) in;\n\
			layout(triangle_strip, max_vertices = 256) out;\n\
			void main()\n\
			{\n\
				//HEXAGON\n\
				//CARA A\n\
				vec4 verticesA[6] = vec4[6](\n\
												vec4(0 + (sqrt(2)*param), 2*sqrt(2) - (sqrt(2)*param), sqrt(2), 1.0),/*E*/ \n\
												vec4(0 + (sqrt(2)*param), sqrt(2), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*A*/ \n\
												vec4(sqrt(2), 2*sqrt(2) - (sqrt(2)*param), 0 + (sqrt(2)*param), 1.0),/*M*/ \n\
												vec4(sqrt(2), 0 + (sqrt(2)*param), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*V*/ \n\
												vec4(2*sqrt(2) - (sqrt(2)*param), sqrt(2), 0 + (sqrt(2)*param), 1.0),/*N*/ \n\
												vec4(2*sqrt(2) - (sqrt(2)*param), 0 + (sqrt(2)*param), sqrt(2), 1.0));/*R*/ \n\
				\n\
				for (int i = 0; i<6; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*verticesA[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 0+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA B\n\
				vec4 verticesB[6] = vec4[6](\n\
												vec4(0 + (sqrt(2)*(-param)), sqrt(2), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*A*/ \n\
												vec4(0 + (sqrt(2)*(-param)), 2*sqrt(2) - (sqrt(2)*param), sqrt(2), 1.0),/*E*/ \n\
												vec4(-sqrt(2), 0 + (sqrt(2)*param), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*W*/ \n\
												vec4(-sqrt(2), 2*sqrt(2) - (sqrt(2)*param), 0 + (sqrt(2)*param), 1.0),/*J*/ \n\
												vec4(-2*sqrt(2) + (sqrt(2)*param), 0 + (sqrt(2)*param), sqrt(2), 1.0),/*S*/ \n\
												vec4(-2*sqrt(2) + (sqrt(2)*param), sqrt(2), 0 + (sqrt(2)*param), 1.0));/*O*/ \n\
				\n\
				for (int i = 0; i<6; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*verticesB[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 1+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA C\n\
				vec4 verticesC[6] = vec4[6](\n\
												vec4(sqrt(2), 0 + (sqrt(2)*(-param)), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*V*/ \n\
												vec4(0 + (sqrt(2)*(param)), -sqrt(2), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*B*/ \n\
												vec4(2*sqrt(2) - (sqrt(2)*param), 0 + (sqrt(2)*(-param)), sqrt(2), 1.0),/*R*/ \n\
												vec4(0 + (sqrt(2)*(param)), -2*sqrt(2) + (sqrt(2)*param), sqrt(2), 1.0),/*F*/ \n\
												vec4(2*sqrt(2) - (sqrt(2)*param), -sqrt(2), 0 + (sqrt(2)*(param)), 1.0),/*P*/ \n\
												vec4(sqrt(2), -2 * sqrt(2) - (sqrt(2)*-param), 0 + (sqrt(2)*(param)), 1.0));/*K*/ \n\
			\n\
			for (int i = 0; i<6; i++) \n\
				{\n\
					gl_Position =rotation*(selfRot*verticesC[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 2+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA D\n\
				vec4 verticesD[6] = vec4[6](\n\
												vec4(0+ (sqrt(2)*(-param)), -2*sqrt(2)- (sqrt(2)*-param), sqrt(2), 1.0),/*F */ \n\
												vec4(0+ (sqrt(2)*(-param)), -sqrt(2), 2*sqrt(2)- (sqrt(2)*param), 1.0),/*B */ \n\
												vec4(-sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(param)), 1.0),/*L */ \n\
												vec4(-sqrt(2), 0+ (sqrt(2)*(-param)), 2*sqrt(2)- (sqrt(2)*param), 1.0),/*W*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), -sqrt(2), 0+ (sqrt(2)*(param)), 1.0),/*Q*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(-param)), sqrt(2), 1.0));/*S*/ \n\
			\n\
			for (int i = 0; i<6; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesD[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 3+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA E\n\
				vec4 verticesE[6] = vec4[6](\n\
												vec4(sqrt(2), 2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(-param)), 1.0),/*M*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), sqrt(2), 0+ (sqrt(2)*(-param)), 1.0),/*N*/ \n\
												vec4(0+ (sqrt(2)*(param)), 2*sqrt(2)- (sqrt(2)*param), -sqrt(2), 1.0),/*G*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(param)), -sqrt(2), 1.0),/*T*/ \n\
												vec4(0+ (sqrt(2)*(param)), sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*C*/ \n\
												vec4(sqrt(2), 0+ (sqrt(2)*(param)), -2*sqrt(2)- (sqrt(2)*-param), 1.0));/*Z*/ \n\
			\n\
			for (int i = 0; i<6; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesE[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 4+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA F\n\
				vec4 verticesF[6] = vec4[6](\n\
												vec4(2*sqrt(2)- (sqrt(2)*param), -sqrt(2), 0+ (sqrt(2)*(-param)), 1.0),/*P*/ \n\
												vec4(sqrt(2), -2 * sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(-param)), 1.0),/*K*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(-param)), -sqrt(2), 1.0),/*T*/ \n\
												vec4(0+ (sqrt(2)*(param)), -2*sqrt(2)- (sqrt(2)*-param), -sqrt(2), 1.0),/*H*/ \n\
												vec4(sqrt(2), 0+ (sqrt(2)*(-param)), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*Z*/ \n\
												vec4(0+ (sqrt(2)*(param)), -sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 1.0));/*D*/ \n\
			\n\
			for (int i = 0; i<6; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesF[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA G\n\
				vec4 verticesG[6] = vec4[6](\n\
												vec4(-sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(-param)), 1.0),/*L*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), -sqrt(2), 0+ (sqrt(2)*(-param)), 1.0),/*Q*/ \n\
												vec4(0 + (sqrt(2)*(-param)), -2*sqrt(2)- (sqrt(2)*-param), -sqrt(2), 1.0),/*H*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(-param)), -sqrt(2), 1.0),/*U*/ \n\
												vec4(0+ (sqrt(2)*(-param)), -sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*D*/ \n\
												vec4(-sqrt(2), 0+ (sqrt(2)*(-param)), -2*sqrt(2)- (sqrt(2)*-param), 1.0));/*AA*/ \n\
			\n\
			for (int i = 0; i<6; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesG[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA H\n\
				vec4 verticesH[6] = vec4[6](\n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(param)), -sqrt(2), 1.0),/*U*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), sqrt(2), 0+ (sqrt(2)*(-param)), 1.0),/*O */ \n\
												vec4(-sqrt(2), 0+ (sqrt(2)*(param)), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*AA*/ \n\
												vec4(-sqrt(2), 2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(-param)), 1.0),/*J*/ \n\
												vec4(0+ (sqrt(2)*(-param)), sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*C*/ \n\
												vec4(0+ (sqrt(2)*(-param)), 2*sqrt(2)- (sqrt(2)*param), -sqrt(2), 1.0));/*G*/ \n\
			\n\
			for (int i = 0; i<6; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesH[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				\n\
				\n\
				//CARA CA\n\
				vec4 verticesCA[8] = vec4[8](\n\
												vec4(0 + (sqrt(2)*-param), sqrt(2), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*A'-*/ \n\
												vec4(-sqrt(2), 0 + (sqrt(2)*(param)), 2 * sqrt(2) - (sqrt(2)*param), 1.0),/*W-*/ \n\
												vec4(0 + (sqrt(2)*param), sqrt(2), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*A-*/ \n\
												vec4(-sqrt(2), 0 + (sqrt(2)*(-param)), 2 * sqrt(2) - (sqrt(2)*param), 1.0),/*W-'*/ \n\
												vec4(sqrt(2), 0 + (sqrt(2)*param), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*V-*/ \n\
												vec4(0 + (sqrt(2)*(-param)), -sqrt(2), 2 * sqrt(2) - (sqrt(2)*param), 1.0),/*B */ \n\
												vec4(sqrt(2), 0 + (sqrt(2)*-param), 2*sqrt(2) - (sqrt(2)*param), 1.0),/*V'-*/ \n\
												vec4(0 + (sqrt(2)*(param)), -sqrt(2), 2 * sqrt(2) - (sqrt(2)*param), 1.0));/*B' */ \n\
		\n\
			for (int i = 0; i<8; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesCA[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 0+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA CB\n\
				vec4 verticesCB[8] = vec4[8](\n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(param)), sqrt(2), 1.0),/*S-*/ \n\
												vec4(-2*sqrt(2) + (sqrt(2)*param), sqrt(2), 0 + (sqrt(2)*param), 1.0),/*O*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(-param)), sqrt(2), 1.0),/*S-*/ \n\
												vec4(-2*sqrt(2) + (sqrt(2)*param), sqrt(2), 0 + (sqrt(2)*-param), 1.0),/*O*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), -sqrt(2), 0+ (sqrt(2)*(param)), 1.0),/*Q-*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(param)), -sqrt(2), 1.0),/*U-*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), -sqrt(2), 0+ (sqrt(2)*(-param)), 1.0),/*Q*/ \n\
												vec4(-2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(-param)), -sqrt(2), 1.0));/*U-*/ \n\
			\n\
			for (int i = 0; i<8; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesCB[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 1+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA CC\n\
				vec4 verticesCC[8] = vec4[8](\n\
												vec4(0 + (sqrt(2)*(param)), 2*sqrt(2) - (sqrt(2)*param), sqrt(2), 1.0),/*E*/ \n\
												vec4(sqrt(2), 2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(param)), 1.0),/*M*/ \n\
												vec4(0 + (sqrt(2)*(-param)), 2*sqrt(2) - (sqrt(2)*param), sqrt(2), 1.0),/*E*/ \n\
												vec4(sqrt(2), 2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(-param)), 1.0),/*M*/ \n\
												vec4(-sqrt(2), 2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(param)), 1.0),/*J*/ \n\
												vec4(0+ (sqrt(2)*(param)), 2*sqrt(2)- (sqrt(2)*param), -sqrt(2), 1.0),/*G*/ \n\
												vec4(-sqrt(2), 2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(-param)), 1.0),/*J*/ \n\
												vec4(0+ (sqrt(2)*(-param)), 2*sqrt(2)- (sqrt(2)*param), -sqrt(2), 1.0));/*G*/ \n\
			\n\
			for (int i = 0; i<8; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesCC[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 2+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA CD\n\
				vec4 verticesCD[8] = vec4[8](\n\
												vec4(0+ (sqrt(2)*(param)), sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*C*/ \n\
												vec4(sqrt(2), 0+ (sqrt(2)*(param)), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*Z*/ \n\
												vec4(0+ (sqrt(2)*(-param)), sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*C*/ \n\
												vec4(sqrt(2), 0+ (sqrt(2)*(-param)), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*Z*/ \n\
												vec4(-sqrt(2), 0+ (sqrt(2)*(param)), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*AA*/ \n\
												vec4(0+ (sqrt(2)*(param)), -sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*D*/ \n\
												vec4(-sqrt(2), 0+ (sqrt(2)*(-param)), -2*sqrt(2)- (sqrt(2)*-param), 1.0),/*AA*/ \n\
												vec4(0+ (sqrt(2)*(-param)), -sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 1.0));/*D*/ \n\
			\n\
			for (int i = 0; i<8; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesCD[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 3+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA CE\n\
				vec4 verticesCE[8] = vec4[8](\n\
												vec4(0+ (sqrt(2)*(-param)), -2*sqrt(2)- (sqrt(2)*-param), sqrt(2), 1.0),/*F */ \n\
												vec4(-sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(param)), 1.0),/*L*/ \n\
												vec4(0+ (sqrt(2)*(param)), -2*sqrt(2)- (sqrt(2)*-param), sqrt(2), 1.0),/*F */ \n\
												vec4(-sqrt(2), -2*sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(-param)), 1.0),/*L*/ \n\
												vec4(sqrt(2), -2 * sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(param)), 1.0),/*K*/ \n\
												vec4(0 + (sqrt(2)*(-param)), -2*sqrt(2)- (sqrt(2)*-param), -sqrt(2), 1.0),/*H*/ \n\
												vec4(sqrt(2), -2 * sqrt(2)- (sqrt(2)*-param), 0+ (sqrt(2)*(-param)), 1.0),/*K*/ \n\
												vec4(0 + (sqrt(2)*(param)), -2*sqrt(2)- (sqrt(2)*-param), -sqrt(2), 1.0));/*H*/ \n\
			\n\
			for (int i = 0; i<8; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesCE[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 4+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA CF\n\
				vec4 verticesCF[8] = vec4[8](\n\
												vec4(2*sqrt(2) - (sqrt(2)*param), 0 + (sqrt(2)*-param), sqrt(2), 1.0),/*R*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), -sqrt(2), 0+ (sqrt(2)*(param)), 1.0),/*P*/ \n\
												vec4(2*sqrt(2) - (sqrt(2)*param), 0 + (sqrt(2)*param), sqrt(2), 1.0),/*R*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), -sqrt(2), 0+ (sqrt(2)*(-param)), 1.0),/*P*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), sqrt(2), 0+ (sqrt(2)*(param)), 1.0),/*N*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(-param)), -sqrt(2), 1.0),/*T*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), sqrt(2), 0+ (sqrt(2)*(-param)), 1.0),/*N*/ \n\
												vec4(2*sqrt(2)- (sqrt(2)*param), 0+ (sqrt(2)*(param)), -sqrt(2), 1.0));/*T*/ \n\
			\n\
			for (int i = 0; i<8; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesCF[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				\n\
			"
			"\
				// CARA RECTANGULO A \n\
				vec4 verticesRA[4] = vec4[4](\n\
												vec4(0 + (sqrt(2) * param), sqrt(2), 2 * sqrt(2) - (sqrt(2) * param), 1.0),/*A*/ \n\
												vec4(0 + (sqrt(2) * param), 2 * sqrt(2) - (sqrt(2) * param), sqrt(2), 1.0),/*E*/ \n\
												vec4(0 + (sqrt(2) * -param), sqrt(2), 2 * sqrt(2) - (sqrt(2) * param), 1.0),/*A'*/ \n\
												vec4(0 + (sqrt(2) * -param), 2 * sqrt(2) - (sqrt(2) * param), sqrt(2), 1.0));/*E'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRA[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 0+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO B \n\
				vec4 verticesRB[4] = vec4[4](\n\
												vec4(-sqrt(2), 0 + (sqrt(2) * param), 2 * sqrt(2) - (sqrt(2) * param), 1.0),/*W*/ \n\
												vec4(-2 * sqrt(2) + (sqrt(2) * param), 0 + (sqrt(2) * param), sqrt(2), 1.0),/*S*/ \n\
												vec4(-sqrt(2), 0 + (sqrt(2) * -param), 2 * sqrt(2) - (sqrt(2) * param), 1.0),/*W'*/ \n\
												vec4(-2 * sqrt(2) + (sqrt(2) * param), 0 + (sqrt(2) * -param), sqrt(2), 1.0));/*S'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRB[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 1+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO C \n\
				vec4 verticesRC[4] = vec4[4](\n\
												vec4(0 + (sqrt(2) * param), -2 * sqrt(2) - (sqrt(2) * -param), sqrt(2), 1.0),/*F*/ \n\
												vec4(0 + (sqrt(2) * param), -sqrt(2), 2 * sqrt(2) - (sqrt(2) * param), 1.0),/*B*/ \n\
												vec4(0 + (sqrt(2) * -param), -2 * sqrt(2) - (sqrt(2) * -param), sqrt(2), 1.0),/*F'*/ \n\
												vec4(0 + (sqrt(2) * -param), -sqrt(2), 2 * sqrt(2) - (sqrt(2) * param), 1.0));/*B'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRC[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 2+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO D \n\
				vec4 verticesRD[4] = vec4[4](\n\
												vec4(2 * sqrt(2) - (sqrt(2) * param), sqrt(2), 0 + (sqrt(2) * -param), 1.0),/*N*/ \n\
												vec4(sqrt(2), 2 * sqrt(2) - (sqrt(2) * param), 0 + (sqrt(2) * -param), 1.0),/*M*/ \n\
												vec4(2 * sqrt(2) - (sqrt(2) * param), sqrt(2), 0 + (sqrt(2) * param), 1.0),/*N'*/ \n\
												vec4(sqrt(2), 2 * sqrt(2) - (sqrt(2) * param), 0 + (sqrt(2) * param), 1.0));/*M'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRD[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 3+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO E \n\
				vec4 verticesRE[4] = vec4[4](\n\
												vec4(2 * sqrt(2) - (sqrt(2) * param), 0 + (sqrt(2) * -param), -sqrt(2), 1.0),/*T*/ \n\
												vec4(sqrt(2), 0 + (sqrt(2) * -param), -2 * sqrt(2) - (sqrt(2) * -param), 1.0),/*Z*/ \n\
												vec4(2 * sqrt(2) - (sqrt(2) * param), 0 + (sqrt(2) * param), -sqrt(2), 1.0),/*T'*/ \n\
												vec4(sqrt(2), 0 + (sqrt(2) * param), -2 * sqrt(2) - (sqrt(2) * -param), 1.0));/*Z'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRE[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 4+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO F \n\
				vec4 verticesRF[4] = vec4[4](\n\
												vec4(0 + (sqrt(2) * param), 2 * sqrt(2) - (sqrt(2) * param), -sqrt(2), 1.0),/*G*/ \n\
												vec4(0 + (sqrt(2) * param), sqrt(2), -2 * sqrt(2) - (sqrt(2) * -param), 1.0),/*C*/ \n\
												vec4(0 + (sqrt(2) * -param), 2 * sqrt(2) - (sqrt(2) * param), -sqrt(2), 1.0),/*G'*/ \n\
												vec4(0 + (sqrt(2) * -param), sqrt(2), -2 * sqrt(2) - (sqrt(2) * -param), 1.0));/*C'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRF[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO G \n\
				vec4 verticesRG[4] = vec4[4](\n\
												vec4(0 + (sqrt(2) * -param), -2 * sqrt(2) - (sqrt(2) * -param), -sqrt(2), 1.0),/*H*/ \n\
												vec4(0 + (sqrt(2) * -param), -sqrt(2), -2 * sqrt(2)- (sqrt(2) * -param), 1.0),/*D*/ \n\
												vec4(0 + (sqrt(2) * param), -2 * sqrt(2) - (sqrt(2) * -param), -sqrt(2), 1.0),/*H'*/ \n\
												vec4(0 + (sqrt(2) * param), -sqrt(2), -2 * sqrt(2)- (sqrt(2) * -param), 1.0));/*D'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRG[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 0+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO H \n\
				vec4 verticesRH[4] = vec4[4](\n\
												vec4(sqrt(2), -2 * sqrt(2) - (sqrt(2) * -param), 0 + (sqrt(2) * -param), 1.0),/*K*/ \n\
												vec4(2 * sqrt(2) - (sqrt(2) * param), -sqrt(2), 0 + (sqrt(2) * -param), 1.0),/*P*/ \n\
												vec4(sqrt(2), -2 * sqrt(2) - (sqrt(2) * -param), 0 + (sqrt(2) * param), 1.0),/*K'*/ \n\
												vec4(2 * sqrt(2) - (sqrt(2) * param), -sqrt(2), 0 + (sqrt(2) * param), 1.0));/*P'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRH[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 1+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO I \n\
				vec4 verticesRI[4] = vec4[4](\n\
												vec4(-2 * sqrt(2) - (sqrt(2) * -param), -sqrt(2), 0 + (sqrt(2) * -param), 1.0),/*Q*/ \n\
												vec4(-sqrt(2), -2 * sqrt(2) - (sqrt(2) * -param), 0 + (sqrt(2) * -param), 1.0),/*L*/ \n\
												vec4(-2 * sqrt(2) - (sqrt(2) * -param), -sqrt(2), 0 + (sqrt(2) * param), 1.0),/*Q'*/ \n\
												vec4(-sqrt(2), -2 * sqrt(2) - (sqrt(2) * -param), 0 + (sqrt(2) * param), 1.0));/*L'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRI[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 2+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO J \n\
				vec4 verticesRJ[4] = vec4[4](\n\
												vec4(-sqrt(2), 2 * sqrt(2) - (sqrt(2) * param), 0 + (sqrt(2) * -param), 1.0),/*J*/ \n\
												vec4(-2 * sqrt(2) + (sqrt(2) * param), sqrt(2), 0 + (sqrt(2) * -param), 1.0),/*O*/ \n\
												vec4(-sqrt(2), 2 * sqrt(2) - (sqrt(2) * param), 0 + (sqrt(2) * param), 1.0),/*J'*/ \n\
												vec4(-2 * sqrt(2) + (sqrt(2) * param), sqrt(2), 0 + (sqrt(2) * param), 1.0));/*O'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRJ[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 3+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO K \n\
				vec4 verticesRK[4] = vec4[4](\n\
												vec4(-2 * sqrt(2) - (sqrt(2) * -param), 0 + (sqrt(2) * param), -sqrt(2), 1.0),/*U*/ \n\
												vec4(-sqrt(2), 0 + (sqrt(2) * param), -2 * sqrt(2) - (sqrt(2) * -param), 1.0),/*AA*/ \n\
												vec4(-2 * sqrt(2) - (sqrt(2) * -param), 0 + (sqrt(2) * -param), -sqrt(2), 1.0),/*U'*/ \n\
												vec4(-sqrt(2), 0 + (sqrt(2) * -param), -2 * sqrt(2) - (sqrt(2) * -param), 1.0));/*AA'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRK[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 4+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				// CARA RECTANGULO L \n\
				vec4 verticesRL[4] = vec4[4](\n\
												vec4(sqrt(2), 0 + (sqrt(2) * -param), 2 * sqrt(2) - (sqrt(2) * param), 1.0),/*V*/ \n\
												vec4(2 * sqrt(2) - (sqrt(2) * param), 0 + (sqrt(2) * -param), sqrt(2), 1.0),/*R*/ \n\
												vec4(sqrt(2), 0 + (sqrt(2) * param), 2 * sqrt(2) - (sqrt(2) * param), 1.0),/*V'*/ \n\
												vec4(2 * sqrt(2) - (sqrt(2) * param), 0 + (sqrt(2) * param), sqrt(2), 1.0));/*R'*/ \n\
			\n\
			for (int i = 0; i<4; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesRL[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
			}"
		};
#pragma endregion

		static const GLchar * fragment_shader_source[] =
		{
			"#version 330\n\
			\n\
			out vec4 color;\n\
			\n\
			void main() {\n\
			const vec4 colors[13] = vec4[13](vec4( 0, 1, 0, 1.0),\n\
											vec4(0, 0.8, 0, 1.0),\n\
											vec4( 0, 0.7, 0, 1.0),\n\
											vec4(0, 0.77, 0, 1.0),\n\
											vec4( 0, 0.9, 0, 1.0),\n\
											vec4( 0, 0.6, 0, 1.0),//White\n\
											vec4(1, 1, 1, 1.0),\n\
											vec4(1, 0.9, 1, 1.0),\n\
											vec4(0.9, 1, 1, 1.0),\n\
											vec4(1, 1, 0.9, 1.0),\n\
											vec4(0.9, 0.9, 1, 1.0),\n\
											vec4(1, 0.9, 0.9, 1.0),\n\
											vec4(1, 0.1, 0.1, 1.0));\n\
			color = colors[gl_PrimitiveID];\n\
			}" };


		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint geom_shader;
		GLuint program;

		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		geom_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geom_shader, 1, geom_shader_source, NULL);
		glCompileShader(geom_shader);

		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glAttachShader(program, geom_shader);
		glLinkProgram(program);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}





	void  myInitCode(void) {

		myRenderProgram = myShaderCompile();
		//glCreateVertexArrays(1, &myVAO); //use this one on class pc
		glGenVertexArrays(1, &myVAO);		//Use this one on home pc
		glBindVertexArray(myVAO);

		glUseProgram(myRenderProgram);


	}



	glm::mat4 myMVP;

	//CODE THAT RENDERS STATTIONARY OBJECTS
	void myRenderCode(double currentTime, glm::vec4 pos) {

		glUseProgram(myRenderProgram);
		glm::mat4 rotation = glm::mat4(1);

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);

		glUniform1f(glGetUniformLocation(myRenderProgram, "param"), (float)abs(cos(currentTime))/*paramSlide*/);

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	//CODE THAT RENDERS ROTATIONAL AND TRANSLATIONAL OBJECTS
	void myRenderCode(double currentTime, glm::vec4 pos, int axis) {

		glUseProgram(myRenderProgram);
		glm::mat4 rotation;
		glm::mat4 translation;

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);


		switch (axis) {

			//Rotation with X axis
		case 0:
			rotation = { 1.0f, 0.f, 0.f, 0.f,
				0.f, cos(currentTime), -sin(currentTime), 0.f,
				0.f, sin(currentTime), cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Y axis
		case 1:
			rotation = { cos(currentTime), 0.f, -sin(currentTime), 0.f,
				0.f, 1.f, 0.f, 0.f,
				sin(currentTime), 0.f, cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Z axis
		case 2:
			rotation = { cos(currentTime), -sin(currentTime), 0.f, 0.f,
				sin(currentTime), cos(currentTime), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;
		}


		//Translate the cubes
		translation = glm::translate(glm::mat4(1), glm::vec3(0.f, fmod(-currentTime, 10.f) + 5.f, 0.f));

		glUniform1f(glGetUniformLocation(myRenderProgram, "param"), paramSlide);

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP*translation));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	//CODE THAT RENDERS ROTATIONAL AND TRANSLATIONAL OBJECTS IF ALIGNED
	void myRenderCode(double currentTime, glm::vec4 pos, int axis, bool checkAlign) {

		glUseProgram(myRenderProgram);
		glm::mat4 rotation;
		glm::mat4 translation;

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);


		switch (axis) {

			//Rotation with X axis
		case 0:
			rotation = { 1.0f, 0.f, 0.f, 0.f,
				0.f, cos(currentTime), -sin(currentTime), 0.f,
				0.f, sin(currentTime), cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Y axis
		case 1:
			rotation = { cos(currentTime), 0.f, -sin(currentTime), 0.f,
				0.f, 1.f, 0.f, 0.f,
				sin(currentTime), 0.f, cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Z axis
		case 2:
			rotation = { cos(currentTime), -sin(currentTime), 0.f, 0.f,
				sin(currentTime), cos(currentTime), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;
		}

		int IDAdd = 0;
		float offset = 0.01;


		if ((cos(currentTime) >= -offset * 2) && (cos(currentTime) <= offset * 10)  /*|| (cos(currentTime) > -1) && (cos(currentTime) <= (-1 + offset )) || (cos(currentTime) >= (1 - offset )) && (cos(currentTime) < 1)*/) {
			IDAdd = 6;
			//std::cout << cos(currentTime) <<std::endl;
		}
		else {
			IDAdd = 0;
		}

		glUniform1i(glGetUniformLocation(myRenderProgram, "IDAdd"), IDAdd);


		//Translate the cubes
		translation = glm::translate(glm::mat4(1), glm::vec3(0.f, (fmod(-currentTime, 16.f)+16) /2, 0.f));

		glUniform1f(glGetUniformLocation(myRenderProgram, "param"), paramSlide);

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP*translation));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	//CODE THAT RENDERS THE MATRIX
	void myRenderCodeMatrix(double currentTime, glm::vec4 pos, int axis, int speed, bool first) {
		glUseProgram(myRenderProgram);

		glm::mat4 rotation;
		glm::mat4 translation;
		int IDAdd = 0;

		

		if (first) {
			IDAdd = 6;
		}
		else {
			IDAdd = 0;
		}

		glUniform1i(glGetUniformLocation(myRenderProgram, "IDAdd"), IDAdd);
		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);

		switch (axis) {

			//Rotation with X axis
		case 0:
			rotation = { 1.0f, 0.f, 0.f, 0.f,
				0.f, cos(currentTime), -sin(currentTime), 0.f,
				0.f, sin(currentTime), cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Y axis
		case 1:
			rotation = { cos(currentTime), 0.f, -sin(currentTime), 0.f,
				0.f, 1.f, 0.f, 0.f,
				sin(currentTime), 0.f, cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Z axis
		case 2:
			rotation = { cos(currentTime), -sin(currentTime), 0.f, 0.f,
				sin(currentTime), cos(currentTime), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;
		}

		translation = glm::translate(glm::mat4(1), glm::vec3(0.f, (fmod(-currentTime, 10.f) + 5.f)*speed, 0.f));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP*translation));
		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));


		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

}





/*
*
*
*
*
*		SHADER WIREFRAME OCTAHEDRON
*
*
*
*
*
*/

namespace MyOctahedronShaderWireframe {
	void myCleanupCode() {
		glDeleteVertexArrays(1, &myVAO);
		glDeleteProgram(myRenderProgram);
	}


	GLuint myShaderCompile(void) {

		static const GLchar * vertex_shader_source[] =
		{
			"#version 330										\n\
			\n\
			uniform vec4 inOne;\n\
			void main() {\n\
			gl_Position = inOne; \n\
			}" };



#pragma region "Octahedron Wireframe"
		static const GLchar * geom_shader_source[] = {
			"#version 330 \n\
			uniform mat4 rotation;\n\
			uniform mat4 selfRot;\n\
			layout(triangles) in;\n\
			uniform int IDAdd;\n\
			layout(line_strip, max_vertices = 100) out;\n\
			void main()\n\
			{\n\
				//HEXAGON\n\
				//CARA A\n\
				vec4 verticesA[7] = vec4[7](\n\
												vec4(0, 2 * sqrt(2), sqrt(2), 1.0),/*E*/ \n\
												vec4(0, sqrt(2), 2 * sqrt(2), 1.0),/*A*/ \n\
												vec4(sqrt(2), 0, 2 * sqrt(2), 1.0),/*V*/ \n\
												vec4(2 * sqrt(2), 0, sqrt(2), 1.0),/*R*/ \n\
												vec4(2 * sqrt(2), sqrt(2), 0, 1.0),/*N*/ \n\
												vec4(sqrt(2), 2 * sqrt(2), 0, 1.0),/*M*/ \n\
												vec4(0, 2 * sqrt(2), sqrt(2), 1.0));/*E*/ \n\
				\n\
				for (int i = 0; i<7; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*verticesA[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 0+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA B\n\
				vec4 verticesB[7] = vec4[7](\n\
												vec4(0, sqrt(2), 2*sqrt(2), 1.0),/*A*/ \n\
												vec4(0, 2*sqrt(2), sqrt(2), 1.0),/*E*/ \n\
												vec4(-sqrt(2), 2 * sqrt(2), 0, 1.0),/*J*/ \n\
												vec4(-2 * sqrt(2), sqrt(2), 0, 1.0),/*O*/ \n\
												vec4(-2 * sqrt(2), 0, sqrt(2), 1.0),/*S*/ \n\
												vec4(-sqrt(2), 0, 2 * sqrt(2), 1.0),/*W*/ \n\
												vec4(0, sqrt(2), 2 * sqrt(2), 1.0));/*A*/ \n\
				\n\
				for (int i = 0; i<7; i++)\n\
				{\n\
					gl_Position = rotation*(selfRot*verticesB[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 1+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA C\n\
				vec4 verticesC[7] = vec4[7](\n\
												vec4(sqrt(2), 0, 2 * sqrt(2), 1.0),/*V*/ \n\
												vec4(0, -sqrt(2), 2 * sqrt(2), 1.0),/*B*/ \n\
												vec4(0, -2 * sqrt(2), sqrt(2), 1.0),/*F*/ \n\
												vec4(sqrt(2), -2 * sqrt(2), 0, 1.0),/*K*/ \n\
												vec4(2 * sqrt(2), -sqrt(2), 0, 1.0),/*P*/ \n\
												vec4(2 * sqrt(2), 0, sqrt(2), 1.0),/*R*/ \n\
												vec4(sqrt(2), 0, 2 * sqrt(2), 1.0));/*V*/ \n\
			\n\
			for (int i = 0; i<7; i++) \n\
				{\n\
					gl_Position =rotation*(selfRot*verticesC[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 2+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA D\n\
				vec4 verticesD[7] = vec4[7](\n\
												vec4(0, -2 * sqrt(2), sqrt(2), 1.0),/*F*/ \n\
												vec4(0, -sqrt(2), 2 * sqrt(2), 1.0),/*B*/ \n\
												vec4(-sqrt(2), 0, 2 * sqrt(2), 1.0),/*W*/ \n\
												vec4(-2 * sqrt(2), 0, sqrt(2), 1.0),/*S*/ \n\
												vec4(-2 * sqrt(2), -sqrt(2), 0, 1.0),/*Q*/ \n\
												vec4(-sqrt(2), -2 * sqrt(2), 0, 1.0),/*L*/ \n\
												vec4(0, -2 * sqrt(2), sqrt(2), 1.0));/*F*/ \n\
			\n\
			for (int i = 0; i<7; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesD[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 3+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA E\n\
				vec4 verticesE[7] = vec4[7](\n\
												vec4(sqrt(2), 2 * sqrt(2), 0, 1.0),/*M*/ \n\
												vec4(2 * sqrt(2), sqrt(2), 0, 1.0),/*N*/ \n\
												vec4(2 * sqrt(2), 0, -sqrt(2), 1.0),/*T*/ \n\
												vec4(sqrt(2), 0, -2 * sqrt(2), 1.0),/*Z*/ \n\
												vec4(0, sqrt(2), -2 * sqrt(2), 1.0),/*C*/ \n\
												vec4(0, 2 * sqrt(2), -sqrt(2), 1.0),/*G*/ \n\
												vec4(sqrt(2), 2 * sqrt(2), 0, 1.0));/*M*/ \n\
			\n\
			for (int i = 0; i<7; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesE[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 4+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA F\n\
				vec4 verticesF[7] = vec4[7](\n\
												vec4(2*sqrt(2), -sqrt(2), 0, 1.0),/*P*/ \n\
												vec4(sqrt(2), -2 * sqrt(2), 0, 1.0),/*K*/ \n\
												vec4(0, -2*sqrt(2), -sqrt(2), 1.0),/*H*/ \n\
												vec4(0, -sqrt(2), -2*sqrt(2), 1.0),/*D*/ \n\
												vec4(sqrt(2), 0, -2*sqrt(2), 1.0),/*Z*/ \n\
												vec4(2*sqrt(2), 0, -sqrt(2), 1.0),/*T*/ \n\
												vec4(2*sqrt(2), -sqrt(2), 0, 1.0));/*P*/ \n\
			\n\
			for (int i = 0; i<7; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesF[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA G\n\
				vec4 verticesG[7] = vec4[7](\n\
												vec4(-sqrt(2), -2*sqrt(2), 0, 1.0),/*L*/ \n\
												vec4(-2*sqrt(2), -sqrt(2), 0, 1.0),/*Q*/ \n\
												vec4(-2*sqrt(2), 0, -sqrt(2), 1.0),/*U*/ \n\
												vec4(-sqrt(2), 0, -2*sqrt(2), 1.0),/*AA*/ \n\
												vec4(0, -sqrt(2), -2*sqrt(2), 1.0),/*D*/ \n\
												vec4(0, -2*sqrt(2), -sqrt(2), 1.0),/*H*/ \n\
												vec4(-sqrt(2), -2*sqrt(2), 0, 1.0));/*L*/ \n\
			\n\
			for (int i = 0; i<7; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesG[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA H\n\
				vec4 verticesH[7] = vec4[7](\n\
												vec4(-2*sqrt(2), 0, -sqrt(2), 1.0),/*U*/ \n\
												vec4(-2*sqrt(2), sqrt(2), 0, 1.0),/*O*/ \n\
												vec4(-sqrt(2), 2*sqrt(2), 0, 1.0),/*J*/ \n\
												vec4(0, 2*sqrt(2), -sqrt(2), 1.0),/*G*/ \n\
												vec4(0, sqrt(2), -2*sqrt(2), 1.0),/*C*/ \n\
												vec4(-sqrt(2), 0, -2*sqrt(2), 1.0),/*AA*/ \n\
												vec4(-2*sqrt(2), 0, -sqrt(2), 1.0));/*U*/ \n\
			\n\
			for (int i = 0; i<7; i++) \n\
				{\n\
					gl_Position = rotation*(selfRot*verticesH[i]+gl_in[0].gl_Position);\n\
					gl_PrimitiveID = 5+IDAdd;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
			}"
		};
#pragma endregion

		static const GLchar * fragment_shader_source[] =
		{
			"#version 330\n\
			\n\
			out vec4 color;\n\
			\n\
			void main() {\n\
			const vec4 colors[12] = vec4[12](vec4( 0, 1, 0, 1.0),\n\
											vec4(0, 0.8, 0, 1.0),\n\
											vec4( 0, 0.7, 0, 1.0),\n\
											vec4(0, 0.77, 0, 1.0),\n\
											vec4( 0, 0.9, 0, 1.0),\n\
											vec4( 0, 0.6, 0, 1.0),//White\n\
											vec4(1, 1, 1, 1.0),\n\
											vec4(1, 0.9, 1, 1.0),\n\
											vec4(0.9, 1, 1, 1.0),\n\
											vec4(1, 1, 0.9, 1.0),\n\
											vec4(0.9, 0.9, 1, 1.0),\n\
											vec4(1, 0.9, 0.9, 1.0));\n\
			color = colors[gl_PrimitiveID];\n\
			}" };


		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint geom_shader;
		GLuint program;

		vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, vertex_shader_source, NULL);
		glCompileShader(vertex_shader);

		geom_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geom_shader, 1, geom_shader_source, NULL);
		glCompileShader(geom_shader);

		fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, fragment_shader_source, NULL);
		glCompileShader(fragment_shader);

		program = glCreateProgram();
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glAttachShader(program, geom_shader);
		glLinkProgram(program);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return program;
	}


	void  myInitCode(void) {

		myRenderProgram = myShaderCompile();
		//glCreateVertexArrays(1, &myVAO); //use this one on class pc
		glGenVertexArrays(1, &myVAO);		//Use this one on home pc
		glBindVertexArray(myVAO);

		glUseProgram(myRenderProgram);


	}



	glm::mat4 myMVP;

	//CODE THAT RENDERS STATTIONARY OBJECTS
	void myRenderCode(double currentTime, glm::vec4 pos) {

		glUseProgram(myRenderProgram);
		glm::mat4 rotation = glm::mat4(1);

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	//
	//CODE THAT RENDERS ROTATIONAL AND TRANSLATIONAL OBJECTS
	void myRenderCode(double currentTime, glm::vec4 pos, int axis) {

		glUseProgram(myRenderProgram);
		glm::mat4 rotation;
		glm::mat4 translation;

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);

		int IDAdd = 0;
		float offset = 0.01;


		if ((cos(currentTime) >= -offset * 2) && (cos(currentTime) <= offset * 10) || (cos(currentTime) > -1) && (cos(currentTime) <= (-1 + offset / 4)) || (cos(currentTime) >= (1 - offset / 4)) && (cos(currentTime) < 1)) {
			IDAdd = 6;
			//std::cout << cos(currentTime) <<std::endl;
		}
		else {
			IDAdd = 0;
		}

		glUniform1i(glGetUniformLocation(myRenderProgram, "IDAdd"), IDAdd);


		switch (axis) {

			//Rotation with X axis
		case 0:
			rotation = { 1.0f, 0.f, 0.f, 0.f,
				0.f, cos(currentTime), -sin(currentTime), 0.f,
				0.f, sin(currentTime), cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Y axis
		case 1:
			rotation = { cos(currentTime), 0.f, -sin(currentTime), 0.f,
				0.f, 1.f, 0.f, 0.f,
				sin(currentTime), 0.f, cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Z axis
		case 2:
			rotation = { cos(currentTime), -sin(currentTime), 0.f, 0.f,
				sin(currentTime), cos(currentTime), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;
		}


		//Translate the cubes
		translation = glm::translate(glm::mat4(1), glm::vec3(0.f, fmod(-currentTime, 10.f) + 5.f, 0.f));



		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP*translation));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}



	//CODE THAT RENDERS THE MATRIX
	void myRenderCodeMatrix(double currentTime, glm::vec4 pos, int axis, int speed) {
		glUseProgram(myRenderProgram);


		glm::mat4 rotation;
		glm::mat4 translation;

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);

		switch (axis) {

			//Rotation with X axis
		case 0:
			rotation = { 1.0f, 0.f, 0.f, 0.f,
				0.f, cos(currentTime), -sin(currentTime), 0.f,
				0.f, sin(currentTime), cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Y axis
		case 1:
			rotation = { cos(currentTime), 0.f, -sin(currentTime), 0.f,
				0.f, 1.f, 0.f, 0.f,
				sin(currentTime), 0.f, cos(currentTime), 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;

			//Rotation with Z axis
		case 2:
			rotation = { cos(currentTime), -sin(currentTime), 0.f, 0.f,
				sin(currentTime), cos(currentTime), 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f };
			break;
		}

		translation = glm::translate(glm::mat4(1), glm::vec3(0.f, (fmod(-currentTime, 10.f) + 5.f)*speed, 0.f));

		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP*translation));
		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));


		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

}

