#pragma once

#include "Mode.hpp"

#include "MeshBuffer.hpp"
#include "WalkMesh.hpp"
#include "GL.hpp"
#include "Scene.hpp"
#include "Sound.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "read_chunk.hpp" //helper for reading a vector of structures from a file

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <fstream>

// The 'CratesMode' shows scene with some crates in it:

struct CratesMode : public Mode {
	CratesMode();
	virtual ~CratesMode();

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;

	//update is called at the start of a new frame, after events are handled:
	virtual void update(float elapsed) override;

	//draw is called after update:
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//starts up a 'quit/resume' pause menu:
	void show_pause_menu();

	struct {
		bool forward = false;
		bool backward = false;
		bool left = false;
		bool right = false;
		bool interact = false;
	} controls;

	bool mouse_captured = false;

	Scene scene;

	Scene::Transform *player = nullptr;

	Scene::Camera *camera = nullptr;

	// Scene::Object *large_crate = nullptr;
	// Scene::Object *small_crate = nullptr;
	std::vector<Scene::Object *> objects;

	//when this reaches zero, the 'dot' sample is triggered at the small crate:
	float dot_countdown = 1.0f;

	//this 'loop' sample is played at the large crate:
	std::shared_ptr< Sound::PlayingSample > loop;

	WalkMesh::WalkPoint walk_point;

	float azimuth = 0.0f;
	float elevation = 0.0f;

	float interaction_distance = 0.8f;

	glm::vec3 player_up, player_forward, player_right;
	// glm::vec3 player_at;
	// glm::vec3 *player_at = &player->position;
	// glm::vec3 player_up{0.0f, 0.0f, 1.0f};
	// glm::vec3 player_forward{1.0f, 0.0f, 0.0f};
	// glm::vec3 player_right{0.0f, -1.0f, 1.0f};
};
