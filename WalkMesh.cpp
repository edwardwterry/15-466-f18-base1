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
	//TODO: if point is closest, closest.triangle gets the current triangle, closest.weights gets the barycentric coordinates
	closest.triangle = getClosestTri(world_point);
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

glm::uvec3 WalkMesh::getClosestTri(glm::vec3 const &world_point){
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
	return min_triangle;
}

uint32_t WalkMesh::getCrossedEdge(glm::vec3 const &pt){
	if (std::min({weights_step.x, weights_step.y, weights_step.z}) < 0.0f){
		// find index of crossed edge
		if (pt.x < 0.0f){
			return 0;
		} else if (pt.y < 0.0f){
			return 1;
		} else {
			return 2;
		}
	} else {
		return -1;
	}
}

void WalkMesh::walk(WalkPoint &wp, glm::vec3 const &step) const {
	//TODO: project step to barycentric coordinates to get weights_step
	glm::vec3 weights_step = computeBaryCoords(wp.triangle, world_point + step);

	//TODO: when does wp.weights + t * weights_step cross a triangle edge?
	float t = 1.0f;
	// if crosses an edge, at least one will be negative
	glm::vec3 weights_update = wp.weights + t * weights_step;
	// the edge opposite this corner was crossed:
	uint32_t crossed_edge = getCrossedEdge(weights_update)
	if (crossed_edge < 0){ //if a triangle edge is not crossed
	// if (t >= 1.0f) { 
		//TODO: wp.weights gets moved by weights_step, nothing else needs to be done.
		wp.weights += weights_step;
	} else { //if a triangle edge is crossed
		//TODO: wp.weights gets moved to triangle edge, and step gets reduced
		std::unordered_map<glm::uvec2, uint32_t>::const_iterator v;
		// following based on: http://www.cplusplus.com/reference/unordered_map/unordered_map/find/
		//if there is another triangle over the edge:
		switch (crossed_edge){
			case 0:
				v = next_vertex.find(glm::uvec2(wp.triangle.z, wp.triangle.y));
				break;
			case 1:
				v = next_vertex.find(glm::uvec2(wp.triangle.x, wp.triangle.z));
				break;
			case 2:
				v = next_vertex.find(glm::uvec2(wp.triangle.y, wp.triangle.x));
				break;
			default:
				break;
		}
		if (v == next_vertex.end()){ // if there is no other triangle over the edge
			
		}
		// work out if 
		if ()
			//TODO: wp.triangle gets updated to adjacent triangle

			//TODO: step gets rotated over the edge
		//else if there is no other triangle over the edge:
			//TODO: wp.triangle stays the same.
			//TODO: step gets updated to slide along the edge
	}
}
