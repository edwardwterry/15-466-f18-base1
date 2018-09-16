// from https://github.com/opengl-tutorials/ogl/blob/master/common/quaternion_utils.hpp
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;


#ifndef QUATERNION_UTILS_H
#define QUATERNION_UTILS_H

quat RotationBetweenVectors(vec3 start, vec3 dest);

quat LookAt(vec3 direction, vec3 desiredUp);

quat RotateTowards(quat q1, quat q2, float maxAngle);


#endif // QUATERNION_UTILS_H