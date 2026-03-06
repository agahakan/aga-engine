#pragma once

#include <aga/window/window.hpp>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>

namespace aga::window::detail {
GLFWwindow* glfw_handle(Window& window);
const GLFWwindow* glfw_handle(const Window& window);
} // namespace aga::window::detail
