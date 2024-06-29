#pragma once
#include "daisysp.h"

using namespace daisysp;
static const float maxDelay = 20.0f; //Max delay in seconds
static const float defaultSamplerate = 48000.0f;
static const size_t maxDelaySamples = ((size_t) (maxDelay*defaultSamplerate));

DelayLine<float, maxDelaySamples> DSY_SDRAM_BSS stereoDelayLine[2];

struct StereoSample{
	float left;
	float right;
};

class StereoDelay {
	public:
		void Init(){
			stereoDelayLine[0].Init();
			stereoDelayLine[1].Init();
            SetDelay(maxDelaySamples);
		};
		void SetDelay(float delay) {
			stereoDelayLine[0].SetDelay(delay);
			stereoDelayLine[1].SetDelay(delay);
		}
		void Write(float left, float right) {
			stereoDelayLine[0].Write(left);
			stereoDelayLine[1].Write(right);
		};
		void Write(StereoSample sample) {
			stereoDelayLine[0].Write(sample.left);
			stereoDelayLine[1].Write(sample.right);
		};
        size_t getMaxDelaySamples() {
            return maxDelaySamples;
        }
		StereoSample Read(float delay) {
			return {
				stereoDelayLine[0].Read(delay),
				stereoDelayLine[1].Read(delay),
			};
		};
};