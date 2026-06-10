#include "App.hpp"

static App* g_appInstance = nullptr;

void App::onScroll(double xoffset, double yoffset)
{
    input.handleScroll(xoffset, yoffset);
}

static void GLFW_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (g_appInstance) {
        g_appInstance->onScroll(xoffset, yoffset);
    }
}

bool App::init()
{
    std::cout << "[App::init]  Sillhouette initialising...\n";

    if (!glfwInit()) {
        std::cerr << "Failed to initialise GLFW.\n";
        return false;
    }

    window = glfwCreateWindow(1280, 720, "Silhouette", nullptr, nullptr);
    if (!window) {
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


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return false;
    }

    // Verify OpenGL is loaded
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;


    glfwSwapInterval(0);
    glEnable(GL_TEXTURE_2D);

    g_appInstance = this;
    glfwSetScrollCallback(window, GLFW_ScrollCallback);

    

    dec.open("D:/shared_media/Mushoku.Tensei.Jobless.Reincarnation.S02.1080p.AMZN.WEB-DL.DDP2.0.H.264-VARYG/Mushoku.Tensei.Jobless.Reincarnation.S02E02.The.Midnight.Forest.1080p.AMZN.WEB-DL.DDP2.0.H.264-VARYG.mkv");
    
    dec.setFrameFetcher(&vp);
    dec.setCacheState(&vp);
    dec.sendInfoToPlayer();

    dec.startDecodingThread();

    scrb.init(&vp);
    vp.open();
    vp.setSeekController(&dec);
    

    keyframePts = vp.KeyFrames;
    return true;
}

void App::run()
{

        std::cout << "[App::run]  entering main loop...\n";
        double lastTime = glfwGetTime();
        double videoFrameTimer = 0.0;

        while (!glfwWindowShouldClose(window)) {
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

            if (input.spaceDown() && !input.leftClickDown())
                glfwSetCursor(window, openHandCursor);
            if (input.spaceDown() && input.leftClickDown())
                glfwSetCursor(window, closedHandCursor);
            if (!input.spaceDown())
                glfwSetCursor(window, nullptr);

            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            float mouseX = (float)mx;
            float mouseY = (float)(height - my);

            // Compute scrubber hit area from its actual rendered rect
            float scrubTop = scrb.scrubberMarkerY;
            float scrubBottom = scrb.scrubberMarkerY + scrb.scrubberBarHeight;
            bool mouseOverScrubber = (mouseY >= scrubTop && mouseY <= scrubBottom);

            bool wantPan = input.leftClickDown() && input.spaceDown() && !mouseOverScrubber;
            bool wantScrubberPan = input.leftClickDown() && input.spaceDown() && mouseOverScrubber;
            camera.setPanning(wantPan);

            if (wantPan) {
                camera.addPanDelta((float)input.deltaX(), (float)input.deltaY());
            }

            if (wantScrubberPan) {
                float dx = (float)input.deltaX();
                scrb.addPanDelta(dx);
            }

            if (input.scrollY() != 0.0f) {
                if (!mouseOverScrubber) {
                    camera.onScroll((float)input.scrollY(), mouseX, mouseY, width, height);
                }
                if (mouseOverScrubber) {
                    scrb.zoom((float)input.scrollY(), (float)mx, (float)width);
                }
            }

            camera.beginFrame(dt);
            scrb.update(dt);

            if (input.kPressed()) {
                playState = !playState;
            }

            static bool wasLeftClickDown = false;

            if (input.leftClickDown() && !input.spaceDown()) {
                float mouseY = static_cast<float>(height - input.mouseY());

                if (mouseOverScrubber) {
                    float cursorNorm = static_cast<float>(input.mouseX()) / width;
                    float timelineNorm = scrb.interpolatedOffset + cursorNorm / scrb.zoomFactor;

                    if (timelineNorm < 0.0f) timelineNorm = 0.0f;
                    if (timelineNorm > 1.0f) timelineNorm = 1.0f;

                    double t = timelineNorm * vp.durationSeconds();
                    bool insideCache = vp.isTimeInsideCache(t);

                    if (insideCache) {
                        vp.seek(t);
                    }
                    else {
                        if (!wasLeftClickDown) {
                            vp.seek(t);
                        }
                    }
                }
            }

            wasLeftClickDown = input.leftClickDown();

        vp.update();

       /* double curTime = vp.currentTimeSeconds();

        if (input.leftPressed()) {
            double target = curTime - 1.0;
            if (vp.seekSeconds(target)) {
                videoFrameTimer = 0.0;
                std::cout << "Seeked left to ~" << target << " sec\n";
            }
        }

        if (input.rightPressed()) {
            double target = curTime + 1.0;
            if (vp.seekSeconds(target)) {
                videoFrameTimer = 0.0;
                std::cout << "Seeked right to ~" << target << " sec\n";
            }
        }*/

        if (input.cPressed()) {
            vp.clearCache();
        }

        if (input.fPressed()) {
            
            //vp.printCacheTimestamps();
            dec.benchmark();
        }

        if (input.kPressed()) {
            //dec.decodeOneFrame();
            useYUVRenderer = !useYUVRenderer;
        }


        //scene rendering(frame only)
        float x0, x1, y0, y1;
        camera.computeImageRect(width, height, vp.width(), vp.height(), x0, x1, y0, y1);
        vp.render(x0, x1, y0, y1, width, height);
           
        
        
        scrb.draw(width, height);

        glfwSwapBuffers(window);
        
        const double targetFPS = 360.0;
        const double targetFrameTime = 1.0 / targetFPS;

        auto frameEnd = std::chrono::high_resolution_clock::now();
        double frameDuration = std::chrono::duration<double>(frameEnd - frameStart).count();
        
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
    dec.stopDecodingThread();
    if (imageTexture != 0) {
        glDeleteTextures(1, &imageTexture);
        imageTexture = 0;
    }

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}




