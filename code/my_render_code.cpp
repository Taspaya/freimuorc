
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <cstdio>
#include <cassert>

#include "GL_framework.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>


int display_w, display_h;

bool reset_escena_uno = true;
bool reset_escena_dos = true;
bool reset_escena_tres = true;



int scene = 0;

//Colors
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

//PENSAR SI SE INCLUYEN TODAS O NO
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

void myGUI() {
	bool show = true;
	ImGui::Begin("Simulation Parameters", &show, 0);

	// Do your GUI code here....
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		ImGui::RadioButton("Escena 1", &scene, 0); ImGui::SameLine;
		ImGui::RadioButton("Escena 2", &scene, 1); ImGui::SameLine;
		ImGui::RadioButton("Escena 3", &scene, 2); 
	}
	// .........................

	ImGui::End();

}


void myInitCode(int width, int height) {

	display_w = width;
	display_h = height;

	glViewport(0, 0, width, height);
	glClearColor(myPalette[0].r, myPalette[0].g, myPalette[0].b, myPalette[0].a);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	int aux = 100;
	

	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	//RV::_projection = glm::ortho((float)(-width / aux), (float)(width / aux), (float)(-height / aux), (float)(height / aux), 0.01f, 100.f);

	Cube::setupCube();
}


void myRenderCode(double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	if (scene == 0) {			//Travelling lateral				CAUTION: should be in orthonormal projection

		if (reset_escena_uno) {

			reset_escena_uno = false;
			reset_escena_dos = true;
			reset_escena_tres = true;

			//Resetear variables

		}

		
		RV::_modelView = glm::lookAt(glm::vec3(0, 5, 10), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
		RV::_modelView = glm::translate(glm::mat4(1.0f), glm::vec3(5-(int)(currentTime*100)%1000*0.01f, 0.f, -7.f));    //kill me plz   

		RV::_MVP = RV::_projection * RV::_modelView;

	}
	else if (scene == 1) {

		if (reset_escena_dos) {

			reset_escena_uno = true;
			reset_escena_dos = false;
			reset_escena_tres = true;

			//Resetear variables

		}

		//Código parte dos: Close up
		RV::_modelView = glm::lookAt(glm::vec3(0, 5, 10), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
		RV::_modelView = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, -15 + (int)(currentTime * 100) % 1000 * 0.01f));    //kill me plz   

		RV::_MVP = RV::_projection * RV::_modelView;

	}
	else if (scene == 2) {
		if (reset_escena_tres) {

			reset_escena_uno = true;
			reset_escena_dos = true;
			reset_escena_tres = false;

			//Resetear variables
		}

		//Código parte tres: Modifies the FOV
		RV::_modelView = glm::lookAt(glm::vec3(0, 5, 15), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
		RV::_projection = glm::perspective(glm::radians(65 + 30*(float)sin(currentTime)), (float)display_w / (float)display_h, RV::zNear, RV::zFar);

		RV::_MVP = RV::_projection * RV::_modelView;

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





		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_PRIMITIVE_RESTART);


		
	}


	void updateColor(const glm::vec4 newColor) {

		//myColor = newColor;
	}
}