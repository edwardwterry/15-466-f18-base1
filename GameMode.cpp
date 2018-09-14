#include "GameMode.hpp"
#include "CratesMode.hpp"

#include "MenuMode.hpp"
#include "Load.hpp"
#include "MeshBuffer.hpp"
#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "compile_program.hpp" //helper to compile opengl shader programs
#include "draw_text.hpp" //helper to... um.. draw text
#include "vertex_color_program.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>


// MeshBuffer::Mesh tile_mesh;
// MeshBuffer::Mesh cursor_mesh;
// MeshBuffer::Mesh doll_mesh;
// MeshBuffer::Mesh egg_mesh;
// MeshBuffer::Mesh cube_mesh;

MeshBuffer::Mesh phone_bank_mesh;
Load< MeshBuffer > meshes(LoadTagDefault, [](){
	MeshBuffer const *ret = new MeshBuffer(data_path("monkey.pnc"));

	// tile_mesh = ret->lookup("Tile");
	// cursor_mesh = ret->lookup("Cursor");
	// doll_mesh = ret->lookup("Doll");
	// egg_mesh = ret->lookup("Egg");
	// cube_mesh = ret->lookup("Cube");

	// phone_bank_mesh = ret->lookup("Cube");
	phone_bank_mesh = ret->lookup("Monkey");

	return ret;
});

// std::vector< glm::vec3 > vertices;
// std::vector< glm::uvec3 > triangles;


// Load< WalkMesh > walk_mesh(LoadTagDefault, [](){ // thanks, Jim!
//     std::ifstream file(data_path("phone-bank-training.walk"), std::ios::binary);
//     read_chunk(file, "walk", &vertices);
//     read_chunk(file, "walk", &triangles);
// // 	WalkMesh const *ret = new WalkMesh(vertices, triangles);
//     return new WalkMesh(vertices, triangles);
// // // MeshBuffer const *ret = new MeshBuffer(data_path("monkey.pnc"));
// // 	return ret;
// });

Load< GLuint > meshes_for_vertex_color_program(LoadTagDefault, [](){
	return new GLuint(meshes->make_vao_for_program(vertex_color_program->program));
});


GameMode::GameMode() {
	//----------------
	//set up game board with meshes and rolls:
	board_meshes.reserve(board_size.x * board_size.y);
	board_rotations.reserve(board_size.x * board_size.y);
	std::mt19937 mt(0xbead1234);

	// std::vector< MeshBuffer::Mesh const * > meshes{ &doll_mesh, &egg_mesh, &cube_mesh };
	std::vector< MeshBuffer::Mesh const * > meshes{ &phone_bank_mesh };

	for (uint32_t i = 0; i < board_size.x * board_size.y; ++i) {
		board_meshes.emplace_back(meshes[mt()%meshes.size()]);
		board_rotations.emplace_back(glm::quat());
	}
}

GameMode::~GameMode() {
}

bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}
	//handle tracking the state of WSAD for roll control:
	if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
			controls.roll_up = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
			controls.roll_down = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_A) {
			controls.roll_left = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
			controls.roll_right = (evt.type == SDL_KEYDOWN);
			return true;
		}
	}
	//move cursor on L/R/U/D press:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat == 0) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_LEFT) {
			if (cursor.x > 0) {
				cursor.x -= 1;
			}
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
			if (cursor.x + 1 < board_size.x) {
				cursor.x += 1;
			}
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_UP) {
			if (cursor.y + 1 < board_size.y) {
				cursor.y += 1;
			}
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_DOWN) {
			if (cursor.y > 0) {
				cursor.y -= 1;
			}
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			//open pause menu on 'ESCAPE':
			show_pause_menu();
			return true;
		}
	}

	if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			controls.interact = (evt.type == SDL_KEYDOWN);
			return true;
		}	
	}

	return false;
}

