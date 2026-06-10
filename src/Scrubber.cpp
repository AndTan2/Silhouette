#include "Scrubber.hpp"
#include <algorithm>




Scrubber::Scrubber()
{
    vp = nullptr;

}

Scrubber::~Scrubber() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (shader) delete shader;
}

void Scrubber::init(VideoPlayer* videoPlayer)
{
    vp = videoPlayer;

    maxZoom = vp->durationSeconds() * vp->fps() / numFramesAtMaxZoom;

    scrubberY0 = 0.0f;

    // Create shader
    shader = new Shader("C:/Users/eu/source/repos/AndTan2/Silhouette/src/scrubber.vert", "C:/Users/eu/source/repos/AndTan2/Silhouette/src/scrubber.frag");

    // Get uniform locations
    colorUniformLoc = glGetUniformLocation(shader->ID, "color");
    transformUniformLoc = glGetUniformLocation(shader->ID, "transform");

    // Create geometry
    createGeometry();
}

void Scrubber::createGeometry() {
    // Quad vertices: positions only (x, y)
    // We'll transform these in the shader
    vertices = {
        // Positions (x, y)
        0.0f, 0.0f,  // bottom-left
        1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f,  // top-right
        0.0f, 1.0f   // top-left
    };

    indices = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };

    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
        vertices.data(), GL_STATIC_DRAW);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

float Scrubber::timelineToX(float t) {
    float x = (t - interpolatedOffset) * zoomFactor * (float)vp->width();
    return x;
}

void Scrubber::renderQuad(float x, float y, float width, float height,
    float r, float g, float b,
    int windowWidth, int windowHeight) {
    if (!shader || windowWidth <= 0 || windowHeight <= 0) return;

    // Calculate screen to NDC transformation manually
    // This works regardless of current matrix state
    float x0 = (2.0f * x / windowWidth) - 1.0f;
    float x1 = (2.0f * (x + width) / windowWidth) - 1.0f;
    float y0 = (2.0f * y / windowHeight) - 1.0f;
    float y1 = (2.0f * (y + height) / windowHeight) - 1.0f;

    // Update the vertex positions directly
    std::vector<float> quadVertices = {
        x0, y0,  // bottom-left
        x1, y0,  // bottom-right
        x1, y1,  // top-right
        x0, y1   // top-left
    };

    // Update VBO with new positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        quadVertices.size() * sizeof(float),
        quadVertices.data());

    // Use simple shader with identity matrix
    shader->Activate();
    glUniform3f(colorUniformLoc, r, g, b);

    // Draw
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Scrubber::zoom(float scrollAmount, float mouseX, float windowWidth)
{
    if (vp == nullptr || windowWidth <= 0) return;  // Check windowWidth, not vp->width()

    float oldZoom = zoomFactor;
    float factor = 1.0f + 0.1f * scrollAmount;
    if (factor <= 0.0f) return;

    zoomFactor *= factor;
    if (zoomFactor < minZoom) zoomFactor = minZoom;
    if (zoomFactor > maxZoom) zoomFactor = maxZoom;

    // FIX THIS LINE: Use windowWidth, not vp->width()
    float cursorNorm = mouseX / windowWidth;

    float timeUnderCursor = offset + cursorNorm / oldZoom;
    offset = timeUnderCursor - cursorNorm / zoomFactor;

    if (offset < 0.0f) offset = 0.0f;
    float maxOffset = 1.0f - 1.0f / zoomFactor;
    if (offset > maxOffset) offset = maxOffset;

    interpolatedOffset = offset;
    panning = true;
}


