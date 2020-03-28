#pragma once
#include "../Scene.h"
#include "../lights/Light.h"

class SceneBuilder
{
public:
	// prevent scene from moving around in memory
	static std::unique_ptr<Scene> buildScene(const Camera& camera);
private:
	SceneBuilder() = delete;
	~SceneBuilder() = delete;
};
