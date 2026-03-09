#include <aga/transform/module.hpp>

#include <glm/gtx/quaternion.hpp>

namespace aga::transform {
glm::mat4 Transform::matrix() const {
  const auto translation_matrix = glm::translate(glm::mat4{1.0F}, translation);
  const auto rotation_matrix = glm::toMat4(rotation);
  const auto scale_matrix = glm::scale(glm::mat4{1.0F}, scale);
  return translation_matrix * rotation_matrix * scale_matrix;
}

glm::vec3 Transform::forward() const { return rotation * glm::vec3{0.0F, 0.0F, -1.0F}; }

glm::vec3 Transform::right() const { return rotation * glm::vec3{1.0F, 0.0F, 0.0F}; }

glm::vec3 Transform::up() const { return rotation * glm::vec3{0.0F, 1.0F, 0.0F}; }

TransformModule::TransformModule(flecs::world& ecs) {
  ecs.module<TransformModule>("aga.transform");
  ecs.component<Transform>("transform");
  ecs.component<GlobalTransform>("global_transform");

  ecs.system<const Transform, GlobalTransform>("aga.transform.update_global")
      .kind(flecs::OnUpdate)
      .each(
          [](const Transform& local, GlobalTransform& global) { global.matrix = local.matrix(); });
}

std::string_view TransformPlugin::name() const { return "transform"; }

void TransformPlugin::build(aga::App& app) { app.import_module<TransformModule>(); }
} // namespace aga::transform
