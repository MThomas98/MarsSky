#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include <chrono>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <string>
#include <time.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include "Shader.h"
#include "Model.h"
#include "Decal.h"
#include "MarsCalc.h"

#define WIN_TITLE "CS310 Project"
#define WIN_WIDTH 1920
#define WIN_HEIGHT 1080
#define WIN_RESIZABLE GL_FALSE
#define RADIUS 10.0f
#define FPS 60

using namespace std;
using namespace std::chrono;

GLFWwindow* window;

Shader* objShader;
Shader* colorShader;
Shader* decalShader;

Model* curiosity;
Model* sphere;
Model* ground;
Model* cube;
Decal* grid;
Decal* compass;

GLuint mapTexID;
int mapWidth;
int mapHeight;

milliseconds lastms;

glm::vec3 camPos = glm::vec3(-13.0f, 6.0f, -13.0f);
glm::vec3 lightPos = glm::vec3(0.0f, RADIUS, 0.0f);

double zoom = 25.0f;
double xRot = 0.0f;
double yRot = 30.0f;
double rotSensitivity = 0.4f;
double scrollSensitivity = 1.0f;

float lon;
float lat;
float azim;
float elev;
time_t sunrise;
time_t sunset;
time_t curTime = time(NULL);

/*------------------------------*
*		  Utility funcs  		*
*-------------------------------*/

glm::vec3 orbitPos(float azim, float elev, float rad) {
	float azimRad = glm::radians(azim);
	float elevRad = glm::radians(elev);

	return glm::vec3(
		 rad*cos(elevRad)*sin(azimRad),
		 rad*sin(elevRad),
		-rad*cos(elevRad)*cos(azimRad)
	);
}

/*------------------------------*
*		  Drawing funcs  		*
*-------------------------------*/

