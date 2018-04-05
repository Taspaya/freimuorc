
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <cstdio>
#include <cassert>
#include <iostream>
#include "GL_framework.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

#define MAX_CUBES 4
#define EXERCISE_NUM 3
glm::vec4 randomPositions[MAX_CUBES] = {glm::vec4(0,0,0,1),glm::vec4(2,2,0,1), glm::vec4(4,0,0,1), glm::vec4(2,-2,0,1) };

int ex = 0;
bool update[EXERCISE_NUM];

namespace MyFirstShader {

	void myInitCode(void);
	GLuint myShaderCompile(void);

	void myCleanupCode(void);
	void myRenderCode(double currentTime, glm::vec4 pos, bool rotate);

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

	}
	// .........................

	ImGui::End();

}


glm::vec4 GetRandomPoint() {
	return glm::vec4(rand()%20, rand() % 20, rand() % 20, 1.0);
}

void myInitCode(int width, int height) {

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

	//int aux = 200;
	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar); 
	//RV::_projection = glm::ortho((float)(-width / aux), (float)(width / aux), (float)(-height / aux), (float)(height / aux), 0.01f, 100.f);

	MyFirstShader::myInitCode();
}


void myRenderCode(double currentTime) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;


	//SWITCH STATEMENT: to check exercises
	switch (ex) {

	//EXERCISE 1
	case 0:
		//Reset of exercises
		if (update[0]) {
			for (int i = 0; i < MAX_CUBES; i++) {
				randomPositions[i] = GetRandomPoint();
			}

			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[0] = false;
		}
		for (int i = 0; i < MAX_CUBES; i++) {
			MyFirstShader::myRenderCode(currentTime, randomPositions[i], false);
		}
		break;


	//EXERCISE 2
	case 1:
		//Reset of exercises
		if (update[1]){
				randomPositions[0] = glm::vec4(0, 0, 0, 1);
				randomPositions[1] = glm::vec4(2, 2, 0, 1);
				randomPositions[2] = glm::vec4(4, 0, 0, 1);
				randomPositions[3] = glm::vec4(2, -2, 0, 1);
				for (int i = 0; i < EXERCISE_NUM; i++) {
					update[i] = true;
				}
				update[1] = false;
		}
		for (int i = 0; i < MAX_CUBES; i++) {
			MyFirstShader::myRenderCode(currentTime, randomPositions[i], false);
		}
		break;

	//EXERCISE 3
	case 2: 
		//Reset of exercises
		if (update[2]) {
			for (int i = 0; i < MAX_CUBES; i++) {
				randomPositions[i] = GetRandomPoint();
			}
			for (int i = 0; i < EXERCISE_NUM; i++) {
				update[i] = true;
			}
			update[2] = false;
		}
		for (int i = 0; i < MAX_CUBES; i++) {
			MyFirstShader::myRenderCode(currentTime, randomPositions[i], true);
		}
		
		break;



	default:
		break;
	}


	


	

	

	ImGui::Render();
}

void myCleanupCode(void) {
	MyFirstShader::myCleanupCode();
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
*		SHADER DE LOS COJONES
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


		/*const vec4 inCenters[2] = vec4[2](vec4(inOne.x, inOne.y, inOne.z, 1), \n\
			vec4(inTwo.x, inTwo.y, inTwo.z, 1)); \n\*/
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
	void myRenderCode(double currentTime, glm::vec4 pos, bool rotate) {

		glUseProgram(myRenderProgram);
		glm::mat4 rotation; 
		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP/*rotation*/));
	

		glm::vec4 tmp = GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, pos.x, pos.y, pos.z, 1);

		if (rotate) {
			rotation = { cos(currentTime), 0.f, -sin(currentTime), 0.f,
						0.f, 1.f, 0.f, 0.f,
						sin(currentTime), 0.f, cos(currentTime), 0.f,
						0.f, 0.f, 0.f, 1.f };
		}
		else {
			rotation = glm::mat4(1);
		}
		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "selfRot"), 1, GL_FALSE, glm::value_ptr(rotation));

		glDrawArrays(GL_TRIANGLES, 0, 3);
	}



}

