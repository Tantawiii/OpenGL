#include <iostream>
#include <gl/glew/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include<gl\glm\glm.hpp>
#include<gl\glm\gtc\type_ptr.hpp>
#include<gl\glm\gtx\transform.hpp>

using namespace std;
using namespace glm;

enum DrawingMode
{
	Points,
	Lines,
	FilledTriangle
};

struct vertex
{
	vec3 position;
	vec3 color;
	vertex() {}
	vertex(vec3 _position, vec3 _color) :position{ _position }, color{ _color }
	{
	}
	vertex(vec3 _position) :position{ _position }, color{ _position }
	{
	}
};

enum RotationDirection
{
	None,
	Up,
	Down,
	Left,
	Right
};

enum CameraMovement
{
	Forward,
	Backward,
	Leftward,
	Rightward,
	Jump,
	Crouch,
	RotateRight,
	RotateLeft,
	RotateUpward,
	RotateDownward,
	CNone
};

RotationDirection currentRotation = None;
CameraMovement cameraCurrentInput = CNone;


GLuint InitShader(const char* vertex_shader_file_name, const char* fragment_shader_file_name);

const GLint WIDTH = 600, HEIGHT = 600;
GLuint VBO_Triangle, VBO_Cube, IBO, BasiceprogramId;
DrawingMode Current_DrawingMode = DrawingMode::FilledTriangle;

// transformation
GLuint modelMatLoc, viewMatLoc, projMatLoc;

mat4 headTransform = mat4(1.0f);
mat4 bodyTransform = mat4(1.0f);
mat4 leftArmTransform = mat4(1.0f);
mat4 rightArmTransform = mat4(1.0f);
mat4 leftLegTransform = mat4(1.0f);
mat4 rightLegTransform = mat4(1.0f);
mat4 floorTransform = mat4(1.0f);

void CreateTriangle()
{
	vertex TriangleVertices[] =
	{
		vec3(-1,-1,0),vec3(0,1,0),
		vec3(1,-1,0),vec3(0,0,1),
		vec3(0,1,0),vec3(1,0,0)
	};

	// create buffer object
	glGenBuffers(1, &VBO_Triangle);

	// binding buffer object
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVertices), TriangleVertices, GL_STATIC_DRAW);

	// shader
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(GL_FLOAT), (char*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);

	//GLuint Vertex_Position_Location = glGetAttribLocation(BasiceprogramId, "vertex_position");
}

void BindTriangle()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Triangle);

	// shader
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(GL_FLOAT), (char*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
}


vertex cube_core_vertices[] = {
{vec3(-0.5, 0.5, 0.5),vec3(1,0,0)},
{vec3(-0.5, -0.5, 0.5),vec3(0,1,0)},
{vec3(0.5, -0.5, 0.5),vec3(1,0,1)},
{vec3(0.5, 0.5, 0.5),vec3(0,0,1)},
{vec3(0.5, 0.5, -0.5),vec3(1,1,0)},
{vec3(0.5, -0.5, -0.5),vec3(0,1,1)},
{vec3(-0.5, -0.5, -0.5),vec3(1,1,1)},
{vec3(-0.5, 0.5, -0.5),vec3(0,0,0)}
};
void CreateCube()
{
	int vertices_Indeces[] = {
		//front
		0,
		1,
		2,

		0,
		2,
		3,
		//Right
		3,
		2,
		5,

		3,
		5,
		4,
		//Back
		4,
		5,
		6,

		4,
		6,
		7,
		//Left
		7,
		6,
		1,

		7,
		1,
		0,
		//Top
		7,
		0,
		3,

		7,
		3,
		4,
		//Bottom
		2,
		1,
		6,

		2,
		6,
		5
	};

	// create VBO
	glGenBuffers(1, &VBO_Cube);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_core_vertices), cube_core_vertices, GL_STATIC_DRAW);

	// Index Buffer
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertices_Indeces), vertices_Indeces, GL_STATIC_DRAW);

	// shader
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vertex), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(vertex), (char*)(sizeof(vec3)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(vertex), (char*)(2 * sizeof(vec3)));
	glEnableVertexAttribArray(2);

}

