#include "Game.h"
#include "ResourceManager.h"
#include "TextureCube.h"
#include "UI.h"

#include <vector>
#include <string>
#include <fstream>
#include <random>

Game::Game()
{
	updateTimer = new Timer();
}

Game::~Game()
{
	delete updateTimer;
}

int activeToonRamp = 0;
bool toonActive = false;

constexpr int frameTimeNumSamples = 600;
int frameTimeCurrSample = 0;
float frameTimeSamples[frameTimeNumSamples];

UniformBuffer* testBuffer;

void Game::initializeGame()
{
	for (int i = 0; i < frameTimeNumSamples; ++i)
		frameTimeSamples[i] = 0.016f;
	if(guiEnabled && !UI::isInit)
		UI::InitImGUI();

	ShaderProgram::initDefault();
	Framebuffer::initFrameBuffers();
	framebuffer.addDepthTarget();
	framebuffer.addColorTarget(GL_RGB8);
	framebuffer.init(windowWidth, windowHeight);
	framebufferTV.addDepthTarget();
	framebufferTV.addColorTarget(GL_RGB8);
	framebufferTV.init(128, 128);

	meshStan.LoadFromObj("stan_big.obj");
	meshSphere.initMeshSphere(32U, 32U);
	meshSkybox.initMeshSphere(32U, 32U, true);
	meshLight.initMeshSphere(6U, 6U);
	meshPlane.initMeshPlane(32U, 32U);
	
	shaderBasic.load("shader.vert", "shader.frag");
	shaderTexture.load("shader.vert", "shaderTexture.frag");
	shaderTextureJupiter.load("shader.vert", "shaderTextureJupiter.frag");
	shaderTextureAlphaDiscard.load("shader.vert", "shaderTextureAlphaDiscard.frag");
	shaderSky.load("shaderSky.vert", "shaderSky.frag");
	shaderPassthrough.load("PassThrough.vert", "PassThrough.frag");
	shaderGrayscale.load("PassThrough.vert", "Post/GreyscalePost.frag");

	ResourceManager::Shaders.push_back(&shaderBasic);
	ResourceManager::Shaders.push_back(&shaderTexture);
	ResourceManager::Shaders.push_back(&shaderTextureJupiter);
	ResourceManager::Shaders.push_back(&shaderTextureAlphaDiscard);
	ResourceManager::Shaders.push_back(&shaderSky);
	ResourceManager::Shaders.push_back(&shaderPassthrough);
	ResourceManager::Shaders.push_back(&shaderGrayscale);
	
	uniformBufferTime.allocateMemory(sizeof(float));
	uniformBufferTime.bind(1);
	uniformBufferLightScene.allocateMemory(sizeof(vec4));
	uniformBufferLightScene.bind(2);
	
	light.init();
	light._UBO.bind(3);

	uniformBufferToon.allocateMemory(sizeof(int) * 4);
	uniformBufferToon.bind(5);
	uniformBufferToon.sendBool(false, 0);

	uniformBufferLightScene.sendVector(vec3(0.05f), 0);
	//uniformBufferLight.sendVector(vec3(1.0f), sizeof(vec4)); // Set color of light to white

	Texture* texBlack = new Texture("black.png");
	Texture* texWhite = new Texture("white.png");
	Texture* texYellow = new Texture("yellow.png");
	//Texture* texGray = new Texture("gray.png");
	Texture* texEarthAlbedo = new Texture("earth.jpg");
	Texture* texEarthEmissive = new Texture("earthEmissive.png");
	Texture* texEarthSpecular = new Texture("earthSpec.png");
	Texture* texRings = new Texture("saturnRings.png");
	//Texture* texMoonAlbedo = new Texture("8k_moon.jpg");
	Texture* texJupiterAlbedo = new Texture("jupiter.png");
	Texture* texSaturnAlbedo = new Texture("8k_saturn.jpg");
	Texture* texCheckerboard = new Texture("checkboard.png");
	Texture* texStanAlbedo = new Texture("stan_tex.png");
	Texture* texStanEmissive = new Texture("stan_emit.png");

	textureToonRamp.push_back(new Texture("TF2.JPG", false));
	textureToonRamp[0]->setWrapParameters(GL_CLAMP_TO_EDGE);
	textureToonRamp[0]->sendTexParameters();

	std::vector<Texture*> texEarth = { texEarthAlbedo, texEarthEmissive, texEarthSpecular };
	std::vector<Texture*> texCheckboards { texCheckerboard, texBlack, texWhite };
	std::vector<Texture*> texSun = { texBlack, texYellow, texBlack };
	//std::vector<Texture*> texMoon = { texMoonAlbedo, texBlack, texBlack };
	std::vector<Texture*> texJupiter = { texJupiterAlbedo, texBlack, texBlack };
	std::vector<Texture*> texPlanet = { texWhite, texBlack, texBlack };
	std::vector<Texture*> texSaturn = { texSaturnAlbedo, texBlack, texBlack };
	std::vector<Texture*> texSaturnRings = { texRings, texBlack, texBlack };
	std::vector<Texture*> texStan = { texStanAlbedo, texStanEmissive, texBlack };
	//std::vector<Texture*> texTV = { texBlack, &framebufferTV._Color._Tex[0] , texBlack };

	goStan = GameObject(&meshStan, texStan);
	goSun = GameObject(&meshSphere, texSun);
	goEarth = GameObject(&meshSphere, texEarth);
	goEarthPlane = GameObject(&meshPlane, texCheckboards);
	//goMoon = GameObject(&meshSphere, texMoon);
	goJupiter = GameObject(&meshSphere, texJupiter);
	//goJupiterMoon[0] = GameObject(&meshSphere, texMoon);
	//goJupiterMoon[1] = GameObject(&meshSphere, texMoon);
	goSaturn = GameObject(&meshSphere, texSaturn);
	goSaturnRings = GameObject(&meshPlane, texSaturnRings);
	//goTV = GameObject(&meshPlane, texTV);

	std::vector<std::string> skyboxTex;
	skyboxTex.push_back("sky2/sky_c00.bmp");
	skyboxTex.push_back("sky2/sky_c01.bmp");
	skyboxTex.push_back("sky2/sky_c02.bmp");
	skyboxTex.push_back("sky2/sky_c03.bmp");
	skyboxTex.push_back("sky2/sky_c04.bmp");
	skyboxTex.push_back("sky2/sky_c05.bmp");
	goSkybox = GameObject(&meshSkybox, new TextureCube(skyboxTex));
	goSkybox.setShaderProgram(&shaderSky);
	camera2.m_pSkybox = camera.m_pSkybox = &goSkybox;

	ResourceManager::addEntity(&goStan);
	ResourceManager::addEntity(&goSun);
	ResourceManager::addEntity(&goEarth);
	ResourceManager::addEntity(&goEarthPlane);
	//ResourceManager::addEntity(&goMoon);
	ResourceManager::addEntity(&goJupiter);
	//ResourceManager::addEntity(&goJupiterMoon[0]);
	//ResourceManager::addEntity(&goJupiterMoon[1]);
	ResourceManager::addEntity(&goSaturn);
	ResourceManager::addEntity(&goSaturnRings);
	ResourceManager::addEntity(&camera);
	ResourceManager::addEntity(&camera2);
	//ResourceManager::addEntity(&goTV);	

	//goLight.setShaderProgram(&shaderPointLight);
	//goLight.setMesh(&meshSphere);
	//goLight.setTextures(gbuffer.textures);

	goStan.setLocalPos(vec3(-4.0f, 10.0f, 0.0f));
	goSun.setLocalPos(vec3(4, 5, 0));
	goEarth.setLocalPos(vec3(-2, 0, 0));
	goEarthPlane.setLocalPos(vec3(0, -5.0f, -50));
	//goMoon.setLocalPos(vec3(-1, 0, -1));
	goJupiter.setLocalPos(vec3(-3, 0, 4));
	//goJupiterMoon[0].setLocalPos(vec3(-4, 0, 5));
	//goJupiterMoon[1].setLocalPos(vec3(-2, 0, 3));
	goSaturn.setLocalPos(vec3(-2, 0, -3));
	goSaturnRings.setLocalPos(vec3(-2, 0, -3));

	std::uniform_real_distribution<float> randomPositionX(-100.0f, 100.0f);
	std::uniform_real_distribution<float> randomPositionY(-100.0f, 100.0f);
	std::uniform_real_distribution<float> randomPositionZ(-100.0f, -10.0f);
	std::uniform_real_distribution<float> randomRotation(0.0f, 360.0f);
	std::uniform_real_distribution<float> randomScale(0.5f, 4.0f);
	std::default_random_engine generator(std::_Random_device());

	//for (int i = 0; i < 500; i++)
	//{
	//	GameObject *object = new GameObject(&meshSphere, texMoon);
	//	object->setLocalPos(vec3(randomPositionX(generator), randomPositionY(generator), randomPositionZ(generator)));
	//	object->setScale(vec3(randomScale(generator)));
	//	object->setLocalRot(vec3(randomRotation(generator), randomRotation(generator), randomRotation(generator)));
	//	object->setShaderProgram(&shaderTexture);
	//	ResourceManager::addEntity(object);
	//	goPlanets.push_back(object);
	//

	goStan.setScale(2.0f);
	goSun.setScale(1.50f);
	goEarth.setScale(0.50f);
	goEarthPlane.setScale(100.50f);
	//goMoon.setScale(0.25f);
	goJupiter.setScale(1.00f);
	//goJupiterMoon[0].setScale(0.25f);
	//goJupiterMoon[1].setScale(0.20f);
	goSaturn.setScale(1.0f);
	goSaturnRings.setScale(2.0f);
	goSaturnRings.setLocalRotZ(-20.0f);

	//goTV			.setShaderProgram(&shaderTexture);
	goStan			.setShaderProgram(&shaderTexture);
	goSun			.setShaderProgram(&shaderTexture);
	goEarth			.setShaderProgram(&shaderTexture);
	goEarthPlane	.setShaderProgram(&shaderTexture);
	//goMoon			.setShaderProgram(&shaderTexture);
	goJupiter		.setShaderProgram(&shaderTextureJupiter);
	//goJupiterMoon[0].setShaderProgram(&shaderTexture);
	//goJupiterMoon[1].setShaderProgram(&shaderTexture);
	goSaturn		.setShaderProgram(&shaderTexture);
	goSaturnRings	.setShaderProgram(&shaderTextureAlphaDiscard);
	   	 
	goSun.addChild(&goEarth);
	// These Render flags can be set once at the start (No reason to waste time calling these functions every frame).
	// Tells OpenGL to respect the depth of the scene. Fragments will not render when they are behind other geometry.
	glEnable(GL_DEPTH_TEST); 
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	   
	// Setup our main scene objects...
	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	camera.perspective(90.0f, aspect, 0.05f, 1000.0f);
	camera.setLocalPos(vec3(0.0f, 4.0f, 4.0f));
	camera.setLocalRotX(-15.0f);
	camera.attachFrameBuffer(&framebuffer);
	camera.m_pClearFlag = ClearFlag::SolidColor;
	camera.setRenderList(ResourceManager::Transforms);

	camera2.perspective(90.0f, aspect, 0.05f, 1000.0f);
	//camera2.setLocalPos();
	camera2.setLocalRotX(-15.0f);
	camera2.setLocalRotY(180.0f);
	camera2.attachFrameBuffer(&framebufferTV);
	camera2.m_pClearFlag = ClearFlag::Skybox;
	camera2.setRenderList(ResourceManager::Transforms);

	testBuffer = new UniformBuffer(32);
	testBuffer->bind(6);
	testBuffer->sendVector(vec4(0.0f, 0.25f, 0.5f, 0.75f), 1);
}

