#include "lab_m2/lab5/lab5.h"

#include <vector>
#include <iostream>

#include "glm/gtc/noise.hpp"
#include "stb/stb_image.h"

using namespace std;
using namespace m2;


//Generates a random value between 0 and 1.
inline float Rand01()
{
	return rand() / static_cast<float>(RAND_MAX);
}

void __stdcall glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

struct Particle_v2
{
	glm::vec4 position;
	glm::vec4 speed;
	glm::vec4 initialPos;
	glm::vec4 initialSpeed;
	float delay;
	float initialDelay;
	float lifetime;
	float initialLifetime;
	float t;

	Particle_v2() {}

	Particle_v2(const glm::vec4& pos, const glm::vec4& speed)
	{
		SetInitial(pos, speed);
	}

	void SetInitial(const glm::vec4& pos, const glm::vec4& speed,
		float delay = 0, float lifetime = 0)
	{
		position = pos;
		initialPos = pos;

		this->speed = speed;
		initialSpeed = speed;

		this->delay = delay;
		initialDelay = delay;

		this->lifetime = lifetime;
		initialLifetime = lifetime;

		t = 0;
	}
};


ParticleEffect<Particle_v2>* particleEffect;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab5::Lab5()
{
}


Lab5::~Lab5()
{
}

// Function to create circular points around a center point
vector<glm::vec3> CreateCirclePoints(const glm::vec3& center, const glm::vec3& up, float radius, int segments) {
	vector<glm::vec3> points;
	glm::vec3 right = glm::normalize(glm::cross(up, glm::vec3(0, 0, 1)));
	glm::vec3 forward = glm::normalize(glm::cross(right, up));

	for (int i = 0; i < segments; i++) {
		float angle = (2.0f * glm::pi<float>() * i) / segments;
		glm::vec3 point = center + radius * (right * cos(angle) + forward * sin(angle));
		points.push_back(point);
	}
	return points;
}

// Function to calculate point on cubic Bézier curve
glm::vec3 CubicBezierPoint(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) {
	float oneMinusT = 1.0f - t;
	return oneMinusT * oneMinusT * oneMinusT * p0 +
		3.0f * oneMinusT * oneMinusT * t * p1 +
		3.0f * oneMinusT * t * t * p2 +
		t * t * t * p3;
}

void CreateCubicBezierMesh(unordered_map<string, Mesh*>& meshes, const string& name,
	const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
	const int curveSegments = 20;  // Number of segments along the curve
	const int circleSegments = 8;  // Number of segments around the tube
	const float tubeRadius = 0.1f; // Radius of the tube

	vector<VertexFormat> vertices;
	vector<unsigned int> indices;

	// Generate vertices
	for (int i = 0; i <= curveSegments; i++) {
		float t = float(i) / curveSegments;

		// Get point on curve
		glm::vec3 curvePoint = CubicBezierPoint(p0, p1, p2, p3, t);

		// Calculate tangent for up vector
		float tNext = glm::min(1.0f, t + 0.01f);
		glm::vec3 nextPoint = CubicBezierPoint(p0, p1, p2, p3, tNext);
		glm::vec3 tangent = glm::normalize(nextPoint - curvePoint);

		// Generate circle points around curve point
		vector<glm::vec3> circlePoints = CreateCirclePoints(curvePoint, tangent, tubeRadius, circleSegments);

		// Add vertices with colors
		for (const auto& point : circlePoints) {
			// Create varying colors based on position
			glm::vec3 color(0.5f + 0.5f * sin(t * glm::pi<float>()),
				0.5f + 0.5f * cos(t * glm::pi<float>()),
				0.5f + 0.5f * sin(2.0f * t * glm::pi<float>()));
			vertices.push_back(VertexFormat(point, color));
		}
	}

	// Generate indices for triangles
	for (int i = 0; i < curveSegments; i++) {
		for (int j = 0; j < circleSegments; j++) {
			int current = i * circleSegments + j;
			int next = i * circleSegments + (j + 1) % circleSegments;
			int current_next = (i + 1) * circleSegments + j;
			int next_next = (i + 1) * circleSegments + (j + 1) % circleSegments;

			// First triangle
			indices.push_back(current);
			indices.push_back(next);
			indices.push_back(current_next);

			// Second triangle
			indices.push_back(next);
			indices.push_back(next_next);
			indices.push_back(current_next);
		}
	}

	// Create the mesh
	meshes[name] = new Mesh(name);
	meshes[name]->InitFromData(vertices, indices);
}

