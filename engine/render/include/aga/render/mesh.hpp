#pragma once

#include <glm/vec3.hpp>

#include <cstdint>
#include <vector>

namespace aga::render {
struct Vertex {
  glm::vec3 position{};
  glm::vec3 normal{};
};

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<std::uint32_t> indices;
};

MeshData make_cube(float half_extent = 0.5F);
} // namespace aga::render
