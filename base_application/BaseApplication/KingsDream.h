/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KINGSDREAM_H
#define KINGSDREAM_H

/*
I derived this animation from the static image tutorial found here:
http://nathanselikoff.com/resources/tutorial-strange-attractors-in-c-and-opengl

I don't know if he was the original creator.  No license was specified.
*/

#include <algorithm>
#include <list>

namespace Emunisce {

class ScreenBuffer;

class KingsDream {
public:
	KingsDream();

	void UpdateAnimation();  ///< Applies one point to the frame.  Expects PointsPerFrame calls per frame.
	ScreenBuffer* GetFrame();

	// Properties

	void SetScreenResolution(
		unsigned int width,
		unsigned int height);  ///< Determines the size of the ScreenBuffer returned from GetFrame.  Default is 320x240.

	void SetAnimationRate(float incrementPerFrame);  ///< The independent variable of the generator is incremented by
													 ///< this value each frame.  Default is 0.002f.
	float GetAnimationRate();

	void SetBrightness(
		unsigned int brightness);  ///< The value each point is incremented by.  Default is 5.  This property can
								   ///< overflow if set too high.  Note that PointsPerFrame can also affect brightness.
	unsigned int GetBrightness();

	void SetFramesPerColor(unsigned int numFrames);  ///< The number of frames required to complete one color transition
	unsigned int GetFramesPerColor();

	void SetPointsPerFrame(
		unsigned int numPoints);  ///< One point is applied per call to UpdateAnimation.  Default is 20000.
	unsigned int GetPointsPerFrame();

	void SetBlendFrames(unsigned int numFrames);  ///< Number of frames to blend together.  Default is 5.  Max is 10.
	unsigned int GetBlendFrames();

private:
	void ResizeScreenBuffers(unsigned int width, unsigned int height);

	void IncrementGenerator();
	void BlendBuffers();

	inline void SilentDream();
	inline void Dream();

	ScreenBuffer* m_screenBuffer;

	static const unsigned int m_maxNumBlendFrames = 10;  ///< Maximum number of frames that can be blended together.
	unsigned int m_numBlendFrames;
	ScreenBuffer* m_frames[m_maxNumBlendFrames];
	unsigned int m_currentFrame;

	float m_incrementPerFrame;  ///< The generator's independent variable (m_a) gets incremented by this value each
								///< frame

	unsigned int m_brightness;

	unsigned int m_framesPerColor;
	unsigned int m_pointsPerColor;
	unsigned int m_pointsThisColor;

	unsigned int m_pointsPerFrame;
	unsigned int m_pointsThisFrame;

	float m_x, m_y;
	float m_a, m_b, m_c, m_d;

	std::list<std::pair<float, float> > m_skipRanges;
};

}  // namespace Emunisce

#endif