void Game::update()
{
	// update our clock so we have the delta time since the last update
	updateTimer->tick();

	float deltaTime = updateTimer->getElapsedTimeSeconds();
	TotalGameTime += deltaTime;

	frameTimeSamples[frameTimeCurrSample] = min(deltaTime, 0.1f);
	frameTimeCurrSample = (frameTimeCurrSample + 1) % frameTimeNumSamples;
	frameTimeSamples[frameTimeCurrSample] = 1.0f;

#pragma region movementCode
	float cameraSpeedMult = 4.0f;
	float cameraRotateSpeed = 90.0f;
	float cameraMouseSpeed = 0.15f;

	if (input.shiftL || input.shiftR)
	{
		cameraSpeedMult *= 4.0f;
	}

	if (input.ctrlL || input.ctrlR)
	{
		cameraSpeedMult *= 0.5f;
	}

	posX = goStan.getLocalPos().x * cos(TotalGameTime) *3;
	posY = goStan.getLocalPos().y * sin(TotalGameTime) / 2;
	posZ = goStan.getLocalPos().z * sin(TotalGameTime);
	goSun.setLocalPos(vec3(posX, posY,posZ));

	//TODO: Rotate Earth, add more "Interesting stuff"

	if (input.moveUp)
	{
		camera.m_pLocalPosition += camera.m_pLocalRotation.up() * cameraSpeedMult * deltaTime;
	}
	if (input.moveDown)
	{
		camera.m_pLocalPosition -= camera.m_pLocalRotation.up() * cameraSpeedMult * deltaTime;
	}
	if (input.moveForward)
	{
		camera.m_pLocalPosition -= camera.m_pLocalRotation.forward() * cameraSpeedMult * deltaTime;
	}
	if (input.moveBackward)
	{
		camera.m_pLocalPosition += camera.m_pLocalRotation.forward() * cameraSpeedMult * deltaTime;
	}
	if (input.moveRight)
	{
		camera.m_pLocalPosition += camera.m_pLocalRotation.right() *  cameraSpeedMult * deltaTime;
	}
	if (input.moveLeft)
	{
		camera.m_pLocalPosition -= camera.m_pLocalRotation.right() * cameraSpeedMult * deltaTime;
	}
	if (input.rotateUp)
	{
		camera.m_pLocalRotationEuler.x += cameraRotateSpeed * deltaTime;
		camera.m_pLocalRotationEuler.x = min(camera.m_pLocalRotationEuler.x, 85.0f);
	}
	if (input.rotateDown)
	{
		camera.m_pLocalRotationEuler.x -= cameraRotateSpeed * deltaTime;
		camera.m_pLocalRotationEuler.x = max(camera.m_pLocalRotationEuler.x, -85.0f);
	}
	if (input.rotateRight)
	{
		camera.m_pLocalRotationEuler.y -= cameraRotateSpeed * deltaTime;
	}
	if (input.rotateLeft)
	{
		camera.m_pLocalRotationEuler.y += cameraRotateSpeed * deltaTime;
	}
	if (!guiEnabled)
	{
		camera.m_pLocalRotationEuler.x -= cameraMouseSpeed * input.mouseMovement.y;
		camera.m_pLocalRotationEuler.y -= cameraMouseSpeed * input.mouseMovement.x;
		camera.m_pLocalRotationEuler.x = clamp(camera.m_pLocalRotationEuler.x, -85.0f, 85.0f);
		input.mouseMovement = vec2(0.0f);
	}
#pragma endregion movementCode

	camera2.setLocalPos(vec3(sin(TotalGameTime) * 10.0f, sin(TotalGameTime * 4) + 4.0f, -50.0f));

	// Give the earth some motion over time.
	goEarth.setLocalRotY(TotalGameTime * 15.0f);
	// Give our Transforms a chance to compute the latest matrices
	for (Transform* object : ResourceManager::Transforms)
	{
		object->update(deltaTime);
	}
	goSkybox.update(deltaTime);
}

