#include <aga/camera/module.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace aga::camera {
glm::mat4 projection_matrix(const Camera& camera, float aspect_ratio) {
  return glm::perspectiveRH_ZO(camera.vertical_fov_radians, aspect_ratio, camera.near_clip,
                               camera.far_clip);
}

glm::mat4 view_matrix(const transform::GlobalTransform& transform) {
  return glm::inverse(transform.matrix);
}

glm::mat4 view_projection_matrix(const Camera& camera, const transform::GlobalTransform& transform,
                                 float aspect_ratio) {
  return projection_matrix(camera, aspect_ratio) * view_matrix(transform);
}

CameraModule::CameraModule(flecs::world& ecs) {
  ecs.module<CameraModule>("aga.camera");
  ecs.component<Camera>("camera");
  ecs.component<ActiveCamera>("active_camera");
  ecs.component<FlyCameraController>("fly_camera_controller");
}

std::string_view CameraPlugin::name() const { return "camera"; }

void CameraPlugin::build(aga::App& app) { app.import_module<CameraModule>(); }
} // namespace aga::camera
