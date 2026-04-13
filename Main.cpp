#include <iostream>
#include <SDL3/SDL.h>
#include<glad/glad.h>
#include <vector>
#include <string>
#include <fstream>

// Globals (prefix "g" to flag these variables as global)
int gScreenWidth = 640;
int gScreenHeight = 480;

SDL_Window* gGraphicsAppWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

bool gQuitApp = false; // If true, the program quits

// VAO
GLuint gVertexArrayObject = 0;

// VBOs
GLuint gVertexBufferObject = 0;
GLuint gVertexBufferObject2 = 1;
GLuint gIndexBufferObject = 2;

// Program Object (for shaders)
GLuint gGraphicsPipelineProgram = 0;

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ GLOBAL VARIABLES ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

static void GLClearAllErrors() {
	while (glGetError() != GL_NO_ERROR) {

	}
}

// Return true if we have an error
static bool GLCheckErrorStatus(const char* function, int line) {
	while (GLenum error = glGetError()) {
		std::cout << "OpenGL Error: " << error 
			<< "\tLine: " << line 
			<< "\tfunction: " << function
			<< std::endl;
		return true;
	}
	return false;
}

// This macro wraps a function that could create an error and prints out the error type, the line and the name of the function
#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ERROR HANDLING ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

std::string LoadShaderAsString(const std::string& filename)
{
	// Shader program loaded as a string
	std::string result = "";

	// Read the file input
	std::string line = "";
	std::ifstream myFile(filename.c_str());

	if (myFile.is_open()) {
		while (std::getline(myFile, line)) {
			result += line + "\n";
		}
		myFile.close();
	}

	return result;
}


void GetOpenGLInfos() {
std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}




void InitProgram() {

	// Initializing error handler
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cout << "SDL3 wasn't able to initialize video subsystem" << std::endl;
		exit(1);
	}

	// OpenGL Attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // OpenGL version 4
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); // OpenGL subversion .1 (OpenGL 4.1)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // OpenGL Core setup (Modern Version of OpenGL)
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // Bits alloc for the double buffer (default: 16)

	// Window Creation
	gGraphicsAppWindow = SDL_CreateWindow("OpenGL Test", gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);

	// Window not created error handler
	if (gGraphicsAppWindow == nullptr) {
		std::cout << "SDL_Window wasn't able to be created" << std::endl;
		exit(1);
	}

	// GL Context creation
	gOpenGLContext = SDL_GL_CreateContext(gGraphicsAppWindow); 

	// OpenGL context not created error handler
	if (gOpenGLContext == nullptr) {
		std::cout << "OpenGL context failed to be created" << std::endl;
		exit(1);
	}

	// GLAD library initialization
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cout << "GLAD library was not initialized" << std::endl;
		exit(1);
	}

	GetOpenGLInfos();
}




void VertexSpecification() 
{
	// Actual vertex data that lives in the CPU
	const std::vector<GLfloat> vertexData
	{	
		 // 0 - Vertex
		-0.5f, -0.5f, 0.0f, // bottom left vertex
		 1.0f,  0.0f, 0.0f, // color
		 // 1 - Vertex
		 0.5f, -0.5f, 0.0f, // bottom right vertex
		 0.0f,  1.0f, 0.0f, // color
		 // 2 - Vertex
		-0.5f,  0.5f, 0.0f, // top left vertex
		 0.0f,  0.0f, 1.0f, // color
		 // 3 - Vertex
		 0.5f,  0.5f, 0.0f, // top right vertex
		 1.0f,  0.0f, 0.0f, // color

	};

	const std::vector<GLuint> indexBufferData
	{
		2, 0, 1, // First triangle
		3, 2, 1  // Second triangle
	};

	// Generating and binding the Vertex Array Object (VAO)
	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	// Generating and binding the Vertex Buffer Object (VBO)
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);

	// Generating and binding the Index/Element Buffer Object (IBO/EBO)
	// Used when we want to draw triangles based on vertex indices
	glGenBuffers(1, &gIndexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLuint), indexBufferData.data(), GL_STATIC_DRAW);

	// Linking the Position attribute to the VAO
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void*)0);
	// Linking the Color attribute to the VAO
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (GLvoid*) (3 * sizeof(GL_FLOAT)));

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}



// Compiling the source code passed in
GLuint CompileShader(GLuint type, const std::string& sourceCode)
{
	GLuint shaderObject = glCreateShader(type);
	const char* src = sourceCode.c_str();
	glShaderSource(shaderObject, 1, &src, nullptr);
	glCompileShader(shaderObject);

	return shaderObject;
}



// Binding all the compiled shaders to a program object
GLuint CreateShaderProgram(const std::string& vertShaderSource, const std::string& fragShaderSource)
{
	GLuint programObject = glCreateProgram();
	
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertShaderSource);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragShaderSource);

	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);
	glLinkProgram(programObject);

	// Validate the program
	glValidateProgram(programObject);

	return programObject;
}




void CreateGraphicsPipeline()
{
	// Loading the shaders from the file and storing them into some variables
	std::string vertexSourceCode = LoadShaderAsString("shaders/vert.glsl");
	std::string fragmentSourceCode = LoadShaderAsString("shaders/frag.glsl");

	gGraphicsPipelineProgram = CreateShaderProgram(vertexSourceCode, fragmentSourceCode);
}



// Input handler
void Input() {
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_EVENT_QUIT) {
			std::cout << "Terminating application..." << std::endl;
			gQuitApp = true;
		}
	}
}



// Sets OpenGL's state
void PreDraw() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(1.0f, 1.0f, 0.2f, 1.f);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glUseProgram(gGraphicsPipelineProgram);
}



// Shows the images on screen
void Draw() {
	glBindVertexArray(gVertexArrayObject);
	GLCheck(glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject));

	GLCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
}



// Window main
void MainLoop() {
	// Checks if the user wants to quit the application
	while (!gQuitApp) {
		Input();

		PreDraw();

		Draw();

		// Update the screen
		SDL_GL_SwapWindow(gGraphicsAppWindow);
	}
}



// Liberating space on the memory
void CleanUp() {
	SDL_DestroyWindow(gGraphicsAppWindow);
	SDL_Quit();
}



// Program main
int main() {

	InitProgram();

	VertexSpecification();

	CreateGraphicsPipeline();

	MainLoop();

	CleanUp();

	return 0;
}