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

#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// screen setting
const int SCR_WIDTH = 1000;
const int SCR_HEIGHT = 1000;
Shader myShader;
GLuint vaoID[4];
GLuint vboID[4];
GLfloat rotation_y = 0.0f;
glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 endPoint = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ambientLight(0.3f, 0.3f, 0.3f);
glm::vec3 lightPosition(0.0f, 1.0f, 0.0f);
size_t size[4];
Texture craft_texture;
Texture planet_texture_1;
Texture planet_texture_2;
Texture rock_texture_1;
Texture rock_texture_2;
Texture spacecraft_texture_1;
Texture spacecraft_texture_2;
GLuint slot_1 = 0;
GLuint slot_2 = 0;
//struct for storing the movement of object
struct ViewInf {
	glm::vec3 viewPoint;
	glm::vec3 endPoint;
};

struct ObjCoordinate{
	glm::vec3 translation;
	GLfloat rotation;
};
ObjCoordinate spaceshipCoordinate;

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
	//TODO
	//Load objects and bind to VAO and VBO
	Model craft = loadOBJ("resources/craft/craft.obj");
	Model planet = loadOBJ("resources/planet/planet.obj");
	Model rock = loadOBJ("resources/rock/rock.obj");
	Model spacecraft = loadOBJ("resources/spacecraft/spacecraft.obj");
	craft_texture.setupTexture("resources/craft/ringTexture.png");
	planet_texture_1.setupTexture("resources/planet/earthTexture.png");
	planet_texture_2.setupTexture("resources/planet/earthNormal.png");
	rock_texture_1.setupTexture("resources/rock/rockTexture.png");
	rock_texture_2.setupTexture("resources/rock/gold.png");
	spacecraft_texture_1.setupTexture("resources/spacecraft/spacecraftTexture.png");
	spacecraft_texture_2.setupTexture("resources/spacecraft/gold.png");

	/*
	Model 1 : Air Craft (enemies)
	*/
	glGenVertexArrays(4, vaoID);
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
}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();
	// Initialize the view infomation and object's coordinate at beginning.
	windowView.viewPoint = glm::vec3(0.0f, 3.0f, 2.0f);
	windowView.endPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	spaceshipCoordinate.translation = glm::vec3(0.0f, 0.0f, 1.0f);
	spaceshipCoordinate.rotation = 0.0f;
	myShader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
	myShader.use();

	//TODO: set up the camera parameters	
	//TODO: set up the vertex shader and fragment shader

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void paintGL(void)  //always run
{
	glClearColor(0.2f, 0.2f, 0.4f, 0.5f); //specify the background color, this is just an example
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	// 	glBindVertexArray(vaoID[4]);
	//Set lighting information, such as position and color of lighting source
	myShader.setVec3("ambientLight", ambientLight);
	myShader.setVec3("lightPosition", lightPosition);
	glm::vec3 eyePosition(0.0f, 0.0f, 0.2f);
	myShader.setVec3("eyePositionWorld", eyePosition);

	//Set transformation matrix for space craft
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	modelTransformMatrix = glm::translate(modelTransformMatrix, spaceshipCoordinate.translation);
	modelTransformMatrix = glm::rotate(modelTransformMatrix, glm::radians(spaceshipCoordinate.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);


	glm::mat4 view = glm::lookAt(windowView.viewPoint,
		windowView.endPoint,
		glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("view", view);
	int movable = 1;
	myShader.setInt("movable", movable);
	//Bind different textures
	glm::mat4 projection = glm::perspective(glm::radians(30.0f), 1.0f, 0.1f, 20.0f);
	myShader.setMat4("projection", projection);
	int independent = 0;
	myShader.setInt("independent", independent);



	
}
float xstart = 400.0f, xoffset;
float MouseSensitivity = 0.1f, yaw = 90.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	xoffset = x - xstart;
	xoffset *= MouseSensitivity;
	xstart = x;
	glm::vec3 shift;
	GLfloat theta = glm::radians(yaw);
	spaceshipCoordinate.rotation += theta;
	shift.x = glm::sin(glm::radians(spaceshipCoordinate.rotation));
	shift.z = glm::cos(glm::radians(spaceshipCoordinate.rotation));
	windowView.viewPoint.x = spaceshipCoordinate.translation.x - shift.x;
	windowView.viewPoint.z = spaceshipCoordinate.translation.z + shift.z;
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

	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