void Game::draw()
{
	uniformBufferTime.sendFloat(TotalGameTime, 0);
	glClear(GL_DEPTH_BUFFER_BIT);

	textureToonRamp[0]->bind(31);


	// Render the scene from both camera's perspective
	// this will render all objects that are in the camera list
	light.position = camera.getView() * vec4(goSun.getWorldPos(), 1.0f);
	light.update(0.0f);
	camera.render();
	light.position = camera2.getView() * vec4(goSun.getWorldPos(), 1.0f);
	light.update(0.0f);
	camera2.render();
	
	shaderGrayscale.bind();
	framebuffer.bindColorAsTexture(0, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	framebuffer.drawFSQ();
	ShaderProgram::unbind();
	framebuffer.unbindTexture(0);

	if(guiEnabled)
		GUI();	

	// Commit the Back-Buffer to swap with the Front-Buffer and be displayed on the monitor.
	glutSwapBuffers();
}

void Game::GUI()
{
	UI::Start(windowWidth, windowHeight);
	// Framerate Visualizer
	ImGui::Begin("Framerate");
	{
		float averageFramerate = 0.0f;
		for (int i = 0; i < frameTimeNumSamples; ++i)
			averageFramerate += frameTimeSamples[i];
		averageFramerate = 1.0f / ((averageFramerate-1.0f) / (frameTimeNumSamples-1));
		std::string framerate = "Framerate: " + std::to_string(averageFramerate);

		ImGui::PlotHistogram("", frameTimeSamples, frameTimeNumSamples, 0, framerate.c_str(), 0.0f, 0.1f, ImVec2(frameTimeNumSamples, 60));
	}
	ImGui::End();

	static float grayscaleAmount = 1.0f;
	if (ImGui::SliderFloat("Grayscale Amount", &grayscaleAmount, 0.0f, 1.0f))
	{
		shaderGrayscale.bind();
		shaderGrayscale.sendUniform("uAmount", grayscaleAmount);
		shaderGrayscale.unbind();
	}

	if (ImGui::Checkbox("Toon Shading Active", &toonActive))
	{
		uniformBufferToon.sendUInt(toonActive, 0);
	}
	if (ImGui::SliderInt("Toon Ramp Selection", &activeToonRamp, 0, (int)textureToonRamp.size() - 1))
	{
		//activeToonRamp = clamp(activeToonRamp, 0, (int)textureToonRamp.size() - 1);
	}

	static vec3 lightPosition = goSun.getLocalPos();
	if (ImGui::DragFloat3("Light Position", &lightPosition[0], 0.5f))
	{
		goSun.setLocalPos(lightPosition);
	}

	if (ImGui::SliderFloat3("Attenuation", &light.constantAtten, 0.0f, 1.0f, "%.8f", 6.0f))
	{
		
	}

	ImGui::Text("Radius: %f", light.radius);

	UI::End();
}

void Game::keyboardDown(unsigned char key, int mouseX, int mouseY)
{
	if (guiEnabled)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[(int)key] = true;
		io.AddInputCharacter((int)key); // this is what makes keyboard input work in imgui
		// This is what makes the backspace button work
		int keyModifier = glutGetModifiers();
		switch (keyModifier)
		{
		case GLUT_ACTIVE_SHIFT:
			io.KeyShift = true;
			break;

		case GLUT_ACTIVE_CTRL:
			io.KeyCtrl = true;
			break;

		case GLUT_ACTIVE_ALT:
			io.KeyAlt = true;
			break;
		}
	}

	switch(key)
	{
	case 27: // the escape key
		break;
	case 'w':
	case 'W':
	case 'w' - 96:
		input.moveForward = true;
		break;
	case 's':
	case 'S':
	case 's' - 96:
		input.moveBackward = true;
		break;
	case 'd':
	case 'D':
	case 'd' - 96:
		input.moveRight = true;
		break;
	case 'a':
	case 'A':
	case 'a' - 96:
		input.moveLeft = true;
		break;
	case 'e':
	case 'E':
	case 'e' - 96:
		input.moveUp = true;
		break;
	case 'q':
	case 'Q':
	case 'q' - 96:
		input.moveDown = true;
		break;
	case 'l':
	case 'L':
	case 'l' - 96:
		input.rotateRight = true;
		break;
	case 'j':
	case 'J':
	case 'j' - 96:
		input.rotateLeft = true;
		break;
	case 'i':
	case 'I':
	case 'i' - 96:
		input.rotateUp = true;
		break;
	case 'k':
	case 'K':
	case 'k' - 96:
		input.rotateDown = true;
		break;
	}
}