void BindCube()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Cube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vertex), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(vertex), (char*)(sizeof(vec3)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(vertex), (char*)(2 * sizeof(vec3)));
	glEnableVertexAttribArray(2);
}

void CompileShader(const char* vertex_shader_file_name, const char* fragment_shader_file_namering, GLuint& programId)
{
	programId = InitShader(vertex_shader_file_name, fragment_shader_file_namering);
	glUseProgram(programId);
}

int Init()
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		cout << "Error";
		getchar();
		return 1;
	}
	else
	{
		if (GLEW_VERSION_3_0)
			cout << "Driver support OpenGL 3.0\nDetails:\n";
	}
	cout << "\tUsing glew " << glewGetString(GLEW_VERSION) << endl;
	cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	CompileShader("VS.glsl", "FS.glsl", BasiceprogramId);
	CreateTriangle();
	CreateCube();

	modelMatLoc = glGetUniformLocation(BasiceprogramId, "modelMat");
	viewMatLoc = glGetUniformLocation(BasiceprogramId, "viewMat");
	projMatLoc = glGetUniformLocation(BasiceprogramId, "projMat");

	glm::mat4 viewMat = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));

	glm::mat4 projMat = glm::perspectiveFov(60.0f, (float)WIDTH, (float)HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, glm::value_ptr(projMat));

	glClearColor(0, 0.5, 0.5, 1);
	glEnable(GL_DEPTH_TEST);

	return 0;
}

//float theta = 0;
//void Update()
//{
//	// add all tick code
//	theta += 0.001f;
//}
// 
// 
//void Update()
//{
//	// Apply rotation based on the current direction
//	switch (currentRotation)
//	{
//	case Up:
//		headTransform = rotate(headTransform, radians(0.5f), vec3(1, 0, 0));
//		bodyTransform = rotate(bodyTransform, radians(0.5f), vec3(1, 0, 0));
//		leftArmTransform = rotate(leftArmTransform, radians(0.5f), vec3(1, 0, 0));
//		rightArmTransform = rotate(rightArmTransform, radians(0.5f), vec3(1, 0, 0));
//		leftLegTransform = rotate(leftLegTransform, radians(0.5f), vec3(1, 0, 0));
//		rightLegTransform = rotate(rightLegTransform, radians(0.5f), vec3(1, 0, 0));
//		break;
//	case Down:
//		headTransform = rotate(headTransform, radians(-0.5f), vec3(1, 0, 0));
//		bodyTransform = rotate(bodyTransform, radians(-0.5f), vec3(1, 0, 0));
//		leftArmTransform = rotate(leftArmTransform, radians(-0.5f), vec3(1, 0, 0));
//		rightArmTransform = rotate(rightArmTransform, radians(-0.5f), vec3(1, 0, 0));
//		leftLegTransform = rotate(leftLegTransform, radians(-0.5f), vec3(1, 0, 0));
//		rightLegTransform = rotate(rightLegTransform, radians(-0.5f), vec3(1, 0, 0));
//		break;
//	case Left:
//		headTransform = rotate(headTransform, radians(0.5f), vec3(0, 1, 0));
//		bodyTransform = rotate(bodyTransform, radians(0.5f), vec3(0, 1, 0));
//		leftArmTransform = rotate(leftArmTransform, radians(0.5f), vec3(0, 1, 0));
//		rightArmTransform = rotate(rightArmTransform, radians(0.5f), vec3(0, 1, 0));
//		leftLegTransform = rotate(leftLegTransform, radians(0.5f), vec3(0, 1, 0));
//		rightLegTransform = rotate(rightLegTransform, radians(0.5f), vec3(0, 1, 0));
//		break;
//	case Right:
//		headTransform = rotate(headTransform, radians(-0.5f), vec3(0, 1, 0));
//		bodyTransform = rotate(bodyTransform, radians(-0.5f), vec3(0, 1, 0));
//		leftArmTransform = rotate(leftArmTransform, radians(-0.5f), vec3(0, 1, 0));
//		rightArmTransform = rotate(rightArmTransform, radians(-0.5f), vec3(0, 1, 0));
//		leftLegTransform = rotate(leftLegTransform, radians(-0.5f), vec3(0, 1, 0));
//		rightLegTransform = rotate(rightLegTransform, radians(-0.5f), vec3(0, 1, 0));
//		break;
//	case None:
//		// No rotation
//		break;
//	}
//}
vec3 cameraPosition = vec3(0, 0, 3);
vec3 cameraTarget = vec3(0, 0, 0);
vec3 cameraUp = vec3(0, 1, 0);
//const vec3 rotationAxisY(0.0f, 1.0f, 0.0f);
//const vec3 rotationAxisX(1.0f, 0.0f, 0.0f);
//vec3 offset = cameraPosition - cameraTarget;

