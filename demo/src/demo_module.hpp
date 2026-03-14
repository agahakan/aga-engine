#pragma once

#include <aga/aga.hpp>

namespace aga::demo {
struct Spin {
  glm::vec3 axis{0.0F, 1.0F, 0.0F};
  float radians_per_second = 1.0F;
};

struct DemoModule {
  explicit DemoModule(flecs::world& ecs);
};
} // namespace aga::demo
