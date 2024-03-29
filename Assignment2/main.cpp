/*
Student Information
Student ID: 1155124469 1155124488
Student Name: SHAN Yuzhen ,LYU An
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"
#include "Dependencies/stb_image/stb_image.h"

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
Shader skyShader;
GLuint vaoID[5];
GLuint vboID[5];
GLuint vao_skybox;
GLuint vbo_skybox;
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
Texture skybox_texture[6];
int craft_texture_flag[3] = { 0 };
int trophy_translation_flag = 0;
int past = time((time_t*)NULL);
float light_delta = 0.3f;
float light_delta1 = 0.2f;
int angle[200];
float height[200];
float distance_to_planet[200];
float angle_z[200];
int isGold[200];
int Gold_paint_flag[200];
int direction[3] = { 1,0,1 };
double time_end;
double time_start;
unsigned int cubemapTexture;
std::vector<std::string> faces
{
	"resources/skybox/right.bmp",
	"resources/skybox/left.bmp",
	"resources/skybox/top.bmp",
	"resources/skybox/bottom.bmp",
	"resources/skybox/front.bmp",
	"resources/skybox/back.bmp"
};
//struct for storing the movement of object
struct TimeCtl {

};
struct ViewInf {
	glm::vec3 viewPoint;
	glm::vec3 endPoint;
};
struct ObjCoordinate {
	glm::vec3 translation;
	GLfloat rotation;
};
struct MouseCtl {
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
ObjCoordinate trophyCoordinate = { glm::vec3(10.0f, 0.0f, -10.0f), 0.0f };
glm::vec3 craft1_trans = glm::vec3(-1.5f, 0.0f, -2.0f);
glm::vec3 craft2_trans = glm::vec3(1.5f, 0.0f, -4.0f);
glm::vec3 craft3_trans = glm::vec3(-1.5f, 0.0f, -6.0f);


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
int allGoldCollected() {
	for (int i = 0; i < 200; i++) {
		if (isGold[i] == 1 && Gold_paint_flag[i] == 1) {
			return 0;
		}
	}
	return 1;
}
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
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
	rock_texture[1].setupTexture("resources/spacecraft/gold.png");
	spacecraft_texture[0].setupTexture("resources/spacecraft/spacecraftTexture.png");
	spacecraft_texture[1].setupTexture("resources/spacecraft/gold.png");
	trophy_texture[0].setupTexture("resources/trophy/gold.png");
	/*
	Skybox
	*/

	float skyboxVertices[] = {

		-25.0f, 25.0f, -25.0f, -1.0f, 1.0f, -1.0f,
		-25.0f, -25.0f, -25.0f, -1.0f, -1.0f, -1.0f,
		25.0f, -25.0f, -25.0f, 1.0f, -1.0f, -1.0f,
		25.0f, -25.0f, -25.0f, 1.0f, -1.0f, -1.0f,
		25.0f, 25.0f, -25.0f, 1.0f, 1.0f, -1.0f,
		-25.0f, 25.0f, -25.0f, -1.0f, 1.0f, -1.0f,

		-25.0f, -25.0f, 25.0f, -1.0f, -1.0f, 1.0f,
		-25.0f, -25.0f, -25.0f, -1.0f, -1.0f, -1.0f,
		-25.0f, 25.0f, -25.0f, -1.0f, 1.0f, -1.0f,
		-25.0f, 25.0f, -25.0f, -1.0f, 1.0f, -1.0f,
		-25.0f, 25.0f, 25.0f, -1.0f, 1.0f, 1.0f,
		-25.0f, -25.0f, 25.0f, -1.0f, -1.0f, 1.0f,
		
		25.0f, -25.0f, -25.0f, 1.0f, -1.0f, -1.0f,
		25.0f, -25.0f, 25.0f, 1.0f, -1.0f, 1.0f,
		25.0f, 25.0f, 25.0f, 1.0f, 1.0f, 1.0f,
		25.0f, 25.0f, 25.0f, 1.0f, 1.0f, 1.0f,
		25.0f, 25.0f, -25.0f, 1.0f, 1.0f, -1.0f,
		25.0f, -25.0f, -25.0f, 1.0f, -1.0f, -1.0f,
		
		-25.0f, -25.0f, 25.0f, -1.0f, -1.0f, 1.0f,
		-25.0f, 25.0f, 25.0f, -1.0f, 1.0f, 1.0f,
		25.0f, 25.0f, 25.0f, 1.0f, 1.0f, 1.0f,
		25.0f, 25.0f, 25.0f, 1.0f, 1.0f, 1.0f,
		25.0f, -25.0f, 25.0f, 1.0f, -1.0f, 1.0f,
		-25.0f, -25.0f, 25.0f, -1.0f, -1.0f, 1.0f,

		-25.0f, 25.0f, -25.0f, -1.0f, 1.0f, -1.0f,
		25.0f, 25.0f, -25.0f, 1.0f, 1.0f, -1.0f,
		25.0f, 25.0f, 25.0f, 1.0f, 1.0f, 1.0f,
		25.0f, 25.0f, 25.0f, 1.0f, 1.0f, 1.0f,
		-25.0f, 25.0f, 25.0f, -1.0f, 1.0f, 1.0f,
		-25.0f, 25.0f, -25.0f, -1.0f, 1.0f, -1.0f,

		-25.0f, -25.0f, -25.0f, -1.0f, -1.0f, -1.0f,
		-25.0f, -25.0f, 25.0f, -1.0f, -1.0f, 1.0f,
		25.0f, -25.0f, -25.0f, 1.0f, -1.0f, -1.0f,
		25.0f, -25.0f, -25.0f, 1.0f, -1.0f, -1.0f,
		-25.0f, -25.0f, 25.0f, -1.0f, -1.0f, 1.0f,
		25.0f, -25.0f, 25.0f, 1.0f, -1.0f, 1.0f,
	};
	glGenVertexArrays(1, &vao_skybox);
	glBindVertexArray(vao_skybox);
	glGenBuffers(1, &vbo_skybox);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_skybox);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
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
	cubemapTexture = loadCubemap(faces);
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
	mouse.sensitivity = 0.15f;
	mouse.yaw = 0.0f;
	mouse.press_flag = 0;
	mouse.move_direction = 0;
	mouse.velocity = 0.05f;
	/* set the shader info */
	skyShader.setupShader("SkyboxVertexShader.glsl", "SkyboxFragmentShader.glsl");
	myShader.setupShader("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
	myShader.use();
	//skyShader.setupShader("SkyboxVertexShader.glsl", "SkyboxFragmentShader.glsl");
	/* initialize the location for rocks */
	for (int i = 0; i < 200; i++) {
		angle[i] = rand() % 360;
		angle_z[i] = rand() % 360;
		height[i] = (float)(rand() % 40) / 100.0f - 0.2f;
		distance_to_planet[i] = (float)(rand() % 100 + 200) / 100.0f;
		isGold[i] = 0;
		Gold_paint_flag[i] = 1;
		//printf("i: %d", angle[i]);
	}
	for (int i = 0; i < 5; i++) {
		isGold[i] = 1;
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void paintGL(void)  //always run
{
	glClearColor(0.2f, 0.2f, 0.4f, 0.5f); //specify the background color, this is just an example
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* skybox */
	skyShader.use();
	skyShader.setInt("skybox", 0);
	glm::mat4 view = glm::lookAt(windowView.viewPoint,
		windowView.endPoint,
		glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::mat4(glm::mat3(view));
	skyShader.setMat4("view", view);
	glm::mat4 projection = glm::perspective(glm::radians(30.0f), 1.78f, 0.1f, 100.0f);
	skyShader.setMat4("projection", projection);
	glBindVertexArray(vao_skybox);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);


	myShader.use();
	int now = time((time_t*)NULL);
	myShader.setInt("normalMapping_flag", GL_FALSE);
	//TODO:
	// 	glBindVertexArray(vaoID[4]);
	//Set lighting information, such as position and color of lighting source
	myShader.setVec3("eyePositionWorld", windowView.viewPoint);
	/*
	light
	*/
	glm::vec4 ambientLight = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
	myShader.setVec4("ambientLight", ambientLight);
	glm::vec3 lightPositionWorld1 = glm::vec3(1.5f, 3.0f, -15.0f);
	myShader.setVec3("lightPositionWorld1", lightPositionWorld1);
	glm::vec3 lightcolor1(1.0f, 1.0f, 1.0f);
	myShader.setVec3("lightcolor1", lightcolor1);
	glm::vec3 lightPositionWorld2 = glm::vec3(5.0f, 3.0f, -15.0f);
	myShader.setVec3("lightPositionWorld2", lightPositionWorld2);
	glm::vec3 lightcolor2(1.0f, 0.843f, 0.0f);
	myShader.setVec3("lightcolor2", lightcolor2);
	myShader.setFloat("delta", light_delta);
	myShader.setFloat("delta1", light_delta1);
	//Set transformation matrix for space craft
	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, spaceshipCoordinate.translation);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.001f, 0.001f, 0.001f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, glm::radians(spaceshipCoordinate.rotation), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);

	view = glm::lookAt(windowView.viewPoint,
		windowView.endPoint,
		glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("view", view);
	int movable = 1;
	myShader.setInt("movable", movable);
	myShader.setMat4("projection", projection);
	/*
	spacecraft
	*/
	if (trophy_translation_flag == 1 || allGoldCollected() == 1) spacecraft_texture[1].bind(0);
	else spacecraft_texture[0].bind(0);
	myShader.setInt("sampler1", 0);
	glBindVertexArray(vaoID[3]);
	glDrawElements(GL_TRIANGLES, size[3], GL_UNSIGNED_INT, 0);
	if (trophy_translation_flag == 1 || allGoldCollected() == 1) spacecraft_texture[1].unbind();
	else spacecraft_texture[0].unbind();
	/*
	planet
	*/
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(1.5f, 0.0f, -15.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, 0.3f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);

	myShader.setInt("normalMapping_flag", 1);
	planet_texture[0].bind(0);
	planet_texture[1].bind(1);
	myShader.setInt("sampler1", 0);
	myShader.setInt("sampler2", 1);
	glBindVertexArray(vaoID[1]);
	glDrawElements(GL_TRIANGLES, size[1], GL_UNSIGNED_INT, 0);
	myShader.setInt("normalMapping_flag", 0);
	planet_texture[0].unbind();
	planet_texture[1].unbind();
	/*
	craft1
	*/
	if (direction[0] == 1) {
		craft1_trans.x += 0.01 * (time_end - time_start);
	}
	else {
		craft1_trans.x -= 0.01 * (time_end - time_start);
	}
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, craft1_trans);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, 0.5f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);
	if (abs(craft1_trans.x - spaceshipCoordinate.translation.x) < 1.3f && abs(craft1_trans.z - spaceshipCoordinate.translation.z) < 0.6f) craft_texture_flag[0] = 1;
	if (craft_texture_flag[0] == 0) craft_texture[0].bind(0);
	if (craft_texture_flag[0] == 1 && now - past >= 1) {
		int i = now % 2;
		craft_texture[i].bind(0);
	}
	myShader.setInt("sampler1", 0);
	glBindVertexArray(vaoID[0]);
	glDrawElements(GL_TRIANGLES, size[0], GL_UNSIGNED_INT, 0);
	/*
	craft2
	*/
	if (direction[1] == 1) {
		craft2_trans.x += 0.01 * (time_end - time_start);
	}
	else {
		craft2_trans.x -= 0.01 * (time_end - time_start);
	}
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, craft2_trans);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, 0.5f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);
	if (abs(craft2_trans.x - spaceshipCoordinate.translation.x) < 1.3f && abs(craft2_trans.z - spaceshipCoordinate.translation.z) < 0.6f) craft_texture_flag[1] = 1;
	if (craft_texture_flag[1] == 0) craft_texture[0].bind(0);
	if (craft_texture_flag[1] == 1 && now - past >= 1) {
		int i = now % 2;
		craft_texture[i].bind(0);
	}
	myShader.setInt("sampler1", 0);
	glBindVertexArray(vaoID[0]);
	glDrawElements(GL_TRIANGLES, size[0], GL_UNSIGNED_INT, 0);
	/*
	craft3
	*/
	if (direction[2] == 1) {
		craft3_trans.x += 0.01 * (time_end - time_start);
	}
	else {
		craft3_trans.x -= 0.01 * (time_end - time_start);
	}
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrix = glm::translate(modelTransformMatrix, craft3_trans);
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, 0.5f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	myShader.setMat4("modelTransformMatrix", modelTransformMatrix);
	if (abs(craft3_trans.x - spaceshipCoordinate.translation.x) < 1.3f && abs(craft3_trans.z - spaceshipCoordinate.translation.z) < 0.6f) craft_texture_flag[2] = 1;
	if (craft_texture_flag[2] == 0) craft_texture[0].bind(0);
	if (craft_texture_flag[2] == 1 && now - past >= 1) {
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
	if (trophy_translation_flag == 0) {
		myShader.setInt("sampler1", 0);
		glBindVertexArray(vaoID[4]);
		glDrawElements(GL_TRIANGLES, size[4], GL_UNSIGNED_INT, 0);
	}
	/*
	rocks
	*/
	for (int i = 0; i < 200; i++) {
		glm::vec3 translation = glm::vec3(1.5f, height[i], -15.0f);
		glm::vec3 translation1 = glm::vec3(distance_to_planet[i] * glm::cos(glm::radians((float)angle[i]) + 0.1 * (float)glfwGetTime()), 0.0f, distance_to_planet[i] * glm::sin(glm::radians((float)angle[i]) + 0.1 * (float)glfwGetTime()));
		translation.x += translation1.x;
		translation.z += translation1.z;
		double distance_to_ship = sqrt(pow(abs(translation.x - spaceshipCoordinate.translation.x), 2) + pow(abs(translation.y - spaceshipCoordinate.translation.y), 2) + pow(abs(translation.z - spaceshipCoordinate.translation.z), 2));
		if (distance_to_ship < 0.3f && isGold[i] == 1) Gold_paint_flag[i] = 0;
		if (Gold_paint_flag[i] == 1) {
			modelTransformMatrix = glm::mat4(1.0f);
			modelTransformMatrix = glm::translate(modelTransformMatrix, translation);
			modelTransformMatrix = glm::rotate(modelTransformMatrix, glm::radians(angle_z[i]), glm::vec3(0.0f, 0.0f, 1.0f));
			modelTransformMatrix = glm::rotate(modelTransformMatrix, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
			modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
			if (isGold[i] == 0) {
				rock_texture[0].bind(0);
			}
			else {
				rock_texture[1].bind(0);
			}
			myShader.setMat4("modelTransformMatrix", modelTransformMatrix);
			myShader.setInt("sampler", 0);
			glBindVertexArray(vaoID[2]);
			glDrawElements(GL_TRIANGLES, size[2], GL_UNSIGNED_INT, 0);
			if (isGold[i] == 0) {
				rock_texture[0].unbind();
			}
			else {
				rock_texture[1].unbind();
			}
		}
	}
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
	spaceshipCoordinate.rotation = 180 - mouse.yaw;
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
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Skyship", NULL, NULL);
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
	time_start = glfwGetTime();
	time_end = 0;
	static double INTERVAL = 0.0167f;
	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		nowTime = glfwGetTime();
		time_end = glfwGetTime();
		deltaTime = nowTime - lastTime;
		if (deltaTime >= INTERVAL) {
			lastTime = nowTime;
			updateMovement();
		}
		if (time_end - time_start >= 3.0f) {
			time_start = time_end;
			for (int i = 0; i < 3; i++) {
				direction[i] = 1 - direction[i];
			}
		}
		paintGL();
		//printf("%d\n", glfwGetTime());
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