void GameMode::update(float elapsed) {
	// //if the roll keys are pressed, rotate everything on the same row or column as the cursor:
	// glm::quat dr = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	// float amt = elapsed * 1.0f;
	// if (controls.roll_left) {
	// 	dr = glm::angleAxis(amt, glm::vec3(0.0f, 1.0f, 0.0f)) * dr;
	// }
	// if (controls.roll_right) {
	// 	dr = glm::angleAxis(-amt, glm::vec3(0.0f, 1.0f, 0.0f)) * dr;
	// }
	// if (controls.roll_up) {
	// 	dr = glm::angleAxis(amt, glm::vec3(1.0f, 0.0f, 0.0f)) * dr;
	// }
	// if (controls.roll_down) {
	// 	dr = glm::angleAxis(-amt, glm::vec3(1.0f, 0.0f, 0.0f)) * dr;
	// }
	// if (dr != glm::quat()) {
	// 	for (uint32_t x = 0; x < board_size.x; ++x) {
	// 		glm::quat &r = board_rotations[cursor.y * board_size.x + x];
	// 		r = glm::normalize(dr * r);
	// 	}
	// 	for (uint32_t y = 0; y < board_size.y; ++y) {
	// 		if (y != cursor.y) {
	// 			glm::quat &r = board_rotations[y * board_size.x + cursor.x];
	// 			r = glm::normalize(dr * r);
	// 		}
	// 	}
	// }

	// if (controls.interact) {
	// 	// which is the closest light?
	// 	// start by assuming you're out of range of any
	// 	uint32_t closest_light = -1;
	// 	for (uint32_t i = 0; i < num_lights; i++){
	// 		// get the distance
	// 		// ASSUMPTION: interaction_distance << 0.5*(distance between closest lights)
	// 		if (glm::distance(player.position, lights[i].position) <= interaction_distance){
	// 			// and assign the index
	// 			closest_light = i;
	// 		}
	// 	}
	// 	// if you were within range of a light
	// 	if (closest_light >= 0){
	// 		// add it to the vector of lights you interacted with
	// 		interaction_record.emplace_back(closest_light);
	// 		// TODO: play its corresponding sound
	// 	}
	// }

	// // compare this to the reference vector
	// // if the last light you interacted with is the same as the corresponding one in the ref sequence
	// if (interaction_record.back() == light_sequence[interaction_record.size()]) {
	// 	// if you are at the last in the sequence
	// 	if (interaction_record.size() == sequence_length){
	// 		add_to_sequence();
	// 	}
	// } else {
	// 	reset_sequence();
	// 	// TODO: play fail sound
	// }
}

