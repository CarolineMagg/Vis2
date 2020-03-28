#pragma once
#include <unordered_map>
#include <functional>
#include "SceneObject.h"
#include "camera\Camera.h"
#include "lights\Light.h"
#include "render\FBO.h"
#include "shading\SpecializedShaders.h"

template<typename T>
struct Job {
	float delay = 0.0f;
	std::function<T()> func;
};

template<typename T>
struct TweenVariableData {
	T* variable;	
	T end;
	float remainingTime;
};

class Scene
{
	using iterator = typename std::unordered_map<std::string, std::shared_ptr<SceneObject>>::iterator;
	using const_iterator = typename std::unordered_map<std::string, std::shared_ptr<SceneObject>>::const_iterator;
private:
		
	std::unordered_map<std::string, std::shared_ptr<SceneObject>> sceneObject;	
	const Camera& camera;
	std::shared_ptr<LightManager> lightManager;
	std::shared_ptr<MainShader> mainShader;		

	std::vector<Job<void>> jobsChain_void;
	std::vector<TweenVariableData<float>> tweeningFloats;

	
public:
	int msTest = 16;
	static unsigned int WIDTH_DEFAULT;
	static unsigned int HEIGHT_DEFAULT;

	Scene() = delete;
	Scene(const Camera& camera);
	void add(std::string key, std::shared_ptr<SceneObject>&& obj);
	SceneObject* get(std::string id);
	void render();
	void preRender(float timeDelta);		
	LightManager& getLightManager() { return *lightManager; };
	iterator begin() { return sceneObject.begin(); }
	iterator end() { return sceneObject.end(); }
	const_iterator begin() const { return sceneObject.begin(); }
	const_iterator end() const { return sceneObject.end(); }	

	void updateJobs(float deltaTime);		
	void addToJobChain(Job<void> &&job);
	void updateVariableTweens(float deltaTime);
	const float tweenUpdatesPerFrame = 1.0f / 60.0f;
	float pooledTweenTime = 0.0;
	
	//FBO mainRenderFBO{ true, true, true, 1000, 800, true };	
	FBO mainRenderFBO{ unsigned(3), true, true, WIDTH_DEFAULT, HEIGHT_DEFAULT };
	FBO contouringMask{ unsigned(1), false, true, WIDTH_DEFAULT, HEIGHT_DEFAULT };
	FBO volLightOutputFBO{ true, false, false, WIDTH_DEFAULT, HEIGHT_DEFAULT };
	FBO blurVerticalOutputFBO{ true, false, false, WIDTH_DEFAULT, HEIGHT_DEFAULT };
	FBO blurHorizontalOutputFBO{ true, false, false, WIDTH_DEFAULT, HEIGHT_DEFAULT };

	std::unique_ptr<VolumetricLightShader> volShader;
	Shader blurShader{ "base_vs.txt", "simple_blur_fs.txt" };
	Shader combineShader{ "base_vs.txt", "2tex_combine_fs.txt" };
	Shader contourShader{ "base_vs.txt", "contouring_fs.txt" };
	//Shader combineThresholdShader{ "base_vs.txt", "2tex_combine_threshold_fs.txt" };
	//Shader gaussBlur{ "base_vs.txt", "gauss_blur_fs.txt" };

	float blendMainTexture = 0.0f;
	float contourBlend = 0.0f;

	
};

