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

struct InputState {
  Keyboard keyboard;
};
} // namespace aga::input