void Game::keyboardUp(unsigned char key, int mouseX, int mouseY)
{
	if (guiEnabled)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[key] = false;

		int keyModifier = glutGetModifiers();
		io.KeyShift = false;
		io.KeyCtrl = false;
		io.KeyAlt = false;
		switch (keyModifier)
		{
		case GLUT_ACTIVE_SHIFT:
			io.KeyShift = true;
			break;

		case GLUT_ACTIVE_CTRL:
			io.KeyCtrl = true;
			break;

		case GLUT_ACTIVE_ALT:
			io.KeyAlt = true;
			break;
		}
	}

	switch(key)
	{
	case 32: // the space bar
		camera.cullingActive = !camera.cullingActive;
		break;
	case 27: // the escape key
		exit(1);
		break;
	case 'w':
	case 'W':
	case 'w' - 96:
		input.moveForward = false;
		break;
	case 's':
	case 'S':
	case 's' - 96:
		input.moveBackward = false;
		break;
	case 'd':
	case 'D':
	case 'd' - 96:
		input.moveRight = false;
		break;
	case 'a':
	case 'A':
	case 'a' - 96:
		input.moveLeft = false;
		break;
	case 'e':
	case 'E':
	case 'e' - 96:
		input.moveUp = false;
		break;
	case 'q':
	case 'Q':
	case 'q' - 96:
		input.moveDown = false;
		break;
	case 'l':
	case 'L':
	case 'l' - 96:
		input.rotateRight = false;
		break;
	case 'j':
	case 'J':
	case 'j' - 96:
		input.rotateLeft = false;
		break;
	case 'i':
	case 'I':
	case 'i' - 96:
		input.rotateUp = false;
		break;
	case 'k':
	case 'K':
	case 'k' - 96:
		input.rotateDown = false;
		break;
	}
}

