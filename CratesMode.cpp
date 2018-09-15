#include "CratesMode.hpp"

#include "MenuMode.hpp"
#include "Load.hpp"
#include "Sound.hpp"
#include "MeshBuffer.hpp"
#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "compile_program.hpp" //helper to compile opengl shader programs
#include "draw_text.hpp" //helper to... um.. draw text
#include "vertex_color_program.hpp"

#include <glm/gtc/type_ptr.hpp>

// based on https://stackoverflow.com/questions/11515469/how-do-i-print-vector-values-of-type-glmvec3-that-have-been-passed-by-referenc
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>

Load< MeshBuffer > phone_bank_meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("phone-bank-training.pnc"));
});

std::vector< glm::vec3 > vertices;
std::vector< glm::uvec3 > triangles;

// std::vector< glm::vec3 > vertices{
// 	glm::vec3(0.0f, 0.0f, 0.0f), 
// 	glm::vec3(0.5f, -0.5f, 0.0f), 
// 	glm::vec3(0.5f, 0.5f, 0.0f), 
// 	glm::vec3(1.0f, 0.0f, 0.0f)};

// std::vector< glm::uvec3 > triangles{
// 	glm::uvec3(0, 1, 2), 
// 	glm::uvec3(3, 2, 1)};

Load< WalkMesh > walk_mesh(LoadTagDefault, [](){ // thanks, Jim!
    std::ifstream file(data_path("phone-bank-training.walk"), std::ios::binary);

	struct Vertex {
		uint32_t Index;
		glm::vec3 Position;
	};

	std::vector< Vertex > vertex_data;
	read_chunk(file, "vtx0", &vertex_data);

	for(auto v : vertex_data){
		vertices.emplace_back(v.Position);
		std::cout<<glm::to_string(v.Position)<<std::endl;
	}

	struct Triangle {
		uint32_t Index;
		glm::uvec3 Vertices;
	};

	std::vector< Triangle > triangle_data;
	read_chunk(file, "tri0", &triangle_data);

	for(auto t : triangle_data){
		triangles.emplace_back(t.Vertices);
	}
    return new WalkMesh(vertices, triangles);
});


Load< GLuint > phone_bank_meshes_for_vertex_color_program(LoadTagDefault, [](){
	return new GLuint(phone_bank_meshes->make_vao_for_program(vertex_color_program->program));
});

// Load< Sound::Sample > sample_dot(LoadTagDefault, [](){
// 	return new Sound::Sample(data_path("dot.wav"));
// });
// Load< Sound::Sample > sample_loop(LoadTagDefault, [](){
// 	return new Sound::Sample(data_path("loop.wav"));
// });

