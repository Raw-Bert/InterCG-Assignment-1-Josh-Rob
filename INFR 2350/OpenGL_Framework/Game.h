#pragma once
#define NOMINMAX
#include <windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "Timer.h"
#include "Transform.h"
#include "Camera.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "IO.h"
#include "GameObject.h"
#include "UniformBuffer.h"
#include "Light.h"
#include "Framebuffer.h"

#define WINDOW_SCREEN_WIDTH		640
#define WINDOW_SCREEN_HEIGHT	432
#define WINDOW_WIDTH			960
#define WINDOW_HEIGHT			720
#define FRAMES_PER_SECOND		60

const int FRAME_DELAY_SPRITE = 1000 / FRAMES_PER_SECOND;

class Game
{
public:
	Game();
	~Game();

	void initializeGame();
	void update();
	void draw();
	void GUI();

	/* input callback functions */
	void keyboardDown(unsigned char key, int mouseX, int mouseY);
	void keyboardUp(unsigned char key, int mouseX, int mouseY);
	void keyboardSpecialDown(int key, int mouseX, int mouseY);
	void keyboardSpecialUp(int key, int mouseX, int mouseY);
	void mouseClicked(int button, int state, int x, int y);
	void mouseMoved(int x, int y);
	void mousePassive(int x, int y);
	void reshapeWindow(int w, int h);

	/* Data Members */
	Timer *updateTimer	= nullptr;
	float TotalGameTime = 0.0f;
	int windowWidth = WINDOW_WIDTH;
	int windowHeight = WINDOW_HEIGHT;

private:
	// Scene Objects.
	Camera camera;
	Camera camera2;

	Framebuffer framebuffer;
	Framebuffer framebufferTV;

	Mesh meshSphere;
	Mesh meshSkybox;
	Mesh meshLight;
	Mesh meshPlane;
	Mesh meshCube;
	Mesh meshStan;

	GameObject goStan;
	GameObject goSun;
	GameObject goEarth;
	GameObject goEarthPlane;
	GameObject goMoon;
	GameObject goJupiter;
	GameObject goJupiterMoon[2];
	GameObject goSaturn;
	GameObject goSaturnRings;
	GameObject goSkybox;
	GameObject goTV;

	GameObject goLight;

	std::vector<GameObject*> goPlanets;

	// OpenGL Handles
	ShaderProgram shaderBasic;
	ShaderProgram shaderTexture;
	ShaderProgram shaderSky;
	ShaderProgram shaderTextureAlphaDiscard;
	ShaderProgram shaderTextureJupiter;

	ShaderProgram shaderGbuffer;
	ShaderProgram shaderPointLight;

	ShaderProgram shaderPassthrough;
	ShaderProgram shaderGrayscale;

	UniformBuffer uniformBufferTime;
	UniformBuffer uniformBufferLightScene;
	UniformBuffer uniformBufferLight;
	Light light;
	UniformBuffer uniformBufferToon;
	std::vector<Texture*> textureToonRamp;

	bool guiEnabled = true;

	struct KeyInput
	{
		bool moveLeft = false;
		bool moveRight = false;
		bool moveUp = false;
		bool moveDown = false;
		bool moveForward = false;
		bool moveBackward = false;

		bool rotateUp = false;
		bool rotateDown = false;
		bool rotateLeft = false;
		bool rotateRight = false;


		bool ctrlL = false;
		bool ctrlR = false;
		bool shiftL = false;
		bool shiftR = false;
		bool altL = false;
		bool altR = false;

		vec2 mouseMovement = vec2(0.0f);
		vec2 mousePos = vec2(0.0f);
		vec2 mousePosOld = vec2(0.0f);
		vec2 mousePosGUI = vec2(10.0f, 20.0f);
		
		bool f11 = false;
	} input;
};
