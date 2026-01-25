#include "Scrubber.hpp"
#include <algorithm>




Scrubber::Scrubber()
{
    vp = nullptr;
}

void Scrubber::init(VideoPlayer* videoPlayer)
{
    vp = videoPlayer;
    scrubberY0 = 0.0f;

}

void Scrubber::zoom(float scrollAmount, float mouseX, float windowWidth)
{
    if (vp == nullptr || vp->width() <= 0) return;

    float oldZoom = zoomFactor;

    float factor = 1.0f + 0.1f * scrollAmount;
    if (factor <= 0.0f) return;

    zoomFactor *= factor;


    if (zoomFactor < minZoom) zoomFactor = minZoom;
    if (zoomFactor > maxZoom) zoomFactor = maxZoom;


    float cursorNorm = mouseX / (float)vp->width();


    float timeUnderCursor = offset + cursorNorm / oldZoom;


    offset = timeUnderCursor - cursorNorm / zoomFactor;


    if (offset < 0.0f) offset = 0.0f;
    float maxOffset = 1.0f - 1.0f / zoomFactor;
    if (offset > maxOffset) offset = maxOffset;


    interpolatedOffset = offset;
    panning = true;
}







void Scrubber::draw()
{

    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, scrubberY0);
    glVertex2f((float)vp->width(), scrubberY0);
    glVertex2f((float)vp->width(), vp->height() / 8);
    glVertex2f(0.0f, vp->height() / 8);
    glEnd();


    auto timelineToX = [this](float t) -> float {
        float x = (t - interpolatedOffset) * zoomFactor * (float)vp->width();
        return x;
        };


    if (!vp->frameCache.empty() && vp->durationSeconds() > 0.0) {
        glColor3f(0.0f, 1.0f, 0.0f);
        float markerWidth = 2.0f;
        for (const auto& frame : vp->frameCache) {
            float timelinePos = vp->ptsToSeconds(frame.pts) / vp->durationSeconds();
            float x = timelineToX(timelinePos);


            if (x + markerWidth / 2 < 0.0f || x - markerWidth / 2 > vp->width())
                continue;

            glBegin(GL_QUADS);
            glVertex2f(x - markerWidth / 2, scrubberY0);
            glVertex2f(x + markerWidth / 2, scrubberY0);
            glVertex2f(x + markerWidth / 2, vp->height() / 8);
            glVertex2f(x - markerWidth / 2, vp->height() / 8);
            glEnd();
        }
    }


    if (!vp->KeyFrames.empty() && vp->durationSeconds() > 0.0) {
        glColor3f(1.0f, 1.0f, 0.0f);
        float markerWidth = 2.0f;
        for (auto pts : vp->KeyFrames) {
            float timelinePos = vp->ptsToSeconds(pts) / vp->durationSeconds();
            float x = timelineToX(timelinePos);

            if (x + markerWidth / 2 < 0.0f || x - markerWidth / 2 > vp->width())
                continue;

            glBegin(GL_QUADS);
            glVertex2f(x - markerWidth / 2, scrubberY0);
            glVertex2f(x + markerWidth / 2, scrubberY0);
            glVertex2f(x + markerWidth / 2, vp->height() / 8);
            glVertex2f(x - markerWidth / 2, vp->height() / 8);
            glEnd();
        }
    }


    if (vp->durationSeconds() > 0.0) {
        float playheadPos = vp->currentCacheTimeSeconds() / vp->durationSeconds();
        float x = timelineToX(playheadPos);
        float playheadWidth = 2.0f;

        if (x + playheadWidth / 2 >= 0.0f && x - playheadWidth / 2 <= vp->width()) {
            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(x - playheadWidth / 2, scrubberY0);
            glVertex2f(x + playheadWidth / 2, scrubberY0);
            glVertex2f(x + playheadWidth / 2, vp->height() / 8);
            glVertex2f(x - playheadWidth / 2, vp->height() / 8);
            glEnd();
        }
    }

    glColor3f(1.0f, 1.0f, 1.0f);
}


void Scrubber::addPanDelta(float dx)
{
    if (vp == nullptr || vp->width() <= 0) return;

    float visibleWidth = 1.0f / zoomFactor;
    float deltaNormalized = dx / (float)vp->width() * visibleWidth;

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
