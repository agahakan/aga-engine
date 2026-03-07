#pragma once

#include <aga/core/app.hpp>
#include <aga/input/input.hpp>

#include <string_view>

namespace aga::input {
struct InputModule {
  explicit InputModule(flecs::world& ecs);
};

class InputPlugin final : public aga::Plugin {
public:
  std::string_view name() const override;
  void build(aga::App& app) override;
};
} // namespace aga::input
