#pragma once

#include <aga/core/app.hpp>
#include <aga/render/renderer.hpp>

#include <memory>
#include <string_view>

namespace aga::render {
struct RenderModule {
  explicit RenderModule(flecs::world& ecs);
};

struct RendererResource {
  std::shared_ptr<Renderer> renderer;
};

class RenderPlugin final : public aga::Plugin {
public:
  explicit RenderPlugin(RendererConfig config = {});
  std::string_view name() const override;
  void build(aga::App& app) override;

private:
  RendererConfig config_;
};
} // namespace aga::render