CratesMode::CratesMode() {
	//----------------
	//set up scene:
	//TODO: this should load the scene from a file!

	{
		std::string const &filename = data_path("phone-bank-training.scene");
		std::ifstream file(filename, std::ios::binary);

		struct SceneString{
			char name;
		};

		struct IndexEntry {
			uint32_t name_begin, name_end;
		};

		// static_assert(sizeof(IndexEntry) == 2*4, "Index entry should be packed");
		std::vector<SceneString> string_data;
		read_chunk(file, "str0", &string_data);

		struct SceneTransform {
			uint32_t parent_ref;
			IndexEntry indices;
			glm::vec3 position;
			glm::quat rotation;
			glm::vec3 scale;
		};
		// static_assert(sizeof(SceneTransform) == 1*4+2*4+3*4+4*4+3*4, "Index entry should be packed");
		std::vector<SceneTransform> transform_data;
		read_chunk(file, "xfh0", &transform_data);
		std::cout<<"num transforms: "<<transform_data.size()<<std::endl;

		struct SceneMesh{
			uint32_t ref;
			IndexEntry indices;
		};
		std::vector<SceneMesh> mesh_data;
		read_chunk(file, "msh0", &mesh_data);

		struct SceneCamera{
			uint32_t ref;
			char view_type;
			float fov;
			float clip_start, clip_end;
		};
		std::vector<SceneCamera> camera_data;
		read_chunk(file, "cam0", &camera_data);

		struct SceneLamp{
			uint32_t ref;
			char type;
			unsigned char r, g, b;
			float energy, distance;
			float fov;
		};
		std::vector<SceneCamera> lamp_data;
		read_chunk(file, "lmp0", &lamp_data);

		auto attach_object = [this](Scene::Transform *transform, std::string const &name) {
			Scene::Object *object = scene.new_object(transform);
			object->program = vertex_color_program->program;
			object->program_mvp_mat4 = vertex_color_program->object_to_clip_mat4;
			object->program_mv_mat4x3 = vertex_color_program->object_to_light_mat4x3;
			object->program_itmv_mat3 = vertex_color_program->normal_to_light_mat3;
			object->vao = *phone_bank_meshes_for_vertex_color_program;
			// std::cout<<"here2"<<std::endl;
			MeshBuffer::Mesh const &mesh = phone_bank_meshes->lookup(name);
			// std::cout<<"here3"<<std::endl;
			object->start = mesh.start;
			object->count = mesh.count;
			return object;
		};

		for (auto const &entry : transform_data){
			Scene::Transform *transform = scene.new_transform();
			transform->position = entry.position;
			transform->rotation = entry.rotation;
			transform->scale = entry.scale;
			std::string name(&string_data[0].name + entry.indices.name_begin, &string_data[0].name + entry.indices.name_end);
			std::cout<<name<<std::endl;
			// based on https://stackoverflow.com/questions/2340281/check-if-a-string-contains-a-string-in-c
			// find the meshes associated with objects (i.e. without *.nnn)
			if (name.find(".") != std::string::npos) {
				uint32_t index = static_cast<uint32_t> (name.find("."));
				name = name.substr(0, index);
			}
			// std::cout<<"here1"<<std::endl;
			objects.emplace_back(attach_object(transform, name));
		}
	}




	// { //build some sort of content:
	// 	//Crate at the origin:
	// 	Scene::Transform *transform1 = scene.new_transform();
	// 	transform1->position = glm::vec3(1.0f, 0.0f, 0.0f);
	// 	large_crate = attach_object(transform1, "Crate");
	// 	//smaller crate on top:
	// 	Scene::Transform *transform2 = scene.new_transform();
	// 	transform2->set_parent(transform1);
	// 	transform2->position = glm::vec3(0.0f, 0.0f, 1.5f);
	// 	transform2->scale = glm::vec3(0.5f);
	// 	small_crate = attach_object(transform2, "Crate");
	// }
	// std::cout<<"here4"<<std::endl;

	{ // Player transform
		player = scene.new_transform();
		// std::cout<<"here1"<<std::endl;
		player->position = glm::vec3(0.0f, 0.0f, 0.0f);
		// std::cout<<"here2"<<std::endl;
		// player->rotation = glm::quat_cast(glm::inverse(glm::mat3(
		// 	glm::vec3(0.0f, 1.0f, 0.0f),
		// 	glm::vec3(1.0f, 0.0f, 0.0f),
		// 	glm::vec3(0.0f, 0.0f, -1.0f))));
		// player->rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))
		// 	* glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		// std::cout<<"start pos1: "<<&player<<std::endl;
	}

	{ //Camera looking at the origin:
		Scene::Transform *transform = scene.new_transform();
		transform->position = glm::vec3(0.0f, 0.0f, 1.0f);
		//Cameras look along -z, so rotate view to look at origin:
		transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		camera = scene.new_camera(transform);
		camera->transform->set_parent(player);
	}


	//start the 'loop' sample playing at the large crate:
	// loop = sample_loop->play(objects[0]->transform->position, 1.0f, Sound::Loop);
	// std::cout<<"here5"<<std::endl;
	// std::cout<<"start pos2: "<<&player<<std::endl;

	walk_point = walk_mesh->start(player->position);
	// std::cout<<"walk_point at init: "<<glm::to_string(walk_point)<<std::endl;
}

CratesMode::~CratesMode() {
	// if (loop) loop->stop();
}

