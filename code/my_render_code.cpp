
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <cstdio>
#include <cassert>

#include "GL_framework.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

#include <iostream>

#define SCENETIME 10

//Auxiliar variables used to set the projection of the camera
int display_w, display_h;

//Scene to display: values are changed through GUI (radio buttons)
int scene = 0;

//Vector array of colors: Color palette
glm::vec4 myPalette[5] = {

	//Peter river
	{0.2f, 0.59f, 0.85f, 1.f},

	//Concrete
	{0.58f, 0.66f, 0.65, 1.f},

	//Alizarin
	{0.9f, 0.29f, 0.24f, 1.f},

	//Emerald
	{ 0.18f, 0.8f, 0.44f, 1.f },

	//#2c3e50 -> rgb 44 62 80
	{ 0.17f, 0.24f, 0.31f, 1.f}

};

//Reset scenes
bool scene_one_reset = true;

//Namespace to draw the cubes: used to draw the city buildings and planes
namespace Cube {
	void setupCube();
	void cleanupCube();
	void updateCube(const glm::mat4& transform);
	void drawCube();
	void drawCity(double currentTime);
	void updateColor(const glm::vec4 newColor);
}

namespace ImGui {
	void Render();
}


namespace RenderVars {
	const float FOV = glm::radians(65.f);
	const float zNear = 1.f;
	const float zFar = 50.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
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

void GUI() {
	bool show = true;
	ImGui::Begin("Simulation Parameters", &show, 0);

	// Do your GUI code here....
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

		//Changes to scene 1: Lateral Travelling
		ImGui::RadioButton("Exercisi 1: Travelling Lateral", &scene, 0); ImGui::SameLine;

		//Changes to scene 2: Zoom-in or close up
		ImGui::RadioButton("Exercisi 2.a: Close-up", &scene, 1); ImGui::SameLine;

		//Changes to scene 3: FOV (field of view) modification
		ImGui::RadioButton("Exercisi 2.a: FOV", &scene, 2); ImGui::SameLine;

		//Changes to scene 4: Vertigo effect: combination of scenes 2 & 3
		ImGui::RadioButton("Exercisi 3: Inverse dolly effect", &scene, 3);
	}
	// .........................

	ImGui::End();

}


void myInitCode(int width, int height) {

	//Set the auxiliar variables to the function-passed paramenters
	display_w = width;
	display_h = height;

	glViewport(0, 0, width, height);
	glClearColor(myPalette[0].r, myPalette[0].g, myPalette[0].b, myPalette[0].a);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//Establish our default camera to have a perspective projection
	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	Cube::setupCube();
}


void myRenderCode(double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Scene 1: Lateral Travelling
	if (scene == 0) {

		//Auxiliar variable for orthonormal projection distance
		int aux = 100;

		//Sets the camera projection to orthonormal projection
		RV::_projection = RV::_projection = glm::ortho((float)(-display_w / aux), (float)(display_w / aux), (float)(-display_h / aux), (float)(display_h / aux), 0.01f, 100.f);
		
		//Fixes the position of the camera
		if (scene_one_reset) {
			RV::_modelView = glm::rotate(RV::_modelView, glm::radians(30.f), glm::vec3(0.f, 1.f, 0.f));
			scene_one_reset = false;
		}
		//Moves the camera from left to right
		glm::mat4 travelling = glm::translate(glm::mat4(1.0f), glm::vec3(5-fmod(currentTime, SCENETIME), 0.f, -7.f));

		//Sets the MVP to the multiplication of the projection matrix by the lateral travelling translation
		RV::_MVP = RV::_projection * RV::_modelView * travelling;
	}

	//Scene 2a: Close up
	else if (scene == 1) {

		//Sets the camera projection to perspective projection
		RV::_projection = glm::perspective(RV::FOV, (float)display_w / (float)display_h, RV::zNear, RV::zFar);

		//Value of closup
		float closeup_val = -10 + fmod(currentTime, SCENETIME / 2);

		//Moves the camera from front to back
		glm::mat4 closeup = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, closeup_val));

		//Sets the MVP to the multiplication of the projection matrix by the lateral travelling translation
		RV::_MVP = RV::_projection * closeup;

		//Resets other scenes
		scene_one_reset = true;
	}

	//Scene 2b: FOV
	else if (scene == 2) {

		//Sets the position of the camera
		RV::_modelView = glm::lookAt(glm::vec3(0, 0, 7), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

		//Changes the fov values over time
		float fov = RV::FOV + glm::radians(fmod((currentTime*SCENETIME), 5*SCENETIME));

		//Moves the camera from closed FOV to openned FOV
		RV::_projection = glm::perspective(fov, (float)display_w / (float)display_h, RV::zNear, RV::zFar);

		//Sets the MVP to the multiplication of the projection matrix (in this case the FOV mod) by the lookat matrix
		RV::_MVP = RV::_projection * RV::_modelView;

		//Resets other scenes
		scene_one_reset = true;

	}

	//Scene 3: Reverse dolly effect
	else if (scene == 3) {

		//Sets the position of the camera
		RV::_modelView = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		//Changes the fov values over time
		float fov = RV::FOV + glm::radians(fmod((currentTime*SCENETIME), 5 * SCENETIME));

		//Value of closup
		float closeup_val = -6.5 + fmod(currentTime, SCENETIME / 2);

		//Moves the camera from closed FOV to openned FOV
		RV::_projection = glm::perspective(fov, (float)display_w / (float)display_h, RV::zNear, RV::zFar);
		glm::mat4 tra = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, closeup_val));


		RV::_MVP = RV::_projection * RV::_modelView * tra;


		//Resets other scenes
		scene_one_reset = true;
	}

	Cube::drawCity(currentTime);
	ImGui::Render();
}