void Game::keyboardSpecialDown(int key, int mouseX, int mouseY)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		guiEnabled = !guiEnabled;
		if (guiEnabled)
		{
			glutWarpPointer((int)input.mousePosGUI.x, (int)input.mousePosGUI.y);
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		else 
		{
			input.f11 = true;
			glutWarpPointer(windowWidth / 2, windowHeight / 2);
			glutSetCursor(GLUT_CURSOR_NONE);
		}
		if (!UI::isInit)
		{
			UI::InitImGUI();
		}
		break;
	case GLUT_KEY_F5:
		for (ShaderProgram* shader : ResourceManager::Shaders)
		{
			shader->reload();
		}
		break;
	case GLUT_KEY_CTRL_L:
		input.ctrlL = true;
		break;
	case GLUT_KEY_CTRL_R:
		input.ctrlR = true;
		break;
	case GLUT_KEY_SHIFT_L:
		input.shiftL = true;
		break;
	case GLUT_KEY_SHIFT_R:
		input.shiftR = true;
		break;
	case GLUT_KEY_ALT_L:
		input.altL = true;
		break;
	case GLUT_KEY_ALT_R:
		input.altR = true;
		break;
	case GLUT_KEY_UP:
		input.moveForward = true;
		break;
	case GLUT_KEY_DOWN:
		input.moveBackward = true;
		break;
	case GLUT_KEY_RIGHT:
		input.moveRight = true;
		break;
	case GLUT_KEY_LEFT:
		input.moveLeft = true;
		break;
	case GLUT_KEY_PAGE_UP:
		input.moveUp = true;
		break;
	case GLUT_KEY_PAGE_DOWN:
		input.moveDown = true;
		break;
	case GLUT_KEY_END:
		exit(1);
		break;
	}
}

