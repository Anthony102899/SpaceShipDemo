/*
Student Information
Student ID:1155124488
Student Name: LYU An
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

#include "Shader.h"
#include "Texture.h"

#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <ctime>

// screen setting
const int SCR_WIDTH = 1600;
const int SCR_HEIGHT = 900;
Shader myShader;
GLuint vaoID[5];
GLuint vboID[5];
glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 endPoint = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ambientLight(0.3f, 0.3f, 0.3f);
glm::vec3 lightPosition(0.0f, 1.0f, 0.0f);
size_t size[5];
Texture trophy_texture[1];
Texture craft_texture[2];
Texture planet_texture[2];
Texture rock_texture[2];
Texture spacecraft_texture[2];
int craft_texture_flag = 0;
int trophy_translation_flag = 0;
int past = time((time_t*)NULL);
float light_delta = 0.1f;
float light_delta1 = 0.2f;

//struct for storing the movement of object
struct TimeCtl {

};
struct ViewInf {
	glm::vec3 viewPoint;
	glm::vec3 endPoint;
};
struct ObjCoordinate{
	glm::vec3 translation;
	GLfloat rotation;
};
struct MouseCtl{
	double x_start;
	double x_current;
	double sensitivity;
	float velocity;
	int move_direction; // (1 2 3 4) stand for (up down left right) respectively.
	double yaw;
	int press_flag;
};
MouseCtl mouse;
ObjCoordinate spaceshipCoordinate;
ObjCoordinate craftCoordinate1 = { glm::vec3(-1.5f, 0.0f, -2.0f), 0.0f };
ObjCoordinate craftCoordinate2;
ObjCoordinate craftCoordinate3;
ObjCoordinate trophyCoordinate = { glm::vec3(5.0f, 0.0f, -15.0f), 0.0f };


ViewInf windowView;
// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};


struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

void setView() {
	windowView.viewPoint.x = spaceshipCoordinate.translation.x - glm::sin(glm::radians(mouse.yaw)) * 5.0f;
	windowView.viewPoint.z = spaceshipCoordinate.translation.z + glm::cos(glm::radians(mouse.yaw)) * 5.0f;
	windowView.endPoint = spaceshipCoordinate.translation;
}
void updateMovement() {
	if (mouse.press_flag == 1) {
		if (mouse.move_direction == 1) {
			spaceshipCoordinate.translation.z -= mouse.velocity * glm::cos(glm::radians(mouse.yaw));
			spaceshipCoordinate.translation.x += mouse.velocity * glm::sin(glm::radians(mouse.yaw));
		}
		else if (mouse.move_direction == 2) {
			spaceshipCoordinate.translation.z += mouse.velocity * glm::cos(glm::radians(mouse.yaw));
			spaceshipCoordinate.translation.x -= mouse.velocity * glm::sin(glm::radians(mouse.yaw));
		}
		else if (mouse.move_direction == 3) {
			spaceshipCoordinate.translation.z -= mouse.velocity * glm::sin(glm::radians(mouse.yaw));
			spaceshipCoordinate.translation.x -= mouse.velocity * glm::cos(glm::radians(mouse.yaw));
		}
		else if (mouse.move_direction == 4) {
			spaceshipCoordinate.translation.z += mouse.velocity * glm::sin(glm::radians(mouse.yaw));
			spaceshipCoordinate.translation.x += mouse.velocity * glm::cos(glm::radians(mouse.yaw));
		}
		setView();
	}
}

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

void sendDataToOpenGL()
{
	Model trophy = loadOBJ("resources/trophy/trophy.obj");
	Model craft = loadOBJ("resources/craft/craft.obj");
	Model planet = loadOBJ("resources/planet/planet.obj");
	Model rock = loadOBJ("resources/rock/rock.obj");
	Model spacecraft = loadOBJ("resources/spacecraft/spacecraft.obj");
	craft_texture[0].setupTexture("resources/craft/ringTexture.png");
	craft_texture[1].setupTexture("resources/craft/Orange.png");
	planet_texture[0].setupTexture("resources/planet/earthTexture.png");
	planet_texture[1].setupTexture("resources/planet/earthNormal.png");
	rock_texture[0].setupTexture("resources/rock/rockTexture.png");
	rock_texture[1].setupTexture("resources/rock/gold.png");
	spacecraft_texture[0].setupTexture("resources/spacecraft/spacecraftTexture.png");
	spacecraft_texture[1].setupTexture("resources/spacecraft/gold.png");
	trophy_texture[0].setupTexture("resources/trophy/gold.png");

	/*
	Model 1 : Air Craft (enemies)
	*/
	glGenVertexArrays(5, vaoID);
	glBindVertexArray(vaoID[0]);

	glGenBuffers(1, &vboID[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, craft.vertices.size() * sizeof(Vertex), &craft.vertices[0], GL_STATIC_DRAW);
	
	GLuint ebo_craft;
	glGenBuffers(1, &ebo_craft);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_craft);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, craft.indices.size() * sizeof(unsigned int), &craft.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[0] = craft.indices.size();
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	/*
	Model 2 : Planet
	*/
	glBindVertexArray(vaoID[1]);

	glGenBuffers(1, &vboID[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
	glBufferData(GL_ARRAY_BUFFER, planet.vertices.size() * sizeof(Vertex), &planet.vertices[0], GL_STATIC_DRAW);

	GLuint ebo_planet;
	glGenBuffers(1, &ebo_planet);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_planet);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, planet.indices.size() * sizeof(unsigned int), &planet.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[1] = planet.indices.size();
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	/*
	Model 3 : Rock
	*/
	glBindVertexArray(vaoID[2]);

	glGenBuffers(1, &vboID[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[2]);
	glBufferData(GL_ARRAY_BUFFER, rock.vertices.size() * sizeof(Vertex), &rock.vertices[0], GL_STATIC_DRAW);

	GLuint ebo_rock;
	glGenBuffers(1, &ebo_rock);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_rock);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, rock.indices.size() * sizeof(unsigned int), &rock.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[2] = rock.indices.size();
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	/*
	Model 4 : Space Craft
	*/
	glBindVertexArray(vaoID[3]);

	glGenBuffers(1, &vboID[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[3]);
	glBufferData(GL_ARRAY_BUFFER, spacecraft.vertices.size() * sizeof(Vertex), &spacecraft.vertices[0], GL_STATIC_DRAW);

	GLuint ebo_spacecraft;
	glGenBuffers(1, &ebo_spacecraft);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_spacecraft);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, spacecraft.indices.size() * sizeof(unsigned int), &spacecraft.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[3] = spacecraft.indices.size();
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	/*
	Model 5 : Trophy
	*/
	glBindVertexArray(vaoID[4]);

	glGenBuffers(1, &vboID[4]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[4]);
	glBufferData(GL_ARRAY_BUFFER, trophy.vertices.size() * sizeof(Vertex), &trophy.vertices[0], GL_STATIC_DRAW);

	GLuint ebo_trophy;
	glGenBuffers(1, &ebo_trophy);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_trophy);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, trophy.indices.size() * sizeof(unsigned int), &trophy.indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	size[4] = trophy.indices.size();
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();
	/* initialize the view info */
	windowView.viewPoint = glm::vec3(0.0f, 1.5f, 5.0f);
	/* initialize the transformation of objects */
	spaceshipCoordinate.translation = glm::vec3(0.0f, 0.0f, 0.0f);
	windowView.endPoint = spaceshipCoordinate.translation;
	spaceshipCoordinate.rotation = 180.0f;
	/* Initialize the mouse control parameter */
	mouse.x_start = 800.0f;
	mouse.sensitivity = 0.1f;
	mouse.yaw = 0.0f;
	mouse.press_flag = 0;
	mouse.move_direction = 0;
	mouse.velocity = 0.05f;
	/* set the shader info */
	myShader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
	myShader.use();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void paintGL(void)  //always run
{
	int now = time((time_t*)NULL);
	glClearColor(0.2f, 0.2f, 0.4f, 0.5f); //specify the background color, this is just an example
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	// 	glBindVertexArray(vaoID[4]);
	//Set lighting information, such as position and color of lighting source
	myShader.setVec3("ambientLight", ambientLight);
	myShader.setVec3("lightPosition", lightPosition);
	myShader.setVec3("eyePositionWorld", windowView.viewPoint);

	//Set transformation matrix for space craft
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, spaceshipCoordinate.translation);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.001f, 0.001f, 0.001f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, glm::radians(spaceshipCoordinate.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);


	glm::mat4 view = glm::lookAt(windowView.viewPoint,
		windowView.endPoint,
		glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("view", view);
	int movable = 1;
	myShader.setInt("movable", movable);
	glm::mat4 projection = glm::perspective(glm::radians(30.0f), 1.78f, 0.1f, 50.0f);
	myShader.setMat4("projection", projection);
	/*
	spacecraft
	*/
	if (trophy_translation_flag == 0) spacecraft_texture[0].bind(0);
	if (trophy_translation_flag == 1) spacecraft_texture[1].bind(0);
	myShader.setInt("sampler1", 0);
	glBindVertexArray(vaoID[3]);
	glDrawElements(GL_TRIANGLES, size[3], GL_UNSIGNED_INT, 0);
	/*
	planet
	*/
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(1.5f, 0.0f, -15.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, 0.3f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);

	planet_texture[0].bind(0);
	myShader.setInt("sampler1", 0);
	glBindVertexArray(vaoID[1]);
	glDrawElements(GL_TRIANGLES, size[1], GL_UNSIGNED_INT, 0);
	/*
	craft
	*/
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, craftCoordinate1.translation);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, 0.5f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);
	if (abs(craftCoordinate1.translation.x - spaceshipCoordinate.translation.x) < 1.3f && abs(craftCoordinate1.translation.z - spaceshipCoordinate.translation.z) < 0.6f) craft_texture_flag = 1;
	if (craft_texture_flag == 0) craft_texture[0].bind(0);
	if (craft_texture_flag == 1 && now - past >= 1) {
		int i = now % 2;
		craft_texture[i].bind(0);
	}
	myShader.setInt("sampler1", 0);
	glBindVertexArray(vaoID[0]);
	glDrawElements(GL_TRIANGLES, size[0], GL_UNSIGNED_INT, 0);
	/*
	trophy
	*/
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, trophyCoordinate.translation);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);
	trophy_texture[0].bind(0);
	if (abs(trophyCoordinate.translation.x - spaceshipCoordinate.translation.x) < 1.3f && abs(trophyCoordinate.translation.z - spaceshipCoordinate.translation.z) < 0.6f) trophy_translation_flag = 1;
	if (trophy_translation_flag == 1) trophyCoordinate.translation += glm::vec3(0.0f, 999.0f, 0.0f);
	myShader.setInt("sampler1", 0);
	glBindVertexArray(vaoID[4]);
	glDrawElements(GL_TRIANGLES, size[4], GL_UNSIGNED_INT, 0);
	/*
	light
	*/
	glm::vec4 ambientLight = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
	myShader.setVec4("ambientLight", ambientLight);
	glm::vec3 lightPositionWorld1 = glm::vec3(1.5f, 0.0f, -15.0f);
	myShader.setVec3("lightPositionWorld1", lightPositionWorld1);
	glm::vec3 lightcolor1(0.0f, 2.0f, 2.0f);
	myShader.setVec3("lightcolor1", lightcolor1);
	glm::vec3 lightPositionWorld2 = glm::vec3(5.0f, 0.0f, -15.0f);
	myShader.setVec3("lightPositionWorld2", lightPositionWorld2);
	glm::vec3 lightcolor2(1.0f, 0.843f, 0.0f);
	myShader.setVec3("lightcolor2", lightcolor2);
	myShader.setFloat("delta", light_delta);
	myShader.setFloat("delta1", light_delta1);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}