bool CratesMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}
	//handle tracking the state of WSAD for movement control:
	if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
			controls.forward = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
			controls.backward = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_A) {
			controls.left = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
			controls.right = (evt.type == SDL_KEYDOWN);
			return true;
		}
	}
	//handle tracking the mouse for rotation control:
	if (!mouse_captured) {
		if (evt.type == SDL_KEYDOWN && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			Mode::set_current(nullptr);
			return true;
		}
		if (evt.type == SDL_MOUSEBUTTONDOWN) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			mouse_captured = true;
			return true;
		}
	} else if (mouse_captured) {
		if (evt.type == SDL_KEYDOWN && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			mouse_captured = false;
			return true;
		}
		if (evt.type == SDL_MOUSEMOTION) {
			//Note: float(window_size.y) * camera->fovy is a pixels-to-radians conversion factor
			float yaw = evt.motion.xrel / float(window_size.y) * camera->fovy;
			float pitch = evt.motion.yrel / float(window_size.y) * camera->fovy;
			yaw = -yaw;
			pitch = -pitch;
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}
	return false;
}

// void CratesMode::update(float elapsed) {
// 	glm::mat3 directions = glm::mat3_cast(camera->transform->rotation);
// 	// std::cout<<"Directions: "<<glm::to_string(directions)<<std::endl;
// 	float amt = 5.0f * elapsed;
// 	// if (controls.right) camera->transform->position += amt * directions[0];
// 	// if (controls.left) camera->transform->position -= amt * directions[0];
// 	// if (controls.backward) camera->transform->position += amt * directions[2];
// 	// if (controls.forward) camera->transform->position -= amt * directions[2];
// 	// std::cout<<"camera pos: "<<glm::to_string(camera->transform->position)<<std::endl;
// 	// glm::mat3 directions = glm::mat3_cast(player->rotation);
// 	if (controls.right) player->position -= amt * directions[0];
// 	if (controls.left) player->position += amt * directions[0];
// 	if (controls.backward) player->position -= amt * directions[2];
// 	if (controls.forward) player->position += amt * directions[2];
// 	std::cout<<"player pos: "<<glm::to_string(player->position)<<std::endl;

// 	// { //set sound positions:
// 	// 	glm::mat4 cam_to_world = camera->transform->make_local_to_world();
// 	// 	Sound::listener.set_position( cam_to_world[3] );
// 	// 	//camera looks down -z, so right is +x:
// 	// 	Sound::listener.set_right( glm::normalize(cam_to_world[0]) );

// 	// 	if (loop) {
// 	// 		glm::mat4 large_crate_to_world = objects[0]->transform->make_local_to_world();
// 	// 		loop->set_position( large_crate_to_world * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) );
// 	// 	}
// 	// }

// 	// dot_countdown -= elapsed;
// 	// if (dot_countdown <= 0.0f) {
// 	// 	dot_countdown = (rand() / float(RAND_MAX) * 2.0f) + 0.5f;
// 	// 	glm::mat4x3 small_crate_to_world = small_crate->transform->make_local_to_world();
// 	// 	sample_dot->play( small_crate_to_world * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) );
// 	// }

// 		//update position on walk mesh:
// 	glm::vec3 step = player_forward * amt;
// 	// glm::vec3 step = glm::mat3_cast(player->rotation)[0] * amt;
// 	// std::cout<<"walk_point: "<<glm::to_string(walk_mesh.world_point(walk_point))<<std::endl;
// 	std::cout<<"step: "<<glm::to_string(step)<<std::endl;
// 	walk_mesh.walk(walk_point, step);

// 	//update player position:
// 	// player_at = walk_mesh.world_point(walk_point);
// 	player_at = &player->position;
// 	// std::cout<<"player at address:"<<player_at<<std::endl;
// 	*player_at = walk_mesh.world_point(walk_point);
// 	// glm::vec3 temp = walk_mesh.world_point(walk_point);
// 	// player_at = temp; 
// 	// std::cout<<"player_at :"<<glm::to_string(*player_at)<<std::endl;
// 	// player->position = player_at;
// 	// std::cout<<"player_pos :"<<glm::to_string(player->position)<<std::endl;