void Game::keyboardSpecialUp(int key, int mouseX, int mouseY)
{
	switch (key)
	{
	case GLUT_KEY_CTRL_L:
		input.ctrlL = false;
		break;
	case GLUT_KEY_CTRL_R:
		input.ctrlR = false;
		break;
	case GLUT_KEY_SHIFT_L:
		input.shiftL = false;
		break;
	case GLUT_KEY_SHIFT_R:
		input.shiftR = false;
		break;
	case GLUT_KEY_ALT_L:
		input.altL = false;
		break;
	case GLUT_KEY_ALT_R:
		input.altR = false;
		break;
	case GLUT_KEY_UP:
		input.moveForward = false;
		break;
	case GLUT_KEY_DOWN:
		input.moveBackward = false;
		break;
	case GLUT_KEY_RIGHT:
		input.moveRight = false;
		break;
	case GLUT_KEY_LEFT:
		input.moveLeft = false;
		break;
	case GLUT_KEY_PAGE_UP:
		input.moveUp = false;
		break;
	case GLUT_KEY_PAGE_DOWN:
		input.moveDown = false;
		break;
	}
}

void Game::mouseClicked(int button, int state, int x, int y)
{
	if (guiEnabled)
	{
		ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);
		ImGui::GetIO().MouseDown[0] = !state;
	}
	else
	{
	}

	if(state == GLUT_DOWN) 
	{
		switch(button)
		{
		case GLUT_LEFT_BUTTON:

			break;
		case GLUT_RIGHT_BUTTON:
		
			break;
		case GLUT_MIDDLE_BUTTON:

			break;
		}
	}
	else
	{

	}
}

