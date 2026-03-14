#include "demo_module.hpp"

#include <aga/camera/camera.hpp>
#include <aga/core/app.hpp>
#include <aga/input/input.hpp>
#include <aga/transform/transform.hpp>

#include <glm/ext/quaternion_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

namespace aga::demo {
namespace {
glm::vec3 normalized_or_zero(glm::vec3 value) {
  const float length = glm::length(value);
  if (length <= 0.0001F) {
    return glm::vec3{0.0F};
  }
  return value / length;
}
} // namespace

DemoModule::DemoModule(flecs::world& ecs) {
  ecs.module<DemoModule>("aga.demo");
  ecs.component<Spin>("spin");

  ecs.system<transform::Transform, const Spin>("aga.demo.spin")
      .kind(flecs::OnUpdate)
      .each([](flecs::iter& it, std::size_t i, transform::Transform& transform, const Spin& spin) {
        (void)i;
        const float delta = static_cast<float>(it.delta_time());
        const auto axis = normalized_or_zero(spin.axis);
        if (glm::length(axis) == 0.0F) {
          return;
        }
        const auto delta_rotation = glm::angleAxis(spin.radians_per_second * delta, axis);
        transform.rotation = glm::normalize(delta_rotation * transform.rotation);
      });

  ecs.system<transform::Transform, const camera::FlyCameraController>("aga.demo.fly_camera")
      .kind(flecs::OnUpdate)
      .each([](flecs::iter& it, std::size_t i, transform::Transform& transform,
               const camera::FlyCameraController& controller) {
        (void)i;
        const auto ecs = it.world();
        const auto* input = ecs.get<input::InputState>();
        if (input == nullptr) {
          return;
        }

        const auto& keys = input->keyboard;
        glm::vec3 movement{0.0F};
        if (keys.down(input::Key::w)) {
          movement += transform.forward();
        }
        if (keys.down(input::Key::s)) {
          movement -= transform.forward();
        }
        if (keys.down(input::Key::d)) {
          movement += transform.right();
        }
        if (keys.down(input::Key::a)) {
          movement -= transform.right();
        }
        if (keys.down(input::Key::space)) {
          movement += glm::vec3{0.0F, 1.0F, 0.0F};
        }
        if (keys.down(input::Key::left_shift)) {
          movement -= glm::vec3{0.0F, 1.0F, 0.0F};
        }

        const float delta = static_cast<float>(it.delta_time());
        transform.translation += normalized_or_zero(movement) * controller.move_speed * delta;

        float yaw = 0.0F;
        float pitch = 0.0F;
        if (keys.down(input::Key::left)) {
          yaw += 1.0F;
        }
        if (keys.down(input::Key::right)) {
          yaw -= 1.0F;
        }
        if (keys.down(input::Key::up)) {
          pitch += 1.0F;
        }
        if (keys.down(input::Key::down)) {
          pitch -= 1.0F;
        }

        if (yaw != 0.0F) {
          transform.rotation = glm::normalize(
              glm::angleAxis(yaw * controller.look_speed * delta, glm::vec3{0.0F, 1.0F, 0.0F}) *
              transform.rotation);
        }
        if (pitch != 0.0F) {
          transform.rotation = glm::normalize(
              transform.rotation *
              glm::angleAxis(pitch * controller.look_speed * delta, glm::vec3{1.0F, 0.0F, 0.0F}));
        }
      });
}
} // namespace aga::demo
