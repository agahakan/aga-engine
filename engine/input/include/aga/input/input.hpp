#pragma once

#include <array>
#include <cstddef>

namespace aga::input {
enum class Key : std::size_t {
  w,
  a,
  s,
  d,
  q,
  e,
  up,
  down,
  left,
  right,
  space,
  escape,
  left_shift,
  count
};

enum class MouseButton : std::size_t { right, count };

class Keyboard {
public:
  void begin_frame();
  void set(Key key, bool is_down);

  bool down(Key key) const;
  bool pressed(Key key) const;
  bool released(Key key) const;

private:
  static constexpr auto key_count = static_cast<std::size_t>(Key::count);

  std::array<bool, key_count> down_{};
  std::array<bool, key_count> pressed_{};
  std::array<bool, key_count> released_{};
};

class Mouse {
public:
  void begin_frame();
  void set(MouseButton button, bool is_down);
  void set_position(double x, double y);
  void reset_delta();

  bool down(MouseButton button) const;
  bool pressed(MouseButton button) const;
  bool released(MouseButton button) const;

  double x() const;
  double y() const;
  double delta_x() const;
  double delta_y() const;

private:
  static constexpr auto button_count = static_cast<std::size_t>(MouseButton::count);

  std::array<bool, button_count> down_{};
  std::array<bool, button_count> pressed_{};
  std::array<bool, button_count> released_{};
  double x_ = 0.0;
  double y_ = 0.0;
  double delta_x_ = 0.0;
  double delta_y_ = 0.0;
  bool has_position_ = false;
};

struct InputState {
  Keyboard keyboard;
  Mouse mouse;
};
} // namespace aga::input