void Game::mouseMoved(int x, int y)
{
	if (guiEnabled)
	{
		ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);

		if (!ImGui::GetIO().WantCaptureMouse)
		{

		}
	}

	
}

void Game::mousePassive(int x, int y)
{
	if (!guiEnabled)
	{
		if (input.f11)
		{
			//glutWarpPointer(windowWidth / 2, windowHeight / 2);
			input.f11 = false;
			input.mousePos = vec2((float)(windowWidth / 2), (float)(windowHeight / 2));
			input.mousePosOld = input.mousePos;
		}
		else
		{
			//input.mousePosOld = vec2((float)(windowWidth / 2), (float)(windowHeight / 2));
			input.mousePos = vec2((float)x, (float)y);
			input.mouseMovement += input.mousePos - input.mousePosOld;
			input.mousePosOld = input.mousePos;
		}

		if (x < 100 || y < 100 || x > windowWidth - 100 || y > windowHeight - 100)
		{
			glutWarpPointer(windowWidth / 2, windowHeight / 2);
			input.mousePosOld = vec2((float)(windowWidth / 2), (float)(windowHeight / 2));
			input.f11 = true;
		}
	}
	else
	{
		input.mousePosGUI = vec2((float)x, (float)y);
	}
}

void Game::reshapeWindow(int w, int h)
{
	windowWidth = w;
	windowHeight = h;

	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	camera.perspective(90.0f, aspect, 0.05f, 1000.0f);
	glViewport(0, 0, w, h);
	framebuffer.reshape(w, h);
}
