#include "Input.hpp"

void Input::beginFrame()
{
	_prevMouseX = _mouseX;
	_prevMouseY = _mouseY;
	_scrollX = 0.0f;
	_scrollY = 0.0f;

	_prevK = _kDown;
	_prevLeftA = _leftArrowDown;
	_prevRightA = _rightArrowDown;
}

void Input::updateFromGlfw(GLFWwindow* window)
{
	glfwGetCursorPos(window, &_mouseX, &_mouseY);

	_leftClickDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

	_spaceDown = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
	
	_kDown = (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS);

	_leftArrowDown = (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);

	_rightArrowDown = (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
}

void Input::handleScroll(double xoffset, double yoffset)
{
	_scrollX += xoffset;
	_scrollY += yoffset;
}