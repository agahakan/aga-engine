#include <aga/window/window.hpp>

#include <aga/window/native_window.hpp>

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <utility>

namespace {
int g_window_count = 0;

void ensure_glfw() {
  if (g_window_count == 0 && glfwInit() != GLFW_TRUE) {
    throw std::runtime_error("failed to initialize glfw");
  }
  ++g_window_count;
}

void release_glfw() {
  --g_window_count;
  if (g_window_count == 0) {
    glfwTerminate();
  }
}
} // namespace

namespace aga::window {
Window::Window(const WindowConfig& config) {
  ensure_glfw();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

  handle_ = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
  if (handle_ == nullptr) {
    release_glfw();
    throw std::runtime_error("failed to create glfw window");
  }
}

Window::~Window() {
  if (handle_ != nullptr) {
    glfwDestroyWindow(handle_);
    handle_ = nullptr;
    release_glfw();
  }
}

Window::Window(Window&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}

Window& Window::operator=(Window&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  if (handle_ != nullptr) {
    glfwDestroyWindow(handle_);
    release_glfw();
  }

  handle_ = std::exchange(other.handle_, nullptr);
  return *this;
}

void Window::poll_events() { glfwPollEvents(); }

bool Window::should_close() const { return glfwWindowShouldClose(handle_) == GLFW_TRUE; }

void Window::request_close() { glfwSetWindowShouldClose(handle_, GLFW_TRUE); }

std::pair<int, int> Window::framebuffer_size() const {
  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(handle_, &width, &height);
  return {width, height};
}

std::pair<int, int> Window::size() const {
  int width = 0;
  int height = 0;
  glfwGetWindowSize(handle_, &width, &height);
  return {width, height};
}
} // namespace aga::window

namespace aga::window::detail {
GLFWwindow* glfw_handle(Window& window) { return window.handle_; }

const GLFWwindow* glfw_handle(const Window& window) { return window.handle_; }
} // namespace aga::window::detail
