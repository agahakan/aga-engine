#include <aga/render/module.hpp>

#include <aga/camera/camera.hpp>
#include <aga/transform/transform.hpp>
#include <aga/window/module.hpp>
#include <aga/window/window.hpp>

#include <stdexcept>

namespace aga::render {
RenderModule::RenderModule(flecs::world& ecs) {
  ecs.module<RenderModule>("aga.render");
  ecs.component<RenderMesh>("render_mesh");
  ecs.component<RendererResource>("renderer_resource");
}

RenderPlugin::RenderPlugin(RendererConfig config) : config_(config) {}

std::string_view RenderPlugin::name() const { return "render"; }

void RenderPlugin::build(aga::App& app) {
  app.import_module<RenderModule>();

  auto* window_resource = app.world().get_mut<window::WindowResource>();
  if (window_resource == nullptr || !window_resource->main) {
    throw std::runtime_error("render plugin requires the window plugin");
  }

  auto renderer = std::make_shared<Renderer>();
  renderer->initialize(*window_resource->main, config_);
  app.world().set<RendererResource>({renderer});

  app.world().system("aga.render.frame").kind(flecs::PostUpdate).run([](flecs::iter& it) {
    auto ecs = it.world();
    auto* renderer_resource = ecs.get_mut<RendererResource>();
    auto* window_resource = ecs.get_mut<window::WindowResource>();
    if (renderer_resource == nullptr || !renderer_resource->renderer ||
        window_resource == nullptr || !window_resource->main) {
      return;
    }

    const auto [fb_width, fb_height] = window_resource->main->framebuffer_size();
    if (fb_width <= 0 || fb_height <= 0) {
      return;
    }

    renderer_resource->renderer->resize(static_cast<std::uint32_t>(fb_width),
                                        static_cast<std::uint32_t>(fb_height));

    RenderScene scene;
    const float aspect = static_cast<float>(fb_width) / static_cast<float>(fb_height);

    bool found_camera = false;
    ecs.each([&](flecs::entity entity, const camera::Camera& camera,
                 const transform::GlobalTransform& transform) {
      if (!found_camera && entity.has<camera::ActiveCamera>()) {
        scene.view_projection = camera::view_projection_matrix(camera, transform, aspect);
        found_camera = true;
      }
    });

    if (!found_camera) {
      return;
    }

    ecs.each([&](const RenderMesh& mesh, const transform::GlobalTransform& transform) {
      scene.items.push_back(RenderItem{mesh.mesh, transform.matrix, mesh.color});
    });

    renderer_resource->renderer->render(scene);
  });
}
} // namespace aga::render
