
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <cstdio>
#include <cassert>

#include "GL_framework.h"

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

glm::vec4 myPalette[3] = {

	//Peter river
	{0.2f, 0.59f, 0.85f, 1.f},

	//Concrete
	//{0.58f, 0.66f, 0.65, 1.f},

	//Alizarin
	{0.9f, 0.29f, 0.24f, 1.f},

	//Emerald
	{ 0.18f, 0.8f, 0.44f, 1.f },

	//
};




namespace MyFirstShader {

	void myInitCode(void);
	GLuint myShaderCompile(void);

	void myCleanupCode(void);
	void myRenderCode(double currentTime);

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

void myGUI() {
	bool show = true;
	ImGui::Begin("Simulation Parameters", &show, 0);

	// Do your GUI code here....
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate

	}
	// .........................

	ImGui::End();

}

void myInitCode(int width, int height) {

	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	MyFirstShader::myInitCode();
}


void myRenderCode(double currentTime) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;
	MyFirstShader::myRenderCode(currentTime);

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


/////////////////////////////////////MY DUPLICATION SHADER
namespace MyFirstShader {
	void myCleanupCode() {
		glDeleteVertexArrays(1, &myVAO);
		glDeleteProgram(myRenderProgram);
	}


	GLuint myShaderCompile(void) {



		//static const GLchar * vertex_shader_source[] = //vec4( 0.25, -0.25, 0.5, 1.0) ... // = { 0.25, -0.25, 0.5, 1.0}
		//{
		//	"#version 330										\n\
				//	\n\
		//	uniform vec4 inOne;\n\
		//	void main() {\n\
		//	const vec4 vertices[3] = vec4[3](vec4( inOne.x,  inOne.y,  inOne.z, 1.0),\n\
		//								     vec4(0.25, 0.25, 0.5, 1.0),\n\
		//									 vec4( -0.25,  -0.25, 0.5, 1.0));\n\
		//	gl_Position = vertices[gl_VertexID];\n\
		//	}"
//};

		static const GLchar * vertex_shader_source[] =
		{
			"#version 330										\n\
		\n\
		void main() {\n\
		const vec4 vertices[3] = vec4[3](vec4( 0.25, -0.25, 0.5, 1.0),\n\
									   vec4(0.25, 0.25, 0.5, 1.0),\n\
										vec4( -0.25,  -0.25, 0.5, 1.0));\n\
		gl_Position = vertices[gl_VertexID];\n\
		}" };

		static const GLchar * geom_shader_source[] = {
			"#version 330 \n\
			uniform mat4 rotation;\n\
			uniform vec4 inOne;\n\
			uniform vec4 inTwo;\n\
			layout(triangles) in;\n\
			layout(triangle_strip, max_vertices = 24) out;\n\
			void main()\n\
			{\n\
				const vec4 vertices[4] = vec4[4](vec4(0.25, -0.25, 0.25, 1.0),\n\
										vec4(0.25, 0.25, 0.25, 1.0),\n\
										vec4(-0.25, -0.25, 0.25, 1.0),\n\
										vec4(-0.25, 0.25, 0.25, 1.0));\n\
				\n\
				//CARA 1\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*vertices[i]+gl_in[0].gl_Position+inOne;\n\
					gl_PrimitiveID = 0;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				\n\
				//CARA 2\n\
				const vec4 vertices2[4]= vec4[4](vec4(0.25, 0.25, 0.25, 1.0),\n\
										vec4(0.25, 0.25, -0.25, 1.0),\n\
										vec4(-0.25, 0.25, 0.25, 1.0),\n\
										vec4(-0.25, 0.25, -0.25, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*vertices2[i]+gl_in[0].gl_Position+inOne;\n\
					gl_PrimitiveID = 1;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 3\n\
				const vec4 vertices3[4]= vec4[4](vec4(-0.25, -0.25, 0.25, 1.0),\n\
										vec4(-0.25, 0.25, 0.25, 1.0),\n\
										vec4(-0.25, -0.25, -0.25, 1.0),\n\
										vec4(-0.25, 0.25, -0.25, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*vertices3[i]+gl_in[0].gl_Position+inOne;\n\
					gl_PrimitiveID = 2;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 4\n\
				const vec4 vertices4[4]= vec4[4](vec4(-0.25, -0.25, -0.25, 1.0),\n\
										vec4(-0.25, 0.25, -0.25, 1.0),\n\
										vec4(0.25, -0.25, -0.25, 1.0),\n\
										vec4(0.25, 0.25, -0.25, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*vertices4[i]+gl_in[0].gl_Position+inOne;\n\
					gl_PrimitiveID = 3;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 5\n\
				const vec4 vertices5[4]= vec4[4](vec4(-0.25, -0.25, 0.25, 1.0),\n\
										vec4(-0.25, -0.25, -0.25, 1.0),\n\
										vec4(0.25, -0.25, 0.25, 1.0),\n\
										vec4(0.25, -0.25, -0.25, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*vertices5[i]+gl_in[0].gl_Position+inOne;\n\
					gl_PrimitiveID = 4;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
				//CARA 6\n\
				const vec4 vertices6[4]= vec4[4](vec4(0.25, -0.25, -0.25, 1.0),\n\
										vec4(0.25, 0.25, -0.25, 1.0),\n\
										vec4(0.25, -0.25, 0.25, 1.0),\n\
										vec4(0.25, 0.25, 0.25, 1.0));\n\
				for (int i = 0; i<4; i++)\n\
				{\n\
					gl_Position = rotation*vertices6[i]+gl_in[0].gl_Position+inOne;\n\
					gl_PrimitiveID = 5;\n\
					EmitVertex();\n\
				}\n\
				EndPrimitive();\n\
\n\
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
			color = colors[gl_PrimitiveID ];\n\
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



	glm::vec4 GetRandomPoint() {
		return glm::vec4(rand() % 5, rand() % 5, rand() % 5, 1/*0.25, -0.25, 0.5, 1.0*/);
	}


	void  myInitCode(void) {

		//glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "inOne"), 1, GL_FALSE, glm::value_ptr(tmp));
		myRenderProgram = myShaderCompile();
		//glCreateVertexArrays(1, &myVAO); //use this one on class pc
		glGenVertexArrays(1, &myVAO);		//Use this one on home pc
		glBindVertexArray(myVAO);

		glUseProgram(myRenderProgram);


		glm::vec4 tmp = glm::vec4(0,0,0, 1.0);//GetRandomPoint();
		GLint loc = glGetUniformLocation(myRenderProgram, "inOne");
		glUniform4f(loc, tmp.x, tmp.y, tmp.z, 1);

		//glm::vec4 tmp2 = glm::vec4(1.25, -2.25, 0.5, 1.0);//GetRandomPoint();
		//GLint loc2 = glGetUniformLocation(myRenderProgram, "inTwo");
		//glUniform4f(loc2, tmp2.x, tmp2.y, tmp2.z, 1);

	}



	glm::mat4 myMVP;
	void myRenderCode(double currentTime) {

		glUseProgram(myRenderProgram);
		glm::mat4 rotation = { cos(currentTime), 0.f, -sin(currentTime), 0.f,
			0.f, 1.f, 0.f, 0.f,
			sin(currentTime), 0.f, cos(currentTime), 0.f,
			0.f, 0.f, 0.f, 1.f };
		glUniformMatrix4fv(glGetUniformLocation(myRenderProgram, "rotation"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));




		glDrawArrays(GL_TRIANGLES, 0, 3);

	}



}

