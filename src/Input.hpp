#pragma once
#include <GLFW/glfw3.h>

struct GLFWwindow;

class Input {
public:
	void beginFrame();
	void updateFromGlfw(GLFWwindow* window);
	void handleScroll(double xoffset, double yoffset);

	double mouseX() const { return _mouseX; }
	double mouseY() const { return _mouseY; }
	double deltaX() const { return _mouseX - _prevMouseX; }
	double deltaY() const { return _mouseY - _prevMouseY; }

	double scrollX() const { return _scrollX; }
	double scrollY() const { return _scrollY; }

	bool leftClickDown() const { return _leftClickDown; }
	bool spaceDown() const { return _spaceDown; }

	bool kPressed() const { return _kDown && !_prevK; }
	bool leftPressed() const { return _leftArrowDown && !_prevLeftA; }
	bool rightPressed() const { return _rightArrowDown && !_prevRightA; }
	bool leftClickPressed() const { return _leftClickDown && !_prevLeftClick; }
private:
	double _mouseX = 0.0f;
	double _mouseY = 0.0f;
	double _prevMouseX = 0.0f;
	double _prevMouseY = 0.0f;

	double _scrollX = 0.0f;
	double _scrollY = 0.0f;

	bool _leftClickDown = false;
	bool _spaceDown = false;

	bool _kDown = false;
	bool _prevK = false;

	bool _leftArrowDown = false;
	bool _rightArrowDown = false;

	bool _prevLeftA = false;
	bool _prevRightA = false;

	bool _prevLeftClick = false;
	
	
};