// Draws the UI to the screen
bool error = false;
string errorStr = "";
bool inputInit = true;
void drawUI() {
	// Setup new UI frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Main inputs
	ImGui::Begin("Input", NULL, ImGuiWindowFlags_NoResize);
		// Data holders
		static char dayStr[3];
		static char monStr[3];
		static char yearStr[5];
		static char hourStr[3];
		static char minStr[3];
		static char secStr[3];
		static char latStr[10];
		static char lonStr[10];

		// Get current time info on first run
		if (inputInit) {
			inputInit = false;
			struct tm* tmInfo = gmtime(&curTime);

			strftime(dayStr,  3, "%d", tmInfo);
			strftime(monStr,  3, "%m", tmInfo);
			strftime(yearStr, 5, "%Y", tmInfo);
			strftime(hourStr, 3, "%H", tmInfo);
			strftime(minStr,  3, "%M", tmInfo);
			strftime(secStr,  3, "%S", tmInfo);
			to_string(lat).copy(latStr, 4);
			to_string(lon).copy(lonStr, 4);

			inputInit = false;
		}

		// Input boxes
		ImGui::PushItemWidth(40);
		ImGui::InputText("day  ", dayStr, IM_ARRAYSIZE(dayStr));
		ImGui::SameLine(); ImGui::InputText("month", monStr, IM_ARRAYSIZE(monStr));
		ImGui::SameLine(); ImGui::InputText("year ", yearStr, IM_ARRAYSIZE(yearStr));
		ImGui::InputText("hour ", hourStr, IM_ARRAYSIZE(dayStr));
		ImGui::SameLine(); ImGui::InputText("min  ", minStr, IM_ARRAYSIZE(monStr));
		ImGui::SameLine(); ImGui::InputText("sec  ", secStr, IM_ARRAYSIZE(yearStr));
		ImGui::InputText("lat  ", latStr, IM_ARRAYSIZE(latStr));
		ImGui::SameLine(); ImGui::InputText("lon  ", lonStr, IM_ARRAYSIZE(lonStr));
		ImGui::PopItemWidth();

		// Error text
		if (error)
			ImGui::Text("ERROR: %s", errorStr.c_str());

		if (ImGui::Button("Run")) {
			tm tmTime;
			tmTime.tm_year = atoi(yearStr) - 1900;
			tmTime.tm_mon  = atoi(monStr) - 1;
			tmTime.tm_mday = atoi(dayStr);
			tmTime.tm_hour = atoi(hourStr);
			tmTime.tm_min  = atoi(minStr);
			tmTime.tm_sec  = atoi(secStr);
			tmTime.tm_isdst = 0;
			curTime = mktime(&tmTime);

			if (!curTime) {
				errorStr = "Invalid date/time given.";
				error = true;
			} else if (atof(latStr) > 90.0f || atof(latStr) < -90.0f) {
				errorStr = "Invalid latitude given.";
				error = true;
			} else {
				error = false;
				lon = atoi(lonStr);
				lat = atoi(latStr);
				elev = MarsCalc::solarElevation(lon, lat, curTime);
				azim = MarsCalc::solarAzimuth(lon, lat, curTime);
				lightPos = orbitPos(azim, elev, RADIUS);
			}
		}
	ImGui::End();

	// Results box
	ImGui::Begin("Results", NULL, ImGuiWindowFlags_NoResize);
		ImGui::Text("Azimuth:      %f", azim);
		ImGui::Text("Elevation:    %f", elev);
		char srbuff[20];
		char ssbuff[20];
		strftime(srbuff, 20, "%H:%M:%S", gmtime(&sunrise));
		strftime(ssbuff, 20, "%H:%M:%S", gmtime(&sunset));
		ImGui::Text("Sunrise:      %s", srbuff);
		ImGui::Text("Sunset:       %s", ssbuff);
	ImGui::End();

	// Draw map location picker
	float width = 620.0f;
	float height = 320.0f;
	ImGui::SetNextWindowSize(ImVec2(width+15, height+35));
	ImGui::Begin("Location", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		ImGuiIO& io = ImGui::GetIO();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		ImGui::Image((void*)(intptr_t)mapTexID, ImVec2(width, height));

		float x = (width * (lon + 180.0f)) / 360.0f;
		float y = -(height * (lat - 90.0f)) / 180.0f;
		float circleSize = 3.0f;
		drawList->AddCircleFilled(ImVec2(pos.x + x, pos.y + y), circleSize, ImColor(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)));
		drawList->AddCircle(ImVec2(pos.x + x, pos.y + y), circleSize + 1.0f, ImColor(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
				float regionSize = 23.0f;
				float zoom = 8.0f;

				float regionX = io.MousePos.x - pos.x - regionSize * 0.5f;
				if (regionX < 0.0f) regionX = 0.0f;
				else if (regionX > width - regionSize) regionX = width - regionSize;

				float regionY = io.MousePos.y - pos.y - regionSize * 0.5f;
				if (regionY < 0.0f) regionY = 0.0f;
				else if (regionY > height - regionSize) regionY = height - regionSize;

				float imagePosX = io.MousePos.x - pos.x;
				float imagePosY = io.MousePos.y - pos.y;
				float lonPos = (360.0f/width * imagePosX) - 180.0f;
				float latPos = -((180.0f/height * imagePosY) - 90.0f);
				ImGui::Text("lat: %0.2f° lon: %0.2f°", latPos, lonPos);

				ImVec2 ttPos = ImGui::GetCursorScreenPos();
				ImVec2 uv0 = ImVec2(regionX / width, regionY / height);
				ImVec2 uv1 = ImVec2((regionX + regionSize) / width, (regionY + regionSize) / height);
				ImGui::Image((void*)(intptr_t)mapTexID, ImVec2(regionSize * zoom, regionSize * zoom),
									uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

				float thickness = 3.0f;
				float len = 40.0f;
				float midX = ttPos.x + ((regionSize*zoom)/2) + 2;
				float midY = ttPos.y + ((regionSize*zoom)/2) + 2;
				drawList->AddLine(ImVec2(midX - len/2 - 2.5f, midY), ImVec2(midX + len/2 + 2.5f, midY), ImColor(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)), thickness+5.0f);
				drawList->AddLine(ImVec2(midX, midY - len/2 - 2.5f), ImVec2(midX, midY + len/2 + 2.5f), ImColor(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)), thickness+5.0f);
				drawList->AddLine(ImVec2(midX - len/2, midY), ImVec2(midX + len/2, midY), ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), thickness);
				drawList->AddLine(ImVec2(midX, midY - len/2), ImVec2(midX, midY + len/2), ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), thickness);
			ImGui::EndTooltip();

			if (io.MouseDown[0]) {
				lon = lonPos;
				lat = latPos;

				sunrise = MarsCalc::lastSunrise(lon, lat, curTime);
				sunset = MarsCalc::nextSunset(lon, lat, curTime);

				elev = MarsCalc::solarElevation(lon, lat, curTime);
				azim = MarsCalc::solarAzimuth(lat, lat, curTime);
				lightPos = orbitPos(azim, elev, RADIUS);

				to_string(lat).copy(latStr, 10);
				to_string(lon).copy(lonStr, 10);
			}
		}

	ImGui::End();

	// TODO: Remove after testing
	// ImGui::ShowDemoWindow();

	// Render UI
	ImGui::Render();
	glUseProgram(0);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// OpenGL rendering code, called once per frame
