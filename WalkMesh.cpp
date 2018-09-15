#include "WalkMesh.hpp"

WalkMesh::WalkMesh(std::vector< glm::vec3 > const &vertices_, std::vector< glm::uvec3 > const &triangles_)
	: vertices(vertices_), triangles(triangles_) {
// WalkMesh::WalkMesh(std::string const &filename){
//     std::ifstream file(filename, std::ios::binary);
//     read_chunk(file, "walk", &vertices);
//     read_chunk(file, "walk", &triangles);
	//TODO: construct next_vertex map
	// for (glm::uvec3 tri : triangles_){
	for (glm::uvec3 tri : triangles){
		std::vector<uint32_t> v = {tri.x, tri.y, tri.z};
		next_vertex.emplace(glm::uvec2(v[0], v[1]), v[2]);
		next_vertex.emplace(glm::uvec2(v[1], v[2]), v[0]);
		next_vertex.emplace(glm::uvec2(v[2], v[0]), v[1]);
		triangles_std = v;
		// std::cout<<v[0]<<" "<<v[1]<<" "<<v[2]<<std::endl;
	}
	// for (glm::vec3 v : vertices_){
	// 	std::cout<<v[0]<<" "<<v[1]<<" "<<v[2]<<std::endl;
	// }
}

WalkMesh::WalkPoint WalkMesh::start(glm::vec3 const &world_point) const {
	WalkPoint closest;
	//TODO: if point is closest, closest.triangle gets the current triangle, closest.weights gets the barycentric coordinates
	std::cout<<"world_point: "<<glm::to_string(world_point)<<std::endl;
	closest.triangle = getClosestTri(world_point);
	std::cout<<"closest tri: "<<glm::to_string(closest.triangle)<<std::endl;
	closest.weights = computeBaryCoords(closest.triangle, world_point);
	std::cout<<"bary coords"<<glm::to_string(closest.weights)<<std::endl;
	return closest;
}

glm::vec3 WalkMesh::computeBaryCoords(glm::uvec3 const &tri, glm::vec3 const &pt) const{
	//based on http://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf p47
	// std::cout<<"tri: "<<glm::to_string(tri)<<std::endl;
	// std::cout<<"pt: "<<glm::to_string(pt)<<std::endl;
	float u, v, w;
	glm::vec3 v0 = glm::vec3(vertices[tri.y] - vertices[tri.x]);
	glm::vec3 v1 = glm::vec3(vertices[tri.z] - vertices[tri.x]);
	glm::vec3 v2 = glm::vec3(pt - vertices[tri.x]);
	// std::cout<<"v0 "<<glm::to_string(v0)<<std::endl;
	// std::cout<<"v1 "<<glm::to_string(v1)<<std::endl;
	// std::cout<<"v2 "<<glm::to_string(v2)<<std::endl;
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
			std::cout<<"distance: "<<distance<<std::endl;
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
	while (getCrossedEdge(weights_step) >= 0){ // as long as step takes you outside current triangle
		t -= 0.05f; // decrement the step scale factor
		glm::vec3 walk_result = world_point(wp) + t * step; // recalculate (x,y,z) of expected step
		weights_step = computeBaryCoords(wp.triangle, walk_result); // bary if this step is taken
	}
	std::cout<<"t: "<<t<<std::endl;
	std::cout<<"weights_step: "<<glm::to_string(weights_step)<<std::endl;
	// glm::vec3 weights_update = wp.weights + t * weights_step; // use the one which will keep you inside triangle
	if (t >= 1.0f) { 
	// 	//TODO: wp.weights gets moved by weights_step, nothing else needs to be done.
		wp.weights = weights_step;
	} else { //if a triangle edge is crossed
	// 	//TODO: wp.weights gets moved to triangle edge, and step gets reduced
		wp.weights = weights_step;
		bool at_boundary = false;
	// 	// based on: http://www.cplusplus.com/reference/unordered_map/unordered_map/find/
		std::unordered_map<glm::uvec2, uint32_t>::const_iterator v;
	// 	//if there is another triangle over the edge:
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
		std::cout<<"next_tri_vertices_glm: "<<glm::to_string(next_tri_vertices_glm)<<std::endl;

		if (at_boundary){ // if there is no other triangle over the edge
		
		} else {
			// thanks to https://stackoverflow.com/questions/22388204/get-index-of-the-matching-item-from-vector-c
			uint32_t index = 0;
			while(std::find(std::begin(triangles), std::end(triangles), next_tri_vertices_glm) != std::end(triangles)){
				std::rotate(std::begin(next_tri_vertices), std::begin(next_tri_vertices) + 1, std::end(next_tri_vertices));
				next_tri_vertices_glm.x = next_tri_vertices[0];
				next_tri_vertices_glm.y = next_tri_vertices[1];
				next_tri_vertices_glm.z = next_tri_vertices[2];
				std::cout<<"index"<<index<<std::endl;
				index++;
			}
			// auto next_tri_index = std::find(std::begin(triangles), std::end(triangles), next_tri_vertices_glm);
			glm::vec3 walk_result = world_point(wp) + (1 - t) * step; // recalculate (x,y,z) of expected step
			std::cout<<"walk_result (x,y,z): "<<glm::to_string(walk_result)<<std::endl;
			// std::cout<<"std::begin(triangles): "<<&std::begin(triangles)<<std::endl;
			// std::cout<<"next_tri_index: "<<&next_tri_index<<std::endl;
			// std::cout<<"dist: "<<std::distance(std::begin(triangles), next_tri_index)<<std::endl;
			wp.triangle = next_tri_vertices_glm; // triangles[std::distance(std::begin(triangles), next_tri_index)];
			weights_step = computeBaryCoords(wp.triangle, walk_result); // bary if this step is taken, will automatically project to surface
			wp.weights = weights_step;
			std::cout<<"triangle: "<<glm::to_string(wp.triangle)<<std::endl;
		}



	// 		// vector of crossed edgeC:\Users\Edward\OneDrive\Documents\03.MRSD_Sem3\01.15-666\15-466-f18-base1
	// 		glm::vec3 crossed_edge_vector;
	// 		glm::vec3 slid_point;
	// 		switch (crossed_edge){
	// 			case 0:
	// 				crossed_edge_vector = glm::vec3(vertices[wp.triangle.z].z - vertices[wp.triangle.y].y);
	// 				break;
	// 			case 1:
	// 				crossed_edge_vector = glm::vec3(vertices[wp.triangle.x].x - vertices[wp.triangle.z].z);
	// 				break;
	// 			case 2:
	// 				crossed_edge_vector = glm::vec3(vertices[wp.triangle.y].y - vertices[wp.triangle.x].x);
	// 				break;
	// 			default:
	// 				break;
	// 		}
	// 		//TODO: step gets updated to slide along the edge
	// 		slid_point = glm::dot(weights_step, crossed_edge_vector) * glm::normalize(weights_step); // projection onto edge in world coords
	// 		//TODO: wp.triangle stays the same.
	// 		wp.weights = computeBaryCoords(wp.triangle, slid_point);
	// 	} else {
	// 		//TODO: wp.triangle gets updated to adjacent triangle
			
	// 		//TODO: step gets rotated over the edge
	// 	}

	}
}