void Update() {
	//const float rotationSpeed = radians(5.0f); // Rotation speed in radians
	//const vec3 rotationAxisY(0.0f, 1.0f, 0.0f); // Axis for horizontal rotation (Y-axis)
	//const vec3 rotationAxisX(1.0f, 0.0f, 0.0f); // Axis for vertical rotation (X-axis)
	//vec3 offset = cameraPosition - cameraTarget; // Offset vector from target to camera
	static float angleY = 0.0f;
	static float angleX = 0.0f;
	const float rotationSpeed = radians(0.01f);
	const float radius = length(cameraPosition - cameraTarget);
	switch (cameraCurrentInput)
	{
	case Forward:
		/*cameraPosition.z += 0.1;*/
		//cameraTarget += vec3(0.0f, 0.0f, -1.0f);
		cameraPosition += vec3(0.0f, 0.0f, -0.01f);
		break;
	case Backward:
		//cameraPosition.z -= 0.1;
		//cameraTarget += vec3(0.0f, 0.0f, 1.0f);
		cameraPosition += vec3(0.0f, 0.0f, 0.01f);
		break;
	case Leftward:
		//cameraPosition.x -= 0.1;
		//cameraTarget += vec3(-1.0f, 0.0f, 0.0f);
		cameraPosition += vec3(-0.01f, 0.0f, 0.0f);
		break;
	case Rightward:
		//cameraPosition.x += 0.1;
		//cameraTarget += vec3(1.0f, 0.0f, 0.0f);
		cameraPosition += vec3(0.01f, 0.0f, 0.0f);
		break;
	case Jump:
		//cameraPosition.y += 0.1;
		//cameraTarget += vec3(0.0f, 1.0f, 0.0f);
		cameraPosition += vec3(0.0f, 0.01f, 0.0f);
		break;
	case Crouch:
		//cameraPosition.y -= 0.1;
		//cameraTarget += vec3(0.0f, -1.0f, 0.0f);
		cameraPosition += vec3(0.0f, -0.01f, 0.0f);
		break;
	//case RotateRight:
	//	//cameraPosition = cameraPosition;
	//	offset = vec3(rotate(mat4(1.0f), rotationSpeed, rotationAxisY) * vec4(offset, 1.0f));
	//	cameraPosition = cameraTarget + offset;
	//	break;
	//case RotateLeft:
	//	//cameraPosition = cameraPosition;
	//	offset = vec3(rotate(mat4(1.0f), -rotationSpeed, rotationAxisY) * vec4(offset, 1.0f));
	//	cameraPosition = cameraTarget + offset;
	//	break;
	//case RotateUpward:
	//	offset = vec3(rotate(mat4(1.0f), -rotationSpeed, rotationAxisX) * vec4(offset, 1.0f));
	//	cameraPosition = cameraTarget + offset;
	//	break;
	//case RotateDownward:
	//	offset = vec3(rotate(mat4(1.0f), rotationSpeed, rotationAxisX) * vec4(offset, 1.0f));
	//	cameraPosition = cameraTarget + offset;
	//	break;
	case RotateRight:
		angleY -= rotationSpeed;
		cameraPosition.x = cameraTarget.x + radius * cos(angleX) * sin(angleY);
		cameraPosition.y = cameraTarget.y + radius * sin(angleX);
		cameraPosition.z = cameraTarget.z + radius * cos(angleX) * cos(angleY);
		break;
	case RotateLeft:
		angleY += rotationSpeed;
		cameraPosition.x = cameraTarget.x + radius * cos(angleX) * sin(angleY);
		cameraPosition.y = cameraTarget.y + radius * sin(angleX);
		cameraPosition.z = cameraTarget.z + radius * cos(angleX) * cos(angleY);
		break;
	case RotateUpward:
		angleX += rotationSpeed;
		cameraPosition.x = cameraTarget.x + radius * cos(angleX) * sin(angleY);
		cameraPosition.y = cameraTarget.y + radius * sin(angleX);
		cameraPosition.z = cameraTarget.z + radius * cos(angleX) * cos(angleY);
		break;
	case RotateDownward:
		angleX -= rotationSpeed;
		cameraPosition.x = cameraTarget.x + radius * cos(angleX) * sin(angleY);
		cameraPosition.y = cameraTarget.y + radius * sin(angleX);
		cameraPosition.z = cameraTarget.z + radius * cos(angleX) * cos(angleY);
		break;
	case CNone:
		//Reset to starting point
		//cameraTarget = vec3(0.0f, 0.0f, 0.0f);
		cameraPosition = vec3(0.0f, 0.0f, 3.0f);
		break;
	}
	cout << "Current Input: " << cameraCurrentInput << endl;




	cout << "Camera Position: " << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << endl;

	cout << "Camera Target: " << cameraTarget.x << ", " << cameraTarget.y << ", " << cameraTarget.z << endl;

	cout << "Camera Up: " << cameraUp.x << ", " << cameraUp.y << ", " << cameraUp.z << endl;

	mat4 viewMat = lookAt(cameraPosition, cameraTarget, cameraUp);
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, value_ptr(viewMat));
}