void drawGraphics(GLFWwindow* window) {
	// Clear the screen and unbind VAO
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Calculate projection and view matricies
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), ((float)width)/((float)height), 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Setup solid color shader matricies
	colorShader->use();
	colorShader->setMat4f("proj", proj);
	colorShader->setMat4f("view", view);

	// Setup decal shader matricies
	decalShader->use();
	decalShader->setMat4f("proj", proj);
	decalShader->setMat4f("view", view);

	// Set up object shader
	objShader->use();
	objShader->setMat4f("proj", 				proj);
	objShader->setMat4f("view", 				view);
	objShader->setVec3f("viewPos", 				camPos.x, camPos.y, camPos.z);
	objShader->setVec3f("light.position",		lightPos.x, lightPos.y, lightPos.z);
	objShader->setVec3f("light.ambient",		1.0f, 0.6f, 0.0f);
	objShader->setVec3f("light.diffuse",		1.0f, 0.6f, 0.0f);
	objShader->setVec3f("light.specular",		0.2f, 0.2f, 0.0f);
	objShader->setFloat("light.constant",		6.0f);
	objShader->setFloat("light.linear",			0.040f);
	objShader->setFloat("light.quadratic",		0.0055f);
	objShader->setVec3f("dlight.ambient",		0.6f, 0.6f, 0.6f);
	objShader->setVec3f("dlight.diffuse",		0.1f, 0.1f, 0.1f);
	objShader->setVec3f("dlight.specular",		0.1f, 0.1f, 0.1f);
	objShader->setVec3f("dlight.direction",		0.0f, -1.0f, 0.0f);

	// Draw light sphere
	colorShader->use();
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	colorShader->setVec4f("color", 1.0f, 0.6f, 0.0f, 1.0f);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPos);
	model = glm::scale(model, glm::vec3(0.6f, 0.6f, 0.6f));
	colorShader->setMat4f("model", model);
	sphere->draw(*colorShader);

	// Draw light sphere outline
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	colorShader->setVec4f("color", 1.0f, 0.0f, 0.0f, 1.0f);
	model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));
	colorShader->setMat4f("model", model);
	sphere->draw(*colorShader);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);

	// Draw reference model
	objShader->use();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
	objShader->setMat4f("model", model);
	curiosity->draw(*objShader);

	// Draw grid decals
	decalShader->use();
	model = glm::scale(model, glm::vec3(2.0f, 0.0f, 2.0f));
	int gridSize = 26; // NOTE: Must be even
	for (int i = -gridSize/2; i < gridSize/2; i++) {
		for (int j = -gridSize/2; j < gridSize/2; j++) {
			model = glm::translate(glm::mat4(1.0f), glm::vec3(1.98f*(i%gridSize), -1.0f, 1.98f*(j%gridSize)));
			decalShader->setMat4f("model", model);
			grid->draw(*objShader);
		}
	}

	// Draw ground compass
	decalShader->use();
	model = glm::scale(glm::mat4(1.0f), glm::vec3(RADIUS, 1.0f, RADIUS));
	model = glm::translate(model, glm::vec3(0.0f, -0.9f, 0.0f));
	model = glm::rotate(model, -3.141590f/2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	decalShader->setMat4f("model", model);
	compass->draw(*objShader);

	// Draw reference sphere
	colorShader->use();
	glDisable(GL_DEPTH_TEST);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(RADIUS, RADIUS, RADIUS));
	colorShader->setMat4f("model", model);
	colorShader->setVec4f("color", 1.0f, 1.0f, 1.0f, 0.2f);
	sphere->draw(*colorShader);
	glEnable(GL_DEPTH_TEST);

	// Render UI
	drawUI();

	// Bring to front buffer
	glfwSwapBuffers(window);
}