void GameMode::draw(glm::uvec2 const &drawable_size) {
	//set up basic OpenGL state:
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Set up a transformation matrix to fit the board in the window:
	glm::mat4 world_to_clip;
	{
		float aspect = float(drawable_size.x) / float(drawable_size.y);

		//want scale such that board * scale fits in [-aspect,aspect]x[-1.0,1.0] screen box:
		float scale = glm::min(
			2.0f * aspect / float(board_size.x),
			2.0f / float(board_size.y)
		);

		//center of board will be placed at center of screen:
		glm::vec2 center = 0.5f * glm::vec2(board_size);

		//NOTE: glm matrices are specified in column-major order
		world_to_clip = glm::mat4(
			scale / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, scale, 0.0f, 0.0f,
			0.0f, 0.0f,-1.0f, 0.0f,
			-(scale / aspect) * center.x, -scale * center.y, 0.0f, 1.0f
		);
	}

	//set up graphics pipeline to use data from the meshes and the simple shading program:
	glBindVertexArray(*meshes_for_vertex_color_program);
	glUseProgram(vertex_color_program->program);

	glUniform3fv(vertex_color_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(0.81f, 0.81f, 0.76f)));
	glUniform3fv(vertex_color_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(-0.2f, 0.2f, 1.0f))));
	glUniform3fv(vertex_color_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.2f, 0.2f, 0.3f)));
	glUniform3fv(vertex_color_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));

	//helper function to draw a given mesh with a given transformation:
	auto draw_mesh = [&](MeshBuffer::Mesh const &mesh, glm::mat4 const &object_to_world) {
		//set up the matrix uniforms:
		if (vertex_color_program->object_to_clip_mat4 != -1U) {
			glm::mat4 object_to_clip = world_to_clip * object_to_world;
			glUniformMatrix4fv(vertex_color_program->object_to_clip_mat4, 1, GL_FALSE, glm::value_ptr(object_to_clip));
		}
		if (vertex_color_program->object_to_light_mat4x3 != -1U) {
			glUniformMatrix4x3fv(vertex_color_program->object_to_light_mat4x3, 1, GL_FALSE, glm::value_ptr(object_to_world));
		}
		if (vertex_color_program->normal_to_light_mat3 != -1U) {
			//NOTE: if there isn't any non-uniform scaling in the object_to_world matrix, then the inverse transpose is the matrix itself, and computing it wastes some CPU time:
			glm::mat3 normal_to_world = glm::inverse(glm::transpose(glm::mat3(object_to_world)));
			glUniformMatrix3fv(vertex_color_program->normal_to_light_mat3, 1, GL_FALSE, glm::value_ptr(normal_to_world));
		}

		//draw the mesh:
		glDrawArrays(GL_TRIANGLES, mesh.start, mesh.count);
	};

	// for (uint32_t y = 0; y < board_size.y; ++y) {
	// 	for (uint32_t x = 0; x < board_size.x; ++x) {
	// 		draw_mesh(tile_mesh,
	// 			glm::mat4(
	// 				1.0f, 0.0f, 0.0f, 0.0f,
	// 				0.0f, 1.0f, 0.0f, 0.0f,
	// 				0.0f, 0.0f, 1.0f, 0.0f,
	// 				x+0.5f, y+0.5f,-0.5f, 1.0f
	// 			)
	// 		);
	// 		draw_mesh(*board_meshes[y*board_size.x+x],
	// 			glm::mat4(
	// 				1.0f, 0.0f, 0.0f, 0.0f,
	// 				0.0f, 1.0f, 0.0f, 0.0f,
	// 				0.0f, 0.0f, 1.0f, 0.0f,
	// 				x+0.5f, y+0.5f, 0.0f, 1.0f
	// 			)
	// 			* glm::mat4_cast(board_rotations[y*board_size.x+x])
	// 		);
	// 	}
	// }
	draw_mesh(phone_bank_mesh,
		glm::mat4(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 2.0f, -1.5f, 1.0f
		)
	);

	if (Mode::current.get() == this) {
		glDisable(GL_DEPTH_TEST);
		std::string message = "PRESS ESC FOR MENU";
		float height = 0.06f;
		float width = text_width(message, height);
		draw_text(message, glm::vec2(-0.5f * width,-0.99f), height, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
		draw_text(message, glm::vec2(-0.5f * width,-1.0f), height, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		glUseProgram(0);
	}

	GL_ERRORS();
}


void GameMode::show_pause_menu() {
	std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >();

	std::shared_ptr< Mode > game = shared_from_this();
	menu->background = game;

	menu->choices.emplace_back("PAUSED");
	menu->choices.emplace_back("RESUME", [game](){
		Mode::set_current(game);
	});
	menu->choices.emplace_back("CRATES", [game](){
		Mode::set_current(std::make_shared< CratesMode >());
	});
	menu->choices.emplace_back("QUIT", [](){
		Mode::set_current(nullptr);
	});

	menu->selected = 1;

	Mode::set_current(menu);
}

void GameMode::reset_sequence() {
	sequence_length = 1;
	interaction_record.clear();
	light_sequence.clear();
	add_to_sequence();
}

void GameMode::add_to_sequence() {
	std::mt19937 mt(0xbead1234);
	light_sequence.emplace_back(mt()%num_lights);
	sequence_length++;
}