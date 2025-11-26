#include "App.hpp"

static App* g_appInstance = nullptr;

void App::onScroll(double xoffset, double yoffset)
{
	input.handleScroll(xoffset, yoffset);
}

static void GLFW_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	if (g_appInstance) {
		g_appInstance->onScroll(xoffset, yoffset);
	}
}

bool App::init()
{
	std::cout << "[App::init]  Sillhouette initialising...\n";

	if (!glfwInit())
	{
		std::cerr << "Failed to initialise GLFW.\n";
		return false;
	}

	window = glfwCreateWindow(1280, 720, "Silhouette", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window.\n";
		glfwTerminate();
		return false;
	}

	int cw, ch, channels;
	unsigned char* pixels = stbi_load("assets/open_palm_cursor.png", &cw, &ch, &channels, 4);
	if (!pixels) {
		std::cerr << "Failed to load cursor image\n";
	}
	else {
		GLFWimage img;
		img.width = cw;
		img.height = ch;
		img.pixels = pixels;

		int hotX = cw / 2;
		int hotY = ch / 2;

		openHandCursor = glfwCreateCursor(&img, hotX, hotY);
		stbi_image_free(pixels);

		if (!openHandCursor) {
			std::cerr << "Failed to create GLFW cursor from image\n";
		}
	}


	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glEnable(GL_TEXTURE_2D);

	g_appInstance = this;
	glfwSetScrollCallback(window, GLFW_ScrollCallback);


	if (!loadTestImage("assets/1r0uqk76pi1g1.png"))
	{
		std::cerr << "Could not load test image.\n";
		return false;
	}
	return true;
}

void App::run()
{
	std::cout << "[App::run]  entering main loop...\n";
	double lastTime = glfwGetTime();


	while (!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClearColor(0.17f, 0.09f, 0.25f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		auto frameStart = std::chrono::high_resolution_clock::now();
		double now = glfwGetTime();
		double dt = now - lastTime;
		lastTime = now;


		input.beginFrame();

		glfwPollEvents();
		input.updateFromGlfw(window);

		if(input.spaceDown())	glfwSetCursor(window, openHandCursor);
		if (!input.spaceDown())	glfwSetCursor(window, nullptr);

		bool wantPan = input.leftClickDown() && input.spaceDown();
		camera.setPanning(wantPan);

		if (wantPan)
		{
			camera.addPanDelta((float)input.deltaX(), (float)input.deltaY());
		}
			

		if (input.scrollY() != 0.0f)
		{
			double mx, my;
			glfwGetCursorPos(window, &mx, &my);

			float mouseX = (float)mx;
			float mouseY = (float)(height - my);

			camera.onScroll((float)input.scrollY(), mouseX, mouseY, width, height);
		}

		camera.beginFrame(dt);
		

		if (imageTexture != 0)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.0, width, 0.0, height, -1.0, 1.0);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glBindTexture(GL_TEXTURE_2D, imageTexture);

			float x0, x1, y0, y1;
			camera.computeImageRect(width, height, imageWidth, imageHeight, x0, x1, y0, y1);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x0, y0);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x1, y0);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x1, y1);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x0, y1);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glfwSwapBuffers(window);

		const double targetFPS = 360.0;
		const double targetFrameTime = 1.0 / targetFPS;

		frameEnd = std::chrono::high_resolution_clock::now();
		frameDuration = std::chrono::duration<double>(frameEnd - frameStart).count();

		if (frameDuration < targetFrameTime) {
			double sleepTime = targetFrameTime - frameDuration;
			std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
		}
		
	}

	std::cout << "[App::run]  main loop finished.\n";
}

void App::shutdown()
{
	std::cout << "[App::shutdown]  cleaning up...\n";

	if (imageTexture != 0)
	{
		glDeleteTextures(1, &imageTexture);
		imageTexture = 0;
	}

	if (window)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}

bool App::loadTestImage(const char* path)
{
	int channels = 0;
	stbi_set_flip_vertically_on_load(1);

	unsigned char* data = stbi_load(path, &imageWidth, &imageHeight, &channels, 4);
	if (!data)
	{
		std::cerr << "Failed to load image: " << path << "\n";
		return false;
	}

	std::cout << "loaded image: " << path << "(" << imageWidth << "x" << imageHeight << ", 4 channels)\n";

	glGenTextures(1, &imageTexture);
	glBindTexture(GL_TEXTURE_2D, imageTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage2D(
		GL_TEXTURE_2D, 0,
		GL_RGBA,
		imageWidth, imageHeight, 0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data
	);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
	return true;
}
