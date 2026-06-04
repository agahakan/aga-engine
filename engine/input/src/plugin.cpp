#include <aga/input/module.hpp>

#include <aga/core/app.hpp>
#include <aga/window/module.hpp>
#include <aga/window/native_window.hpp>

#include <GLFW/glfw3.h>

namespace aga::input {
namespace {
struct KeyMap {
  Key key;
  int glfw_key;
};

constexpr KeyMap key_map[] = {
    {Key::w, GLFW_KEY_W},
    {Key::a, GLFW_KEY_A},
    {Key::s, GLFW_KEY_S},
    {Key::d, GLFW_KEY_D},
    {Key::q, GLFW_KEY_Q},
    {Key::e, GLFW_KEY_E},
    {Key::up, GLFW_KEY_UP},
    {Key::down, GLFW_KEY_DOWN},
    {Key::left, GLFW_KEY_LEFT},
    {Key::right, GLFW_KEY_RIGHT},
    {Key::space, GLFW_KEY_SPACE},
    {Key::escape, GLFW_KEY_ESCAPE},
    {Key::left_shift, GLFW_KEY_LEFT_SHIFT},
};

struct MouseButtonMap {
  MouseButton button;
  int glfw_button;
};

constexpr MouseButtonMap mouse_button_map[] = {
    {MouseButton::right, GLFW_MOUSE_BUTTON_RIGHT},
};
} // namespace

void Keyboard::begin_frame() {
  pressed_.fill(false);
  released_.fill(false);
}

void Keyboard::set(Key key, bool is_down) {
  const auto index = static_cast<std::size_t>(key);
  pressed_[index] = is_down && !down_[index];
  released_[index] = !is_down && down_[index];
  down_[index] = is_down;
}

bool Keyboard::down(Key key) const { return down_[static_cast<std::size_t>(key)]; }

bool Keyboard::pressed(Key key) const { return pressed_[static_cast<std::size_t>(key)]; }

bool Keyboard::released(Key key) const { return released_[static_cast<std::size_t>(key)]; }

void Mouse::begin_frame() {
  pressed_.fill(false);
  released_.fill(false);
  delta_x_ = 0.0;
  delta_y_ = 0.0;
}

void Mouse::set(MouseButton button, bool is_down) {
  const auto index = static_cast<std::size_t>(button);
  pressed_[index] = is_down && !down_[index];
  released_[index] = !is_down && down_[index];
  down_[index] = is_down;
}

void Mouse::set_position(double x, double y) {
  if (has_position_) {
    delta_x_ = x - x_;
    delta_y_ = y - y_;
  }

  x_ = x;
  y_ = y;
  has_position_ = true;
}

void Mouse::reset_delta() {
  delta_x_ = 0.0;
  delta_y_ = 0.0;
}

bool Mouse::down(MouseButton button) const { return down_[static_cast<std::size_t>(button)]; }

bool Mouse::pressed(MouseButton button) const { return pressed_[static_cast<std::size_t>(button)]; }

bool Mouse::released(MouseButton button) const {
  return released_[static_cast<std::size_t>(button)];
}

double Mouse::x() const { return x_; }

double Mouse::y() const { return y_; }

double Mouse::delta_x() const { return delta_x_; }

double Mouse::delta_y() const { return delta_y_; }

InputModule::InputModule(flecs::world& ecs) {
  ecs.module<InputModule>("aga.input");
  ecs.component<InputState>("input_state");
}

std::string_view InputPlugin::name() const { return "input"; }

void InputPlugin::build(aga::App& app) {
  app.import_module<InputModule>();
  app.world().set<InputState>({});

  app.world().system("aga.input.poll").kind(flecs::PreUpdate).run([](flecs::iter& it) {
    auto ecs = it.world();
    auto* input = ecs.get_mut<InputState>();
    auto* window_resource = ecs.get_mut<window::WindowResource>();
    if (input == nullptr || window_resource == nullptr || !window_resource->main) {
      return;
    }

    input->keyboard.begin_frame();
    input->mouse.begin_frame();
    GLFWwindow* handle = window::detail::glfw_handle(*window_resource->main);
    for (const auto& binding : key_map) {
      const int state = glfwGetKey(handle, binding.glfw_key);
      input->keyboard.set(binding.key, state == GLFW_PRESS || state == GLFW_REPEAT);
    }

    double cursor_x = 0.0;
    double cursor_y = 0.0;
    glfwGetCursorPos(handle, &cursor_x, &cursor_y);
    input->mouse.set_position(cursor_x, cursor_y);

    for (const auto& binding : mouse_button_map) {
      const int state = glfwGetMouseButton(handle, binding.glfw_button);
      input->mouse.set(binding.button, state == GLFW_PRESS);
    }

    if (input->mouse.pressed(MouseButton::right)) {
      glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      input->mouse.reset_delta();
    } else if (input->mouse.released(MouseButton::right)) {
      glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      input->mouse.reset_delta();
    }

    if (input->keyboard.pressed(Key::escape)) {
      window_resource->main->request_close();
    }

    ecs.modified<InputState>();
  });
}
} // namespace aga::input