glm::vec3 bezier(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
{
	return (1 - t) * (1 - t) * (1 - t) * p0 + 3 * (1 - t) * (1 - t) * t * p1 + 3 * (1 - t) * t * t * p2 + t * t * t * p3;
}

float clamp(float x, float a, float b)
{
	return max(a, min(b, x));
}

void Lab5::Init()
{
	TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "ground.jpg");
	TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "waterDUDV.png");
	TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "normal.png");
	TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "rain.png");

	// Load a mesh from file into GPU memory
	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("plane");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	// Load a mesh from file into GPU memory
	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("quad");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		glm::vec3 p0 = glm::vec3(51, 20, 30);
		glm::vec3 p1 = glm::vec3(35, 7, 28);
		glm::vec3 p2 = glm::vec3(24, 4, 18);
		glm::vec3 p3 = glm::vec3(4, -3, 4);
		float t = 0.0;
		float step = 0.1;
		vector<glm::vec2> bezier_projection;
		for (int i = 0; i <= 10; i++)
		{
			glm::vec3 point = bezier(p0, p1, p2, p3, t);
			t += step;
			bezier_projection.push_back(glm::vec2(point.x, point.z));
		}

		vector <VertexFormat> vertices;
		vector <unsigned int> indices;
		int m = 100;
		int n = 100;
		float size = 50.0f;

		float half = size / 2.0f;
		float stepX = size / (float)m;
		float stepZ = size / (float)n;

		float h_max = 7.5f;
		float r = 12.5f;
		glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);

		float r_cascada = 3.0f;

		for (int i = 0; i <= m; i++)
		{
			for (int j = 0; j <= n; j++)
			{
				float x = -half + i * stepX;
				float z = -half + j * stepZ;
				float y = 0.0f;
				glm::vec2 text_coord = glm::vec2(i / (float)m, j / (float)n);
				float d = glm::distance(glm::vec3(x, 0.0f, z), center) / r;
				if (d < 1)
				{
					y = d * d / 2.0f * h_max;
				}
				else {
					y = (1 - (2 - d) * (2 - d) / 2.0f) * h_max;
				}

				glm::vec2 st = glm::vec2(4.0f * x / size, 4.0f * z / size);
				float noise = glm::simplex(st);
				noise = noise * d * d / 1.0f;
				y = y + noise;
				y = y - 2.5f;

				float d_bezier = 1000;
				float min_t = 0;
				for (int b = 0; b < bezier_projection.size() - 1; b++)
				{
					glm::vec2 p0 = bezier_projection[b];
					glm::vec2 p1 = bezier_projection[b + 1];
					glm::vec2 d = p1 - p0;
					glm::vec2 f = glm::vec2(x, z) - p0;

					float denom = glm::dot(d, d);
					float t_prime = glm::dot(f, d) / denom;

					t_prime = glm::clamp(t_prime, 0.0f, 1.0f);
					glm::vec2 closest_point = p0 + t_prime * d;

					float dist = glm::length(closest_point - glm::vec2(x, z));
					if (dist < d_bezier)
					{
						d_bezier = dist;
						min_t = (b * step) + t_prime * step;
					}
				}

				glm::vec3 b_closest = bezier(p0, p1, p2, p3, min_t);

				float h_prime = lerp(b_closest.y, y, 1 - glm::sin(3.14 / 2.0 - clamp(d_bezier / r_cascada, 0, 1) * 3.14 / 2.0));

				vertices.push_back(VertexFormat(glm::vec3(x, h_prime, z), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), text_coord));
			}
		}


		for (int i = 0; i < m; i++)
		{
			for (int j = 0; j < n; j++)
			{
				indices.push_back(i * (n + 1) + j);
				indices.push_back(i * (n + 1) + j + 1);
				indices.push_back((i + 1) * (n + 1) + j + 1);
				indices.push_back(i * (n + 1) + j);
				indices.push_back((i + 1) * (n + 1) + j + 1);
				indices.push_back((i + 1) * (n + 1) + j);
			}
		}

		vector<glm::vec3> normals;
		for (int i = 0; i < vertices.size(); i++)
		{
			normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
		}

		for (int i = 0; i < indices.size(); i += 3) {
			unsigned int idx1 = indices[i];
			unsigned int idx2 = indices[i + 1];
			unsigned int idx3 = indices[i + 2];

			glm::vec3 v1 = vertices[idx1].position;
			glm::vec3 v2 = vertices[idx2].position;
			glm::vec3 v3 = vertices[idx3].position;

			glm::vec3 edge1 = v2 - v1;
			glm::vec3 edge2 = v3 - v1;


			glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

			normals[idx1] += normal;
			normals[idx2] += normal;
			normals[idx3] += normal;
		}

		for (int i = 0; i < vertices.size(); i++) {
			vertices[i].normal = glm::normalize(normals[i]);
		}

		Mesh* mesh = new Mesh("grid");
		mesh->InitFromData(vertices, indices);
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

	LoadShader("Normal");
	LoadShader("Water");
	LoadShader("CubeMap");
	LoadShader("FrameBuffer");

	ResetParticlesRainSnow(10, 10, 10);

	for (int i = 0; i < 40; ++i)
	{
		LightInfo lightInfo;

		lightInfo.position = glm::vec3(Rand01() * 80 - 40, Rand01() * 12, Rand01() * 80 - 40);
		lightInfo.color = glm::vec3(Rand01() * 1, Rand01() * 1, Rand01() * 1);

		lights.push_back(lightInfo);
	}


	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	std::string texturePath = PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cubemap_night");
	cubeMapTextureID = UploadCubeMapTexture(
		PATH_JOIN(texturePath, "pos_x.png"),
		PATH_JOIN(texturePath, "pos_y.png"),
		PATH_JOIN(texturePath, "pos_z.png"),
		PATH_JOIN(texturePath, "neg_x.png"),
		PATH_JOIN(texturePath, "neg_y.png"),
		PATH_JOIN(texturePath, "neg_z.png"));

	auto resolution = window->GetResolution();
	CreateFramebuffer(resolution.x, resolution.y, reflection_framebuffer, reflection_color_texture, reflection_depth_texture);
	CreateFramebuffer(resolution.x, resolution.y, refraction_framebuffer, refraction_color_texture, refraction_depth_texture);

	glEnable(GL_CLIP_DISTANCE0);

	move_factor = 0.0;

	generator_position = glm::vec3(0, 0, 0);
	offset = 0.05;
}

