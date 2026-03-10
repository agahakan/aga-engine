#pragma once

#include <aga/transform/transform.hpp>

#include <glm/mat4x4.hpp>

namespace aga::camera {
struct Camera {
  float vertical_fov_radians = 1.04719755F;
  float near_clip = 0.1F;
  float far_clip = 200.0F;
};

struct ActiveCamera {};

struct FlyCameraController {
  float move_speed = 5.0F;
  float look_speed = 1.6F;
};

glm::mat4 projection_matrix(const Camera& camera, float aspect_ratio);
glm::mat4 view_matrix(const transform::GlobalTransform& transform);
glm::mat4 view_projection_matrix(const Camera& camera, const transform::GlobalTransform& transform,
                                 float aspect_ratio);
} // namespace aga::camera
