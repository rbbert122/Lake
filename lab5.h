#pragma once

#include <string>
#include <vector>

#include "components/simple_scene.h"
#include "components/transform.h"
#include "core/gpu/frame_buffer.h"
#include "core/gpu/particle_effect.h"


namespace m2
{
	struct LightInfo
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	class Lab5 : public gfxc::SimpleScene
	{
	public:
		Lab5();
		~Lab5();

		void Init() override;

	private:
		void FrameStart() override;
		void Update(float deltaTimeSeconds) override;
		void FrameEnd() override;

		void OnInputUpdate(float deltaTime, int mods) override;
		void OnKeyPress(int key, int mods) override;
		void OnKeyRelease(int key, int mods) override;
		void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
		void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
		void OnWindowResize(int width, int height) override;

		void LoadShader(const std::string& fileName);
		void LoadShader(const std::string& name, const std::string& VS, const std::string& FS, const std::string& GS, bool hasGeomtery);
		unsigned int UploadCubeMapTexture(const std::string& pos_x, const std::string& pos_y, const std::string& pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z);
		void RenderScene(glm::vec4 plane);
		void RenderWater();
		void CreateFramebuffer(int width, int height, unsigned int& framebuffer_object, unsigned int& color_texture, unsigned int& depth_texture);
		void ResetParticlesRainSnow(int xSize, int ySize, int zSize);
	private:
		std::vector<LightInfo> lights;
		unsigned int cubeMapTextureID;
		unsigned int reflection_framebuffer;
		unsigned int reflection_color_texture;
		unsigned int reflection_depth_texture;
		unsigned int refraction_framebuffer;
		unsigned int refraction_color_texture;
		unsigned int refraction_depth_texture;
		float move_factor;
		glm::vec3 generator_position;
		float offset;
	};
}   // namespace m2