void Lab5::CreateFramebuffer(int width, int height, unsigned int& framebuffer_object, unsigned int& color_texture, unsigned int& depth_texture)
{
	glGenFramebuffers(1, &framebuffer_object);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);

	glGenTextures(1, &color_texture);
	glBindTexture(GL_TEXTURE_2D, color_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);

	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "Error creating framebuffer" << endl;
	}

	// Bind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Lab5::FrameStart()
{
}


void Lab5::Update(float deltaTimeSeconds)
{
	for (auto& l : lights)
	{
		float rotationRadians = 12 * TO_RADIANS * deltaTimeSeconds;

		glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.0f), rotationRadians, glm::vec3(0, 1, 0));
		l.position = rotateMatrix * glm::vec4(l.position, 1.0f);
	}

	move_factor += 0.13 * deltaTimeSeconds;
	move_factor = fmod(move_factor, 1.0);

	ClearScreen();

	{
		auto camera = GetSceneCamera();
		glm::vec3 original_position = camera->m_transform->GetWorldPosition();
		glm::quat original_rotation = camera->m_transform->GetWorldRotation();

		glBindFramebuffer(GL_FRAMEBUFFER, reflection_framebuffer);
		ClearScreen();
		glm::vec3 inverted_position = original_position;
		inverted_position.y = 2 * 0.001 - original_position.y;
		camera->m_transform->SetWorldPosition(inverted_position);
		glm::vec3 euler_angles = glm::eulerAngles(original_rotation);
		euler_angles.x = -euler_angles.x;
		glm::quat inverted_rotation = glm::quat(euler_angles);
		camera->m_transform->SetWorldRotation(inverted_rotation);
		camera->Update();
		RenderScene(glm::vec4(0, 1, 0, 0.001));

		glBindFramebuffer(GL_FRAMEBUFFER, refraction_framebuffer);
		ClearScreen();
		camera->m_transform->SetWorldPosition(original_position);
		camera->m_transform->SetWorldRotation(original_rotation);
		camera->Update();
		RenderScene(glm::vec4(0, -1, 0, 0.001));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	{
		RenderScene(glm::vec4(0, -1, 0, 1000));
		RenderWater();
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	glDisable(GL_BLEND);
}

void Lab5::RenderScene(glm::vec4 plane)
{
	{
		auto shader = shaders["Normal"];
		shader->Use();

		auto camera = GetSceneCamera();
		glm::vec3 eye_position = camera->m_transform->GetWorldPosition();
		glUniform3fv(glGetUniformLocation(shader->program, "eye_position"), 1, glm::value_ptr(eye_position));

		for (int i = 0;i < lights.size();++i)
		{
			std::string name = std::string("lights[") + std::to_string(i) + std::string("].position");
			glUniform3fv(glGetUniformLocation(shader->program, name.c_str()), 1, glm::value_ptr(lights[i].position));
			name = std::string("lights[") + std::to_string(i) + std::string("].color");
			glUniform3fv(glGetUniformLocation(shader->program, name.c_str()), 1, glm::value_ptr(lights[i].color));
		}

		glUniform1i(glGetUniformLocation(shader->program, "lights_count"), lights.size());

		glUniform4fv(glGetUniformLocation(shader->program, "plane"), 1, glm::value_ptr(plane));

		TextureManager::GetTexture("default.png")->BindToTextureUnit(GL_TEXTURE0);

		// Render scene objects
		RenderMesh(meshes["box"], shader, glm::vec3(1.5, 0.5, 0), glm::vec3(0.5f));
		RenderMesh(meshes["box"], shader, glm::vec3(0, 1.05, 0), glm::vec3(2));
		RenderMesh(meshes["box"], shader, glm::vec3(-2, 1.5, 0));
		RenderMesh(meshes["sphere"], shader, glm::vec3(-4, -1, 1));

		// Render a simple point light bulb for each light (for debugging purposes)
		TextureManager::GetTexture("default.png")->BindToTextureUnit(GL_TEXTURE0);
		for (auto& l : lights)
		{
			auto model = glm::translate(glm::mat4(1), l.position);
			model = glm::scale(model, glm::vec3(0.2));
			RenderMesh(meshes["sphere"], shader, model);
		}

		TextureManager::GetTexture("ground.jpg")->BindToTextureUnit(GL_TEXTURE0);
		RenderMesh(meshes["grid"], shader, glm::vec3(0, 0, 0), glm::vec3(2));
	}

	{
		auto shader = shaders["CubeMap"];
		shader->Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
		glUniform1i(glGetUniformLocation(shader->program, "texture_cubemap"), 0);

		RenderMesh(meshes["box"], shader, glm::vec3(0, 0, 0), glm::vec3(100));
	}
}

void Lab5::RenderWater() {
	{
		auto shader = shaders["Water"];
		shader->Use();

		// Send reflection texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reflection_color_texture);
		glUniform1i(glGetUniformLocation(shader->program, "reflection_texture"), 0);

		// Send refraction texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, refraction_color_texture);
		glUniform1i(glGetUniformLocation(shader->program, "refraction_texture"), 1);

		TextureManager::GetTexture("waterDUDV.png")->BindToTextureUnit(GL_TEXTURE2);
		glUniform1i(glGetUniformLocation(shader->program, "dudvMap"), 2);

		TextureManager::GetTexture("normal.png")->BindToTextureUnit(GL_TEXTURE3);
		glUniform1i(glGetUniformLocation(shader->program, "normalMap"), 3);

		glUniform1f(glGetUniformLocation(shader->program, "move_factor"), move_factor);

		auto camera = GetSceneCamera();
		glm::vec3 eye_position = camera->m_transform->GetWorldPosition();
		glUniform3fv(glGetUniformLocation(shader->program, "eye_position"), 1, glm::value_ptr(eye_position));


		RenderMesh(meshes["plane"], shader, glm::vec3(0, 0, 0), glm::vec3(2));
	}
}


void Lab5::FrameEnd()
{
	// DrawCoordinateSystem();
}

void Lab5::ResetParticlesRainSnow(int xSize, int ySize, int zSize)
{
	unsigned int nrParticles = 100000;

	particleEffect = new ParticleEffect<Particle_v2>();
	particleEffect->Generate(nrParticles, true);

	auto particleSSBO = particleEffect->GetParticleBuffer();
	Particle_v2* data = const_cast<Particle_v2*>(particleSSBO->GetBuffer());

	// Calculate the half sizes once
	float xHalfSize = xSize / 20.0f;
	float yHalfSize = ySize / 20.0f;
	float zHalfSize = zSize / 20.0f;

	for (unsigned int i = 0; i < nrParticles; i++)
	{
		// Generate random floating-point numbers between -0.5 and 0.5
		float randX = (static_cast<float>(rand()) / RAND_MAX - 0.5f);
		float randY = (static_cast<float>(rand()) / RAND_MAX - 0.5f);
		float randZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f);

		glm::vec4 pos(1);
		pos.x = randX * xSize / 10.0f;
		pos.y = randY * ySize / 10.0f;
		pos.z = randZ * zSize / 10.0f;

		glm::vec4 speed(0);
		speed.x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;
		speed.y = -((static_cast<float>(rand()) / RAND_MAX) * 2.0f + 2.0f);
		speed.z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f;

		float delay = (static_cast<float>(rand()) / RAND_MAX) * 3.0f;
		float lifetime = (static_cast<float>(rand()) / RAND_MAX) * 5.0f + 5.0f;

		data[i].SetInitial(pos, speed, delay, lifetime);
	}

	particleSSBO->SetBufferData(data);
}