void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	double xoffset = x - mouse.x_start;
	xoffset *= mouse.sensitivity;
	mouse.x_start = x;
	mouse.yaw += xoffset;
	spaceshipCoordinate.rotation =  180 - mouse.yaw;
	setView();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Sets the scoll callback for the current window.

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Sets the Keyboard callback for the current window.
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		mouse.press_flag = 1;
		mouse.move_direction = 1;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		mouse.press_flag = 1;
		mouse.move_direction = 2;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		mouse.press_flag = 1;
		mouse.move_direction = 3;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		mouse.press_flag = 1;
		mouse.move_direction = 4;
	}
	if ((key == GLFW_KEY_UP || key == GLFW_KEY_DOWN || key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT) && action == GLFW_RELEASE) {
		mouse.press_flag = 0;
		mouse.move_direction = 0;
	}
	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		if (mouse.velocity == 0.2f) {
			mouse.velocity = 0.05f;
		}
		else {
			mouse.velocity = 0.2f;
		}
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		light_delta = light_delta + 0.02f;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		light_delta = light_delta - 0.02f;
	}
}


int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	initializedGL();

	double lastTime = glfwGetTime();
	double deltaTime = 0, nowTime = 0;
	static double INTERVAL = 0.0167f;
	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		nowTime = glfwGetTime();
		deltaTime = nowTime - lastTime;
		if (deltaTime >= INTERVAL) {
			lastTime = nowTime;
			updateMovement();
		}
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






