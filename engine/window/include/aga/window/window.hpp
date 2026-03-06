#pragma once

#include <aga/window/module.hpp>

#include <utility>

struct GLFWwindow;

namespace aga::window {
class Window;

namespace detail {
GLFWwindow* glfw_handle(Window& window);
const GLFWwindow* glfw_handle(const Window& window);
} // namespace detail

class Window {
public:
  explicit Window(const WindowConfig& config);
  ~Window();

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  Window(Window&& other) noexcept;
  Window& operator=(Window&& other) noexcept;

  void poll_events();
  bool should_close() const;
  void request_close();
  std::pair<int, int> framebuffer_size() const;
  std::pair<int, int> size() const;

private:
  friend GLFWwindow* detail::glfw_handle(Window& window);
  friend const GLFWwindow* detail::glfw_handle(const Window& window);

  GLFWwindow* handle_ = nullptr;
};
} // namespace aga::window