/*------------------------------*
*		  Callback funcs		*
*-------------------------------*/

// Called on a keypress
static void keyCallback(GLFWwindow*, int key, int, int, int) {
	// TODO
}

// Called on a mouse press
bool rmouseDown = false;
static void mouseButtonCallback(GLFWwindow*, int button, int action, int) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			rmouseDown = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		else if (action == GLFW_RELEASE) {
			rmouseDown = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

// Called on mouse move
double lastX, lastY;
static void mousePosCallback(GLFWwindow*, double x, double y) {
	if (rmouseDown && !ImGui::IsAnyWindowHovered()) {
		xRot += (x - lastX) * rotSensitivity;
		yRot += (y - lastY) * rotSensitivity;
		if (yRot >= 85)  yRot = 85;
		if (yRot <= -85) yRot = -85;
		camPos = orbitPos(xRot, yRot, zoom);
	}

	lastX = x;
	lastY = y;
}

// Called on mouse scroll
static void mouseScrollCallback(GLFWwindow*, double, double scroll) {
	if (!ImGui::IsAnyWindowHovered()) {
		zoom-=scroll*scrollSensitivity;
		if (zoom <= 5.0f) zoom = 5.0f;
		if (zoom >= 65.0f) zoom = 65.0f;
		camPos = orbitPos(xRot, yRot, zoom);
	}
}

/*------------------------------*
*		  Initialisation		*
*-------------------------------*/

// Entry function
int main(int argc, char** argv) {
	// For fixing stdout buffering on windows
	#ifdef _WIN32
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	#endif

	// Initialise GLFW3
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: Failed to initialise GLFW.\n");
		return 1;
	}
	printf("\nInitialised GLFW.\n");

	// Create GLFW window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, WIN_RESIZABLE);
	window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: Failed to create window.\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	printf("Created window.\n");

	// Initialise GLEW
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		fprintf(stderr, "ERROR: %s\n", glewGetErrorString(err));
		return 0;
	}
	printf("Initialised GLEW.\n");

	// Set GLFW/GL settings, callback funcs etc.
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSwapInterval(1);
	glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	printf("Initialised GL settings.\n");

	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// TODO: Setup the 3D space plots

	// Create the shaders
	printf("Compiling Shaders... ");
	objShader = new Shader("shaders/objshader.vs", "shaders/objshader.fs");
	colorShader = new Shader("shaders/colorshader.vs", "shaders/colorshader.fs");
	// gridShader = new Shader("shaders/gridshader.vs", "shaders/gridshader.fs");
	decalShader = new Shader("shaders/decalshader.vs", "shaders/decalShader.fs");
	printf("Done.\n");

	// Load in Assets
	printf("Loading Assets... ");
	curiosity = new Model("models/curiosity/curiosity.obj");
	cube = new Model("models/cube/cube.obj");
	sphere = new Model("models/sphere/sphere.obj");
	grid = new Decal("img/grid.png");
	compass = new Decal("img/compass.png");

	// Load map texture
	unsigned char* mapData = stbi_load("img/marsterrain.png", &mapWidth, &mapHeight, NULL, 4);
	if (!mapData) {
		printf("\nERROR: Failed to load mars terrain image.\n");
	}
	glGenTextures(1, &mapTexID);
	glBindTexture(GL_TEXTURE_2D, mapTexID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 1000);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mapWidth, mapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mapData);
	stbi_image_free(mapData);
	printf("Done.\n");

	// Get current sun position
	printf("Setting up inital values...");
	curTime = time(NULL);
	elev = MarsCalc::solarElevation(0, 0, curTime);
	azim = MarsCalc::solarAzimuth(0, 0, curTime);
	lightPos = orbitPos(azim, elev, RADIUS);
	camPos = orbitPos(xRot, yRot, zoom);
	sunrise = MarsCalc::lastSunrise(lon, lat, curTime);
	sunset = MarsCalc::nextSunset(lon, lat, curTime);
	printf("Done.\n");

	// Main loop (draw -> poll -> draw ...)
	printf("Finished initialisation\n\n");
	while (!glfwWindowShouldClose(window)) {
		milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		milliseconds delta = ms - lastms;

		if (delta.count() >= FPS/1000) {
			drawGraphics(window);
			glfwPollEvents();
		}
	}

	// Cleanup GLFW
	ImGui_ImplGlfw_Shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
