#pragma once

#include <aga/camera/camera.hpp>
#include <aga/core/app.hpp>

#include <string_view>

namespace aga::camera {
struct CameraModule {
  explicit CameraModule(flecs::world& ecs);
};

class CameraPlugin final : public aga::Plugin {
public:
  std::string_view name() const override;
  void build(aga::App& app) override;
};
} // namespace aga::camera