void Scrubber::draw(int windowWidth, int windowHeight) {
    if (!vp || !shader) return;

    currentWindowWidth = windowWidth;
    currentWindowHeight = windowHeight;

    // Save OpenGL state
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Calculate scrubber height based on window
    float markerY = windowHeight / 8;
    float minScrubberHeight = windowHeight / 20.0f;
    float maxScrubberHeight = ((float)windowWidth / (float)numFramesAtMaxZoom) * (9.0f / 16.0f);
    float normalizedZoom = zoomFactor / maxZoom;
    float growthStart = 0.2f;

    float scrubberHeight;
    if (normalizedZoom < growthStart) {
        float logT = std::log(1.0f + normalizedZoom / growthStart * 9.0f) / std::log(10.0f);
        scrubberHeight = minScrubberHeight * (1.0f + 0.3f * logT);
    }
    else {
        float t = (normalizedZoom - growthStart) / (1.0f - growthStart);
        t = t * t * (3.0f - 2.0f * t);
        scrubberHeight = minScrubberHeight * 1.3f + t * (maxScrubberHeight - minScrubberHeight * 1.3f);
    }

    scrubberMarkerY = markerY;
    scrubberBarHeight = scrubberHeight;

    // 1. Draw background - use window dimensions, not video dimensions!
    renderQuad(0.0f, markerY, (float)windowWidth, scrubberHeight,
        0.3f, 0.3f, 0.3f, windowWidth, windowHeight);

    // Convert timeline to screen X
    auto timelineToScreenX = [this, windowWidth](float t) -> float {
        return (t - interpolatedOffset) * zoomFactor * (float)windowWidth;
        };

    // 2. Draw cache markers (green)
    if (!vp->frameCache.empty() && vp->durationSeconds() > 0.0) {
        float maxMarkerWidth = scrubberHeight * (16.0f / 9.0f);
        float gap = maxMarkerWidth * (1.0f / 8.0f);
        float markerWidth = (float)windowWidth * zoomFactor / (vp->durationSeconds() * vp->fps());
        if (markerWidth < 2.0f) markerWidth = 2.0f;
        if (markerWidth > maxMarkerWidth + gap) markerWidth = maxMarkerWidth + gap;
        float markerHeight = scrubberHeight;
        

        float currentTime = vp->currentCacheTimeSeconds();
        const CachedFrame* selectedFrame = nullptr;
        float minTimeDiff = std::numeric_limits<float>::max();

        for (const auto& frame : vp->frameCache) {
            float frameTime = vp->ptsToSeconds(frame.pts);
            float timeDiff = std::abs(frameTime - currentTime);
            if (timeDiff < minTimeDiff) {
                minTimeDiff = timeDiff;
                selectedFrame = &frame;
            }
        }

        for (const auto& frame : vp->frameCache) {
            float timelinePos = vp->ptsToSeconds(frame.pts) / vp->durationSeconds();
            float screenX = timelineToScreenX(timelinePos);

            float r, g, b;
            if (&frame == selectedFrame && minTimeDiff < 0.1f) {
                r = 1.0f; g = 1.0f; b = 1.0f;
            }
            else {
                r = 0.0f; g = 1.0f; b = 0.0f;
            }

            renderQuad(screenX - markerWidth / 2 + gap / 2, markerY,
                markerWidth - gap, markerHeight,
                r, g, b, windowWidth, windowHeight);
        }
    }

    // 3. Draw keyframe markers (yellow)
  /*  if (!vp->KeyFrames.empty() && vp->durationSeconds() > 0.0) {
        for (auto pts : vp->KeyFrames) {
            float timelinePos = vp->ptsToSeconds(pts) / vp->durationSeconds();
            float screenX = timelineToScreenX(timelinePos);

            if (screenX + 1.0f < 0.0f || screenX - 1.0f > windowWidth) continue;

            renderQuad(screenX - 1.0f, markerY, 2.0f, scrubberHeight,
                1.0f, 1.0f, 0.0f, windowWidth, windowHeight);
        }
    }*/

    // 4. Draw playhead (white)
    if (vp->durationSeconds() > 0.0) {
        float playheadPos = vp->currentCacheTimeSeconds() / vp->durationSeconds();
        float screenX = timelineToScreenX(playheadPos);

        if (screenX + 1.0f >= 0.0f && screenX - 1.0f <= windowWidth) {
            renderQuad(screenX - 1.0f, markerY, 2.0f, scrubberHeight,
                1.0f, 1.0f, 1.0f, windowWidth, windowHeight);
        }
    }

    glPopAttrib();
}

// Helper to convert screen coordinates to timeline position
float Scrubber::screenXToTimeline(float screenX, int windowWidth) {
    // Convert screen X to normalized timeline position
    float normalizedX = screenX / windowWidth;
    return interpolatedOffset + normalizedX / zoomFactor;
}



void Scrubber::addPanDelta(float dx)
{
    if (vp == nullptr || vp->width() <= 0) return;

    float visibleWidth = 1.0f / zoomFactor;
    float deltaNormalized = dx / (float)currentWindowWidth * visibleWidth;

    offset -= deltaNormalized;

    panning = true;
}


void Scrubber::update(float dt)
{
    float alpha = 1.0f;

    if (!panning)
        interpolatedOffset += (offset - interpolatedOffset) * alpha;
    else
        interpolatedOffset = offset;


    if (interpolatedOffset < 0.0f) interpolatedOffset = 0.0f;
    float maxOffset = 1.0f - 1.0f / zoomFactor;
    if (interpolatedOffset > maxOffset) interpolatedOffset = maxOffset;

    panning = false;
}