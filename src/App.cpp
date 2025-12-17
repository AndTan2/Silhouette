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

	/*int cw, ch, channels;
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

	pixels = stbi_load("assets/closed_palm_cursor.png", &cw, &ch, &channels, 4);
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

		closedHandCursor = glfwCreateCursor(&img, hotX, hotY);
		stbi_image_free(pixels);

		if (!closedHandCursor) {
			std::cerr << "Failed to create GLFW cursor from image\n";
		}
	}*/

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glEnable(GL_TEXTURE_2D);

	g_appInstance = this;
	glfwSetScrollCallback(window, GLFW_ScrollCallback);

	if (!vp.open("assets/test.mkv"))
	{
		std::cerr << "Failed to open video.\n";
	}
	else
	{
		if (vp.decodeOneFrame())
		{
			std::cout << "RGBA buffer info:\n";
			std::cout << "  width: " << vp.width()
				<< " height: " << vp.height() << "\n";
			std::cout << "  stride: " << vp.rgbaStride() << " bytes\n";
		}
	}

	return true;
}

void App::run()
{
	std::cout << "[App::run]  entering main loop...\n";
	double lastTime = glfwGetTime();
	double videoFrameTimer = 0.0;

	while (!glfwWindowShouldClose(window))
	{
		
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

		if(input.spaceDown() && !input.leftClickDown())	glfwSetCursor(window, openHandCursor);
		if (input.spaceDown() && input.leftClickDown())	glfwSetCursor(window, closedHandCursor);
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

		if (input.kPressed()) {
			playState = !playState;
		}

		if (input.leftClickPressed())
		{
			float mouseY = (float)(height - input.mouseY()); 
			if (mouseY <= scrubberY1 && mouseY >= scrubberY0)
			{
				double t = (input.mouseX() / width) * vp.durationSeconds();
				if (vp.seekSeconds(t))
				{
					videoFrameTimer = 0.0;
					std::cout << "Scrubber clicked: seeked to " << t << " sec\n";
				}
			}

		}

		double curTime = vp.currentTimeSeconds();

		if (input.leftPressed())
		{
			double target = curTime - 1.0;
			if (vp.seekSeconds(target))
			{
				videoFrameTimer = 0.0;
				std::cout << "Seeked left to ~" << target << " sec\n";
			}
		}

		if (input.rightPressed())
		{
			double target = curTime + 1.0;
			if (vp.seekSeconds(target))
			{
				videoFrameTimer = 0.0;
				std::cout << "Seeked right to ~" << target << " sec\n";
			}
		}

		if(playState)
		{ 
			videoFrameTimer += dt;
			double frameDuration = 1.0 / vp.fps();   
		
			
			while (videoFrameTimer >= frameDuration)
			{
				if (vp.decodeOneFrame())
				{
					videoFrameTimer -= frameDuration;
				}
				else
				{
					playState = false;
				}
			}

		}
		
		GLuint tex = vp.texture();

		if (tex != 0)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.0, width, 0.0, height, -1.0, 1.0);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glBindTexture(GL_TEXTURE_2D, tex);

			float x0, x1, y0, y1;
			camera.computeImageRect(width, height, vp.width(), vp.height(), x0, x1, y0, y1);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x0, y0);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x1, y0);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x1, y1);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x0, y1);
			glEnd();
			
			glBindTexture(GL_TEXTURE_2D, 0);

		}

		scrubberY1 = height / 8.0f;
		scrubberY0 = 0.0f;
		drawScrubber();

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

void App::drawScrubber()
{

	glColor3f(0.3f, 0.3f, 0.3f);
	glBegin(GL_QUADS);
	glVertex2f(0.0f, scrubberY0);
	glVertex2f((float)width, scrubberY0);
	glVertex2f((float)width, scrubberY1);
	glVertex2f(0.0f, scrubberY1);
	glEnd();

	if (vp.durationSeconds() > 0.0)
	{
		float playheadX = (float)(vp.currentTimeSeconds() / vp.durationSeconds() * width);
		float playheadWidth = 2.0f; // 2 pixels wide

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex2f(playheadX - playheadWidth / 2.0f, scrubberY0);
		glVertex2f(playheadX + playheadWidth / 2.0f, scrubberY0);
		glVertex2f(playheadX + playheadWidth / 2.0f, scrubberY1);
		glVertex2f(playheadX - playheadWidth / 2.0f, scrubberY1);
		glEnd();
	}

	glColor3f(1.0f, 1.0f, 1.0f);
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

	

	stbi_image_free(data);
	return true;
}
