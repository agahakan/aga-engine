#pragma once

#include <aga/core/app.hpp>
#include <aga/transform/transform.hpp>

#include <string_view>

namespace aga::transform {
struct TransformModule {
  explicit TransformModule(flecs::world& ecs);
};

class TransformPlugin final : public aga::Plugin {
public:
  std::string_view name() const override;
  void build(aga::App& app) override;
};
} // namespace aga::transform
