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
#include <list>
#include <algorithm>

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

	std::vector<Scene::Object *> objects;

	WalkMesh::WalkPoint walk_point;

	float hearing_distance = 3.0f;
	float interaction_distance = 2.0f;

	std::list<int32_t> interaction_record;
	std::list<int32_t> winning_sequence;
	int32_t latest_interaction = -1;

	glm::vec3 player_up, player_forward, player_right;
};
