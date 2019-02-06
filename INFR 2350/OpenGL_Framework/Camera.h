#pragma once
#include "GameObject.h"
#include "UniformBuffer.h"
#include "Framebuffer.h"
#include <vector>

/*
  ////////////
 // Camera //
////////////

The Camera class uses the transform class to position the View Matrix, along 
with tracking which objects should be drawn, and whether they should be culled

*/

enum ProjectionType
{
	Perspective,
	Orthographic
};

enum class ClearFlag
{
	Skybox,
	SolidColor,
	DepthOnly,
	NoClear
};

class Camera : public Transform
{
public:	
	Camera();
	Camera(ProjectionType projType);
	~Camera();

	void perspective(
		float fovy, float aspect, 
		float zNear, float zFar);

	void orthographic(
		float left, float right,
		float bottom, float top,
		float zNear, float zFar);

	void init();
	mat4 getView() const;
	mat4 getViewProjection() const;
	mat4 getProjection() const;
	mat4* getViewProjectionPtr();
	void update(float dt);
	void sendUBO() const;
	void draw();
	void render();
	void clear();

	void cull();
	void sort();

	void attachFrameBuffer(Framebuffer* fb);
	
	void setRenderList(std::vector<Transform*> objects);
	void addToRenderList(std::vector<Transform*> objects);

	bool cullingActive = false;
	GameObject* m_pSkybox;
private:
	mat4 m_pProjection;
	mat4 m_pViewMatrix;
		
	vec4 m_pOrthoSize;
	vec2 m_pFov;
	float m_pAspectRatio;
	float m_pNear;
	float m_pFar;
	ProjectionType m_pProjectionType = ProjectionType::Perspective;
public:
	ClearFlag m_pClearFlag = ClearFlag::SolidColor;
	vec4 m_pClearColor = vec4(0, 0, 0, 0);
private:

	std::vector<Transform*> objectList;
	std::vector<Transform*> cullList;

	UniformBuffer m_pUBO;
	Framebuffer* m_pFB;
};