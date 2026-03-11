#pragma once

#include <aga/render/mesh.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace aga::window {
class Window;
}

namespace aga::render {
struct MeshId {
  std::uint32_t value = 0;
};

struct RenderMesh {
  MeshId mesh{};
  glm::vec4 color{1.0F, 1.0F, 1.0F, 1.0F};
};

struct RendererConfig {
  bool vsync = true;
  glm::vec4 clear_color{0.02F, 0.025F, 0.03F, 1.0F};
};

struct RenderItem {
  MeshId mesh{};
  glm::mat4 model{1.0F};
  glm::vec4 color{1.0F};
};

struct RenderScene {
  glm::mat4 view_projection{1.0F};
  std::vector<RenderItem> items;
};

class Renderer {
public:
  Renderer();
  ~Renderer();

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) noexcept;
  Renderer& operator=(Renderer&&) noexcept;

  void initialize(window::Window& window, const RendererConfig& config);
  MeshId create_mesh(const MeshData& mesh);
  void resize(std::uint32_t width, std::uint32_t height);
  void render(const RenderScene& scene);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
} // namespace aga::render