// 	// //update player orientation:
// 	// glm::vec3 old_player_up = player_up;
// 	// player_up = walk_mesh.world_normal(walk_point);
// 	// std::cout<<"player_up :"<<glm::to_string(player_up)<<std::endl;
// 	// std::cout<<"player rot :"<<glm::to_string(player->rotation)<<std::endl;

// 	// angle between 2 vectors
// 	// glm::quat orientation_change = glm::angle(old_player_up, player_up);

// 	// glm::quat orientation_change = std::min()
// 	// https://gamedev.stackexchange.com/questions/20097/how-to-calculate-a-3x3-rotation-matrix-from-2-direction-vectors
// 		// glm::vec3()
// 	// (compute rotation that takes old_player_up to player_up)
// 	// player_forward = orientation_change * player_forward;

// 	// //make sure player_forward is perpendicular to player_up (the earlier rotation should ensure that, but it might drift over time):
// 	// player_forward = glm::normalize(player_forward - player_up * glm::dot(player_up, player_forward));

// 	// //compute rightward direction from forward and up:
// 	// player_right = glm::cross(player_forward, player_up);

// 	// glm::mat3 player_orientation = glm::mat3(player_up, player_forward, player_right);
// }

void CratesMode::update(float elapsed) {
	glm::mat3 directions = glm::mat3_cast(camera->transform->rotation);
	// std::cout<<"Directions: "<<glm::to_string(directions)<<std::endl;	
	float amt = 3.0f * elapsed;
	glm::vec3 step = glm::vec3(0.0f, 0.0f, 0.0f);
	// if (controls.right) player->position -= amt * directions[0];
	// if (controls.left) player->position += amt * directions[0];
	// if (controls.backward) player->position -= amt * directions[2];
	// if (controls.forward) player->position += amt * directions[2];
	if (controls.forward) {
		step += directions[2] * amt;
	}
	if (controls.backward) {
		step -= directions[2] * amt;
	}
	walk_mesh->walk(walk_point, step);
	player->position = walk_mesh->world_point(walk_point);
	// std::cout<<"step :"<<glm::to_string(step)<<std::endl;
	// std::cout<<"player_pos: "<<glm::to_string(walk_mesh.world_point(walk_point))<<std::endl;
	// std::cout<<"on tri: "<<glm::to_string(walk_point.triangle)<<std::endl;
}

void CratesMode::draw(glm::uvec2 const &drawable_size) {
	//set up basic OpenGL state:
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//set up light position + color:
	glUseProgram(vertex_color_program->program);
	glUniform3fv(vertex_color_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(0.81f, 0.81f, 0.76f)));
	glUniform3fv(vertex_color_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(-0.2f, 0.2f, 1.0f))));
	glUniform3fv(vertex_color_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.4f, 0.4f, 0.45f)));
	glUniform3fv(vertex_color_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));
	glUseProgram(0);

	//fix aspect ratio of camera
	camera->aspect = drawable_size.x / float(drawable_size.y);

	scene.draw(camera);

	if (Mode::current.get() == this) {
		glDisable(GL_DEPTH_TEST);
		std::string message;
		if (mouse_captured) {
			message = "ESCAPE TO UNGRAB MOUSE * WASD MOVE";
		} else {
			message = "CLICK TO GRAB MOUSE * ESCAPE QUIT";
		}
		float height = 0.06f;
		float width = text_width(message, height);
		draw_text(message, glm::vec2(-0.5f * width,-0.99f), height, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
		draw_text(message, glm::vec2(-0.5f * width,-1.0f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		glUseProgram(0);
	}

	GL_ERRORS();
}


void CratesMode::show_pause_menu() {
	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	menu->background = game;

	menu->choices.emplace_back("PAUSED");
	menu->choices.emplace_back("RESUME", [game](){
		Mode::set_current(game);
	});
	menu->choices.emplace_back("QUIT", [](){
		Mode::set_current(nullptr);
	});

	menu->selected = 1;

	Mode::set_current(menu);
}
