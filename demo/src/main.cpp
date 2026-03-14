#include "demo_module.hpp"

#include <aga/aga.hpp>

#include <glm/gtc/quaternion.hpp>

int main() {
  aga::App app({.name = "aga demo"});

  app.add_plugin<aga::window::WindowPlugin>(
         aga::window::WindowConfig{.title = "aga engine demo", .width = 1280, .height = 720})
      .add_plugin<aga::input::InputPlugin>()
      .add_plugin<aga::transform::TransformPlugin>()
      .add_plugin<aga::camera::CameraPlugin>()
      .add_plugin<aga::render::RenderPlugin>();

  app.import_module<aga::demo::DemoModule>();

  auto* renderer_resource = app.world().get_mut<aga::render::RendererResource>();
  const auto cube = renderer_resource->renderer->create_mesh(aga::render::make_cube());

  app.world()
      .entity("camera")
      .set(aga::transform::Transform{.translation = {0.0F, 2.0F, 6.0F}})
      .set(aga::transform::GlobalTransform{})
      .set(aga::camera::Camera{})
      .set(aga::camera::FlyCameraController{})
      .add<aga::camera::ActiveCamera>();

  app.world()
      .entity("cube.blue")
      .set(aga::transform::Transform{.translation = {-1.3F, 0.0F, 0.0F}})
      .set(aga::transform::GlobalTransform{})
      .set(aga::render::RenderMesh{.mesh = cube, .color = {0.25F, 0.55F, 1.0F, 1.0F}})
      .set(aga::demo::Spin{.axis = {0.2F, 1.0F, 0.1F}, .radians_per_second = 1.0F});

  app.world()
      .entity("cube.orange")
      .set(aga::transform::Transform{.translation = {1.3F, 0.0F, 0.0F}})
      .set(aga::transform::GlobalTransform{})
      .set(aga::render::RenderMesh{.mesh = cube, .color = {1.0F, 0.45F, 0.18F, 1.0F}})
      .set(aga::demo::Spin{.axis = {0.0F, 1.0F, 0.4F}, .radians_per_second = -0.7F});

  app.world()
      .entity("cube.green")
      .set(aga::transform::Transform{.translation = {0.0F, -1.2F, -1.5F},
                                     .scale = {0.75F, 0.75F, 0.75F}})
      .set(aga::transform::GlobalTransform{})
      .set(aga::render::RenderMesh{.mesh = cube, .color = {0.32F, 0.9F, 0.55F, 1.0F}})
      .set(aga::demo::Spin{.axis = {1.0F, 0.3F, 0.0F}, .radians_per_second = 1.4F});

  app.run();
  return 0;
}
