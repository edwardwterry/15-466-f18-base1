#include "WalkMesh.hpp"

WalkMesh(std::vector< glm::vec3 > const &vertices_, std::vector< glm::uvec3 > const &triangles_)
	: vertices(vertices_), triangles(triangles_) {
	//TODO: construct next_vertex map
	for (glm::uvec3 triangle : triangles_){
		std::vector<uint32_t> vertices = {triangle.x, triangle.y, triangle.z};
		next_vertex.insert(glm::uvec2(vertices.at(0), vertices.at(1)), vertices.at(2));
		next_vertex.insert(glm::uvec2(vertices.at(1), vertices.at(2)), vertices.at(0));
		next_vertex.insert(glm::uvec2(vertices.at(2), vertices.at(0)), vertices.at(1));
	}
}

WalkPoint WalkMesh::start(glm::vec3 const &world_point) const {
	WalkPoint closest;
	//TODO: iterate through triangles
	for (glm::uvec3 const triangle : triangles){
		float distance;
		float min_distance = -1.0f; // negative so condition will always be met at least once
		//TODO: for each triangle, find closest point on triangle to world_point
		std::vector<uint32_t> vertices = {triangle.x, triangle.y, triangle.z};
		for (glm::vec3 const vertex : vertices){
			distance = glm::distance(world_point, vertex);
			if (distance < min_distance){
				min_distance = distance;
				min_triangle = triangle;
			}
		}
	}
	//TODO: if point is closest, closest.triangle gets the current triangle, closest.weights gets the barycentric coordinates
	closest.triangle = min_triangle;
	closest.weights = computeBaryCoords(closest.triangle, world_point);
	return closest;
}

glm::vec3 WalkMesh::computeBaryCoords(glm::uvec3 const &triangle, glm::vec3 const &world_point){
	//based on http://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf p47
	float u, v, w;
	glm::vec3 v0 = triangle.y - triangle.x;
	glm::vec3 v1 = triangle.z - triangle.x;
	glm::vec3 v2 = world_point - triangle.x;
	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;
	return glm::vec3(u, v, w);
}

void WalkMesh::walk(WalkPoint &wp, glm::vec3 const &step) const {
	//TODO: project step to barycentric coordinates to get weights_step
	glm::vec3 weights_step;

	//TODO: when does wp.weights + t * weights_step cross a triangle edge?
	float t = 1.0f;

	if (t >= 1.0f) { //if a triangle edge is not crossed
		//TODO: wp.weights gets moved by weights_step, nothing else needs to be done.

	} else { //if a triangle edge is crossed
		//TODO: wp.weights gets moved to triangle edge, and step gets reduced
		//if there is another triangle over the edge:
			//TODO: wp.triangle gets updated to adjacent triangle
			//TODO: step gets rotated over the edge
		//else if there is no other triangle over the edge:
			//TODO: wp.triangle stays the same.
			//TODO: step gets updated to slide along the edge
	}
}
