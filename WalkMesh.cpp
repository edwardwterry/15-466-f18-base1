#include "WalkMesh.hpp"

WalkMesh::WalkMesh(std::vector< glm::vec3 > const &vertices_, std::vector< glm::uvec3 > const &triangles_)
	: vertices(vertices_), triangles(triangles_) {
	for (glm::uvec3 tri : triangles){
		std::vector<uint32_t> v = {tri.x, tri.y, tri.z};
		next_vertex.emplace(glm::uvec2(v[0], v[1]), v[2]);
		next_vertex.emplace(glm::uvec2(v[1], v[2]), v[0]);
		next_vertex.emplace(glm::uvec2(v[2], v[0]), v[1]);
	}
}

WalkMesh::WalkPoint WalkMesh::start(glm::vec3 const &world_point) const {
	WalkPoint closest;
	//TODO: if point is closest, closest.triangle gets the current triangle, closest.weights gets the barycentric coordinates
	closest.triangle = getClosestTri(world_point);
	closest.weights = computeBaryCoords(closest.triangle, world_point);
	return closest;
}

glm::vec3 WalkMesh::computeBaryCoords(glm::uvec3 const &tri, glm::vec3 const &pt) const{
	//based on http://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf p47
	float u, v, w;
	glm::vec3 v0 = glm::vec3(vertices[tri.y] - vertices[tri.x]);
	glm::vec3 v1 = glm::vec3(vertices[tri.z] - vertices[tri.x]);
	glm::vec3 v2 = glm::vec3(pt - vertices[tri.x]);
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

glm::uvec3 WalkMesh::getClosestTri(glm::vec3 const &world_point) const{
	//TODO: iterate through triangles
	uint32_t min_triangle = -1;
	for (uint32_t i = 0; i < triangles.size(); i++){
		float distance, min_distance;
		//TODO: for each triangle, find closest point on triangle to world_point
		std::vector<uint32_t> vs = {triangles[i].x, triangles[i].y, triangles[i].z};
		for (uint32_t v : vs){
			distance = glm::distance(world_point, vertices[v]);
			// std::cout<<"distance: "<<distance<<std::endl;
			if (i == 0){
				min_distance = distance;
				min_triangle = i;
			} else if (distance < min_distance){
				min_distance = distance;
				min_triangle = i;
			}
		}
	}
	return triangles[min_triangle];
}

int32_t WalkMesh::getCrossedEdge(glm::vec3 const &pt) const{
	if (std::min({pt.x, pt.y, pt.z}) < 0.0f){
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
	float t = 1.0f; // proportion of step within current triangle
	glm::vec3 original_walk_result = world_point(wp) + t * step; // in (x,y,z) coordinates
	//TODO: project step to barycentric coordinates to get weights_step
	glm::vec3 weights_step = computeBaryCoords(wp.triangle, original_walk_result); // bary if this step is taken
	while (getCrossedEdge(weights_step) >= 0 && t > 0.0f){ // as long as step takes you outside current triangle
		t -= 0.05f; // decrement the step scale factor
		glm::vec3 walk_result = world_point(wp) + t * step; // recalculate (x,y,z) of expected step
		weights_step = computeBaryCoords(wp.triangle, walk_result); // bary if this step is taken
		// std::cout<<"t: "<<t<<std::endl;
	}
	if (t >= 1.0f) { 
	// 	//TODO: wp.weights gets moved by weights_step, nothing else needs to be done.
		wp.weights = weights_step;
	} else { //if a triangle edge is crossed
	// 	//TODO: wp.weights gets moved to triangle edge, and step gets reduced
		wp.weights = weights_step;
		bool at_boundary = false;
		// based on: http://www.cplusplus.com/reference/unordered_map/unordered_map/find/
		std::unordered_map<glm::uvec2, uint32_t>::const_iterator v;
		//if there is another triangle over the edge:
		uint32_t crossed_edge = getCrossedEdge(computeBaryCoords(wp.triangle, original_walk_result));
		std::vector<uint32_t> next_tri_vertices{wp.triangle.x, wp.triangle.y, wp.triangle.z};
		switch (crossed_edge){
			case 0:
				v = next_vertex.find(glm::uvec2(wp.triangle.z, wp.triangle.y));
				if (v != next_vertex.end()){
					next_tri_vertices[0] = v->second;
					std::swap(next_tri_vertices[1], next_tri_vertices[2]);					
				} else {at_boundary = true;}
				break;
			case 1:
				v = next_vertex.find(glm::uvec2(wp.triangle.x, wp.triangle.z));
				if (v != next_vertex.end()){
					next_tri_vertices[1] = v->second;
					std::swap(next_tri_vertices[0], next_tri_vertices[2]);
				} else {at_boundary = true;}	
				break;
			case 2:
				v = next_vertex.find(glm::uvec2(wp.triangle.y, wp.triangle.x));
				if (v != next_vertex.end()){
					next_tri_vertices[2] = v->second;
					std::swap(next_tri_vertices[0], next_tri_vertices[1]);
				} else {at_boundary = true;}					
				break;
			default:
				break;
		}
		glm::uvec3 next_tri_vertices_glm;
		next_tri_vertices_glm.x = next_tri_vertices[0];
		next_tri_vertices_glm.y = next_tri_vertices[1];
		next_tri_vertices_glm.z = next_tri_vertices[2];

		if (at_boundary){ // if there is no other triangle over the edge
			// sorry, couldn't figure this out!
		} else {
			// std::cout<<"here"<<std::endl;
			// thanks to https://stackoverflow.com/questions/22388204/get-index-of-the-matching-item-from-vector-c
			while(std::find(std::begin(triangles), std::end(triangles), next_tri_vertices_glm) != std::end(triangles)){
				std::rotate(std::begin(next_tri_vertices), std::begin(next_tri_vertices) + 1, std::end(next_tri_vertices));
				next_tri_vertices_glm.x = next_tri_vertices[0];
				next_tri_vertices_glm.y = next_tri_vertices[1];
				next_tri_vertices_glm.z = next_tri_vertices[2];
			}
			glm::vec3 walk_result = world_point(wp) + (1 - t) * step; // recalculate (x,y,z) of expected step
			wp.triangle = next_tri_vertices_glm; // triangles[std::distance(std::begin(triangles), next_tri_index)];
			weights_step = computeBaryCoords(wp.triangle, walk_result); // bary if this step is taken, will automatically project to surface
			wp.weights = weights_step;
		}
	}
}