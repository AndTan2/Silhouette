#include "Input.hpp"
#include <GLFW/glfw3.h>

void Input::beginFrame()
{
	_prevMouseX = _mouseX;
	_prevMouseY = _mouseY;
	_scrollX = 0.0f;
	_scrollY = 0.0f;
}

void Input::updateFromGlfw(GLFWwindow* window)
{
	glfwGetCursorPos(window, &_mouseX, &_mouseY);

	_leftClickDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

	_spaceDown = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
}

void Input::handleScroll(double xoffset, double yoffset)
{
	_scrollX += xoffset;
	_scrollY += yoffset;
}