void Lab5::LoadShader(const std::string& name)
{
	std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab5", "shaders");

	// Create a shader program for particle system
	{
		Shader* shader = new Shader(name);
		shader->AddShader(PATH_JOIN(shaderPath, name + ".VS.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(shaderPath, name + ".FS.glsl"), GL_FRAGMENT_SHADER);

		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

void Lab5::LoadShader(const std::string& name, const std::string& VS, const std::string& FS, const std::string& GS, bool hasGeomtery)
{
	std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab5", "shaders");

	// Create a shader program for particle system
	{
		Shader* shader = new Shader(name);
		shader->AddShader(PATH_JOIN(shaderPath, VS + ".VS.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(shaderPath, FS + ".FS.glsl"), GL_FRAGMENT_SHADER);
		if (hasGeomtery)
		{
			shader->AddShader(PATH_JOIN(shaderPath, GS + ".GS.glsl"), GL_GEOMETRY_SHADER);
		}

		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

unsigned int Lab5::UploadCubeMapTexture(const std::string& pos_x, const std::string& pos_y, const std::string& pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z)
{
	int width, height, chn;

	unsigned char* data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &chn, 0);
	unsigned char* data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &chn, 0);
	unsigned char* data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &chn, 0);
	unsigned char* data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &chn, 0);
	unsigned char* data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &chn, 0);
	unsigned char* data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &chn, 0);

	unsigned int textureID = 0;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (GLEW_EXT_texture_filter_anisotropic) {
		float maxAnisotropy;

		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_y);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_z);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_y);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_z);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	if (GetOpenGLError() == GL_INVALID_OPERATION)
	{
		cout << "\t[NOTE] : For students : DON'T PANIC! This error should go away when completing the tasks." << std::endl;
	}

	// Free memory
	SAFE_FREE(data_pos_x);
	SAFE_FREE(data_pos_y);
	SAFE_FREE(data_pos_z);
	SAFE_FREE(data_neg_x);
	SAFE_FREE(data_neg_y);
	SAFE_FREE(data_neg_z);

	return textureID;
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab5::OnInputUpdate(float deltaTime, int mods)
{
	// Treat continuous update based on input
}


void Lab5::OnKeyPress(int key, int mods)
{
	// Add key press event
}


void Lab5::OnKeyRelease(int key, int mods)
{
	// Add key release event
}


void Lab5::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// Add mouse move event
}


void Lab5::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button press event
}


void Lab5::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button release event
}


void Lab5::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
	// Treat mouse scroll event
}


void Lab5::OnWindowResize(int width, int height)
{
	// Treat window resize event
}
