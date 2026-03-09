#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace aga::transform {
struct Transform {
  glm::vec3 translation{0.0F, 0.0F, 0.0F};
  glm::quat rotation{1.0F, 0.0F, 0.0F, 0.0F};
  glm::vec3 scale{1.0F, 1.0F, 1.0F};

  glm::mat4 matrix() const;
  glm::vec3 forward() const;
  glm::vec3 right() const;
  glm::vec3 up() const;
};

struct GlobalTransform {
  glm::mat4 matrix{1.0F};
};
} // namespace aga::transform
