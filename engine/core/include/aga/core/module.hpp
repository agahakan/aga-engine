#pragma once

#include <flecs.h>

namespace aga::core {
struct CoreModule {
  explicit CoreModule(flecs::world& ecs);
};
} // namespace aga::core
