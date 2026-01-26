#include "Camera.hpp"

void Camera::beginFrame(double dt)
{
	float alpha = 1.0f - std::exp(-_smoothK * (float)dt);

	_interpolatedZoom += (_zoom - _interpolatedZoom) * alpha;

	if (_panning)
	{
		_interpolatedPanX = _panX;
		_interpolatedPanY = _panY;
	}
	else
	{
		_interpolatedPanX += (_panX - _interpolatedPanX) * alpha;
		_interpolatedPanY += (_panY - _interpolatedPanY) * alpha;
	}
}

void Camera::addPanDelta(float dx, float dy)
{
	_panX += dx;
	_panY += dy;
}

void Camera::onScroll(float scrollY, float mouseX, float mouseY, int screenWidth, int screenHeight)
{

	if (scrollY != 0.0f)
	{
		float oldZoom = _zoom;

		float factor = 1.0f + 0.1f * static_cast<float>(scrollY);
		if (factor > 0.0f)
		{
			_zoom *= factor;
			if (_zoom < 0.1f) _zoom = 0.1f;
			if (_zoom > 20.0f) _zoom = 20.0f;

			float scale = _zoom / oldZoom;

			float cx = 0.5f * screenWidth;
			float cy = 0.5f * screenHeight;


			float vx = mouseX - (cx + _panX);
			float vy = (screenHeight - mouseY) - (cy + _panY);

			_panX += (1.0f - scale) * vx;
			_panY += (1.0f - scale) * vy;

			if (scrollY < 0.0f)
			{


				float zNorm = _zoomFullView / _zoom;
				if (zNorm < 0.0f) zNorm = 0.0f;
				if (zNorm > 1.0f) zNorm = 1.0f;


				float curve = std::pow(zNorm, _recenterCurveK);

				float maxRecenterStrength = 0.20f;
				float recenterStrength = maxRecenterStrength * curve;

				_panX *= (1.0f - recenterStrength);
				_panY *= (1.0f - recenterStrength);
			}
		}
	}
}

void Camera::computeImageRect(int screenWidth, int screenHeight, int imageWidth, int imageHeight, float& x0, float& x1, float& y0, float& y1) const
{
	float aspectImage = (float)imageWidth / (float)imageHeight;
	float baseHeight = (float)screenHeight / 2;
	float baseWidth = baseHeight * aspectImage;

	float drawWidth = baseWidth * _interpolatedZoom;
	float drawHeight = baseHeight * _interpolatedZoom;

	float centerX = 0.5f * screenWidth + _interpolatedPanX;
	float centerY = 0.5f * screenHeight + _interpolatedPanY;

	x0 = centerX - drawWidth * 0.5f;
	y0 = centerY - drawHeight * 0.5f;
	x1 = centerX + drawWidth * 0.5f;
	y1 = centerY + drawHeight * 0.5f;
}