#include "ResourceManager.h"

std::vector<Transform*> ResourceManager::Transforms;
std::vector<ShaderProgram*> ResourceManager::Shaders;
std::vector<Camera*> ResourceManager::Cameras;

void ResourceManager::addEntity(Transform * entity)
{
	Transforms.push_back(entity);
	if (Camera* camera = dynamic_cast<Camera*>(entity))
	{
		Cameras.push_back(camera);
	}
	else
	{

	}
}

void ResourceManager::addShader(ShaderProgram * shader)
{
	Shaders.push_back(shader);
}