void CreateHumanOnFloor()
{
	// Use the existing transformation matrices directly
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, value_ptr(headTransform));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);

	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, value_ptr(bodyTransform));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);

	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, value_ptr(leftArmTransform));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);

	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, value_ptr(rightArmTransform));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);

	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, value_ptr(leftLegTransform));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);

	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, value_ptr(rightLegTransform));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);

	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, value_ptr(floorTransform));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
}


void Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (Current_DrawingMode)
	{
	case Points:
		glPointSize(10);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case Lines:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case FilledTriangle:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	default:
		break;
	}

	BindCube();
	CreateHumanOnFloor();
}


int main()
{
	sf::ContextSettings context;
	context.depthBits = 24;
	sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!", sf::Style::Close, context);

	if (Init()) return 1;

	// Initialize transformations with translation and scaling
	headTransform = translate(cameraTarget + vec3(0.0f, 0.8f, 0.0f)) * scale(vec3(0.3f));
	bodyTransform = translate(cameraTarget + vec3(0.0f, 0.3f, 0.0f)) * scale(vec3(0.5f, 0.7f, 0.3f));
	leftArmTransform = translate(cameraTarget + vec3(-0.4f, 0.5f, 0.0f)) * scale(vec3(0.2f, 0.5f, 0.2f));
	rightArmTransform = translate(cameraTarget + vec3(0.4f, 0.5f, 0.0f)) * scale(vec3(0.2f, 0.5f, 0.2f));
	leftLegTransform = translate(cameraTarget + vec3(-0.2f, -0.2f, 0.0f)) * scale(vec3(0.2f, 0.5f, 0.2f));
	rightLegTransform = translate(cameraTarget + vec3(0.2f, -0.2f, 0.0f)) * scale(vec3(0.2f, 0.5f, 0.2f));
	floorTransform = translate(cameraTarget + vec3(0.0f, -0.5f, 0.0f)) * scale(vec3(1.5f, 0.1f, 1.0f));
	//vec3 cameraPosition = vec3(0.0f, 0.0f, 3.0f);
	//vec3 cameraTarget = vec3(0.0f, 0.0f, 0.0f);
	//vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
	float y_axisControl = -90.0f;
	float x_axisControl = 0.0f;
	float sensitivity = 0.01f;
	bool rightMousePressed = false;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
			{
				window.close();
				break;
			}
			case sf::Event::KeyPressed:
			{
				if (event.key.code == sf::Keyboard::Num1)
				{
					Current_DrawingMode = DrawingMode::Points;
				}
				if (event.key.code == sf::Keyboard::Num2)
				{
					Current_DrawingMode = DrawingMode::Lines;
				}
				if (event.key.code == sf::Keyboard::Num3)
				{
					Current_DrawingMode = DrawingMode::FilledTriangle;
				}
				if (event.key.code == sf::Keyboard::F) 
				{
					cameraCurrentInput = CNone;
				}
				if (event.key.code == sf::Keyboard::W)
				{
					/*currentRotation = Up;*/
					cameraCurrentInput = Forward;
				}
				if (event.key.code == sf::Keyboard::S)
				{
					/*currentRotation = Down;*/
					cameraCurrentInput = Backward;
				}
				if (event.key.code == sf::Keyboard::A)
				{
					//currentRotation = Left;
					cameraCurrentInput = Leftward;
				}
				if (event.key.code == sf::Keyboard::D)
				{
					//currentRotation = Right;
					cameraCurrentInput = Rightward;
				}
				if (event.key.code == sf::Keyboard::Space) // Stop rotation
				{
					//currentRotation = None;
					cameraCurrentInput = Jump;
				}
				if (event.key.code == sf::Keyboard::LControl) 
				{
					cameraCurrentInput = Crouch;
				}
				if (event.key.code == sf::Keyboard::E)
				{
					cameraCurrentInput = RotateRight;
				}
				if (event.key.code == sf::Keyboard::Q)
				{
					cameraCurrentInput = RotateLeft;
				}
				if (event.key.code == sf::Keyboard::Z)
				{
					cameraCurrentInput = RotateUpward;
				}
				if (event.key.code == sf::Keyboard::C)
				{
					cameraCurrentInput = RotateDownward;
				}
				break;
			}
			case sf::Event::MouseButtonPressed:
			{
				if (event.mouseButton.button == sf::Mouse::Right) {
					rightMousePressed = true;
					sf::Mouse::setPosition(sf::Vector2i(WIDTH / 2, HEIGHT / 2), window);
				}
				break;
			}
			case sf::Event::MouseButtonReleased:
			{
				if (event.mouseButton.button == sf::Mouse::Right) {
					rightMousePressed = false;
				}
				break;
			}
			case sf::Event::MouseMoved:
			{
				if (rightMousePressed) {
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					int X = mousePos.x - WIDTH/2;
					int Y = HEIGHT/2 - mousePos.y;

					y_axisControl += X * sensitivity;
					x_axisControl += Y * sensitivity;
				}
				break;
			}
			}
		}
		if (rightMousePressed) {
			vec3 direction = vec3 (0,0,0);
			direction.x = cos(radians(y_axisControl)) * cos(radians(x_axisControl));
			direction.y = sin(radians(x_axisControl));
			direction.z = sin(radians(y_axisControl)) * cos(radians(x_axisControl));
			cameraTarget = cameraPosition + normalize(direction);
		}
		//cameraTarget = cameraPosition + glm::vec3(0.0f, 0.0f, -1.0f); // Update target to always look forward
		Update();
		Render();

		window.display();
	}
	return 0;
}


//void Render()
//{
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	switch (Current_DrawingMode)
//	{
//	case Points:
//		glPointSize(10);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
//		break;
//	case Lines:
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//		break;
//	case FilledTriangle:
//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//		break;
//	default:
//		break;
//	}
//
//	BindCube();
//
//	// draw cube
//	mat4 ModelMat = glm::translate(glm::vec3(-0.8, 0, -0.8f)) *
//		glm::rotate(theta * 180 / 3.14f, glm::vec3(0, 1, 0)) *
//		glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
//	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(ModelMat));
//	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
//
//	// draw triangle
//	ModelMat = glm::translate(glm::vec3(0.8, 0, 0.0f)) *
//	glm::rotate(theta * 180 / 3.14f, glm::vec3(0, 0, 1))*
//		glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
//	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(ModelMat));
//	
//	glDrawArrays(GL_TRIANGLES, 0, 3);
//
//}