#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp> //allows the use of 'uvec2' as an unordered_map key
#include <glm/gtx/string_cast.hpp> //https://stackoverflow.com/questions/11515469/how-do-i-print-vector-values-of-type-glmvec3-that-have-been-passed-by-referenc
#include "read_chunk.hpp" //helper for reading a vector of structures from a file

struct WalkMesh {
	//Walk mesh will keep track of triangles, vertices:
	std::vector< glm::vec3 > vertices;
	std::vector< glm::uvec3 > triangles; //CCW-oriented

	//TODO: consider also loading vertex normals for interpolated "up" direction:
	//std::vector< glm::vec3 > vertex_normals;

	//This "next vertex" map includes [a,b]->c, [b,c]->a, and [c,a]->b for each triangle, and is useful for checking what's over an edge from a given point:
	std::unordered_map< glm::uvec2, uint32_t > next_vertex;

	//Construct new WalkMesh and build next_vertex structure:
	WalkMesh(std::vector< glm::vec3 > const &vertices_, std::vector< glm::uvec3 > const &triangles_);

	struct WalkPoint {
		glm::uvec3 triangle = glm::uvec3(-1U); //indices of current triangle
		glm::vec3 weights = glm::vec3(std::numeric_limits< float >::quiet_NaN()); //barycentric coordinates for current point
	};

	//used to initialize walking -- finds the closest point on the walk mesh:
	// (should only need to call this at the start of a level)
	WalkPoint start(glm::vec3 const &world_point) const;

	//used to update walk point:
	void walk(WalkPoint &wp, glm::vec3 const &step) const;

	//used to read back results of walking:
	glm::vec3 world_point(WalkPoint const &wp) const {
		return wp.weights.x * vertices[wp.triangle.x]
		     + wp.weights.y * vertices[wp.triangle.y]
		     + wp.weights.z * vertices[wp.triangle.z];
	}

	glm::vec3 world_normal(WalkPoint const &wp) const {
		return glm::normalize(glm::cross(
			vertices[wp.triangle.y] - vertices[wp.triangle.x],
			vertices[wp.triangle.z] - vertices[wp.triangle.x]
		));
	}

	int32_t getCrossedEdge(glm::vec3 const &pt) const;
	glm::uvec3 getClosestTri(glm::vec3 const &world_point) const;
	glm::vec3 computeBaryCoords(glm::uvec3 const &triangle, glm::vec3 const &world_point) const;

};