void myCleanupCode(void) {
	Cube::cleanupCube();
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


////////////////////////////////////////////////// CUBE
namespace Cube {
	GLuint cubeVao;
	GLuint cubeVbo[3];
	GLuint cubeShaders[2];
	GLuint cubeProgram;

	//Matriz de transformacion
	glm::mat4 objMat = glm::mat4(1.f);


	int next = 5;

	extern const float halfW = 0.5f;
	int numVerts = 24 + 6; // 4 vertex/face * 6 faces + 6 PRIMITIVE RESTART

						   //   4---------7
						   //  /|        /|
						   // / |       / |
						   //5---------6  |
						   //|  0------|--3
						   //| /       | /
						   //|/        |/
						   //1---------2
	glm::vec3 verts[] = {
		glm::vec3(-halfW, -halfW, -halfW),
		glm::vec3(-halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW,  halfW),
		glm::vec3(halfW, -halfW, -halfW),
		glm::vec3(-halfW,  halfW, -halfW),
		glm::vec3(-halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW,  halfW),
		glm::vec3(halfW,  halfW, -halfW)
	};
	glm::vec3 norms[] = {
		glm::vec3(0.f, -1.f,  0.f),
		glm::vec3(0.f,  1.f,  0.f),
		glm::vec3(-1.f,  0.f,  0.f),
		glm::vec3(1.f,  0.f,  0.f),
		glm::vec3(0.f,  0.f, -1.f),
		glm::vec3(0.f,  0.f,  1.f)
	};

	glm::vec3 cubeVerts[] = {
		verts[1], verts[0], verts[2], verts[3],
		verts[5], verts[6], verts[4], verts[7],
		verts[1], verts[5], verts[0], verts[4],
		verts[2], verts[3], verts[6], verts[7],
		verts[0], verts[4], verts[3], verts[7],
		verts[1], verts[2], verts[5], verts[6]
	};
	glm::vec3 cubeNorms[] = {
		norms[0], norms[0], norms[0], norms[0],
		norms[1], norms[1], norms[1], norms[1],
		norms[2], norms[2], norms[2], norms[2],
		norms[3], norms[3], norms[3], norms[3],
		norms[4], norms[4], norms[4], norms[4],
		norms[5], norms[5], norms[5], norms[5]
	};
	GLubyte cubeIdx[] = {
		0, 1, 2, 3, UCHAR_MAX,
		4, 5, 6, 7, UCHAR_MAX,
		8, 9, 10, 11, UCHAR_MAX,
		12, 13, 14, 15, UCHAR_MAX,
		16, 17, 18, 19, UCHAR_MAX,
		20, 21, 22, 23, UCHAR_MAX
	};




	const char* cube_vertShader =
		"#version 330\n\
	in vec3 in_Position;\n\
	in vec3 in_Normal;\n\
	out vec4 vert_Normal;\n\
	uniform mat4 objMat;\n\
	uniform mat4 mv_Mat;\n\
	uniform mat4 mvpMat;\n\
	void main() {\n\
		gl_Position = mvpMat * objMat * vec4(in_Position, 1.0);\n\
		vert_Normal = mv_Mat * objMat * vec4(in_Normal, 0.0);\n\
	}";


	const char* cube_fragShader =
		"#version 330\n\
			in vec4 vert_Normal;\n\
			out vec4 out_Color;\n\
			uniform mat4 mv_Mat;\n\
			uniform vec4 color;\n\
			void main() {\n\
		out_Color = vec4(color.xyz * dot(vert_Normal, mv_Mat*vec4(0.0, 1.0, 0.2, 0.0)) + color.xyz * 0.3, 1.0 );\n\
}";
	void setupCube() {
		glGenVertexArrays(1, &cubeVao);
		glBindVertexArray(cubeVao);
		glGenBuffers(3, cubeVbo);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNorms), cubeNorms, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glPrimitiveRestartIndex(UCHAR_MAX);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cubeShaders[0] = compileShader(cube_vertShader, GL_VERTEX_SHADER, "cubeVert");
		cubeShaders[1] = compileShader(cube_fragShader, GL_FRAGMENT_SHADER, "cubeFrag");

		cubeProgram = glCreateProgram();
		glAttachShader(cubeProgram, cubeShaders[0]);
		glAttachShader(cubeProgram, cubeShaders[1]);
		glBindAttribLocation(cubeProgram, 0, "in_Position");
		glBindAttribLocation(cubeProgram, 1, "in_Normal");
		linkProgram(cubeProgram);
	}
	void cleanupCube() {
		glDeleteBuffers(3, cubeVbo);
		glDeleteVertexArrays(1, &cubeVao);

		glDeleteProgram(cubeProgram);
		glDeleteShader(cubeShaders[0]);
		glDeleteShader(cubeShaders[1]);
	}
	void updateCube(const glm::mat4& transform) {
		objMat = transform;
	}
	void drawCube() {
		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), 0.1f, 1.f, 1.f, 0.f);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);
	}
	void drawCity(double currentTime) {

		glEnable(GL_PRIMITIVE_RESTART);
		glBindVertexArray(cubeVao);
		glUseProgram(cubeProgram);

		glm::mat4 t;
		glm::mat4 r;
		glm::mat4 s;


		//Floor
		t = glm::translate(glm::mat4(1.0f), glm::vec3(0, -2.f, 0.f));
		s = glm::scale(glm::mat4(1.0f), glm::vec3(20.f, .1f, 20.f));

		objMat = t*s;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mv_Mat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_modelView));
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RenderVars::_MVP));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[4].r, myPalette[4].g, myPalette[4].b, myPalette[4].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);



		#pragma region "Back row"
		//Edificio 0
		t = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.f, -2.f));
		s = glm::scale(glm::mat4(1.0f), glm::vec3(1.f, 4.f, 1.f));

		objMat = t*s;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//Edificio 1
		t = glm::translate(glm::mat4(1.0f), glm::vec3(2, 0.f, -2.f));
		objMat = t*s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//Edificio 2
		t = glm::translate(glm::mat4(1.0f), glm::vec3(4, 0.f, -2.f));
		objMat = t*s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//Edificio -1
		t = glm::translate(glm::mat4(1.0f), glm::vec3(-2, 0.f, -2.f));
		objMat = t*s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//Edificio -2
		t = glm::translate(glm::mat4(1.0f), glm::vec3(-4, 0.f, -2.f));
		objMat = t*s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		#pragma endregion

		#pragma region "Mid row"
		//Edificio 0
		t = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.f, 0.f));
		s = glm::scale(glm::mat4(1.0f), glm::vec3(1.f, 2.f, 1.f));

		objMat = t*s;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//Edificio 1
		t = glm::translate(glm::mat4(1.0f), glm::vec3(2, -1.f, 0.f));
		objMat = t*s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//Edificio 2
		t = glm::translate(glm::mat4(1.0f), glm::vec3(4, -1.f, 0.f));
		objMat = t*s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//Edificio -1
		t = glm::translate(glm::mat4(1.0f), glm::vec3(-2, -1.f, 0.f));
		objMat = t*s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);

		//Edificio -2
		t = glm::translate(glm::mat4(1.0f), glm::vec3(-4, -1.f, 0.f));
		objMat = t*s;
		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[1].r, myPalette[1].g, myPalette[1].b, myPalette[1].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		#pragma endregion

		//Edificio 0
		t = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.f, 5.f));
		s = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));

		objMat = t * s;

		glUniformMatrix4fv(glGetUniformLocation(cubeProgram, "objMat"), 1, GL_FALSE, glm::value_ptr(objMat));
		glUniform4f(glGetUniformLocation(cubeProgram, "color"), myPalette[3].r, myPalette[3].g, myPalette[3].b, myPalette[3].a);
		glDrawElements(GL_TRIANGLE_STRIP, numVerts, GL_UNSIGNED_BYTE, 0);
		


		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);



	}


	void updateColor(const glm::vec4 newColor) {

		//myColor = newColor;
	}
}
