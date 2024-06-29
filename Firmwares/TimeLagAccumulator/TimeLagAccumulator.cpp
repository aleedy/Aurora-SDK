/** Time Lag Accumulator
 */
#include "aurora.h"
#include "daisysp.h"

using namespace daisy;
using namespace aurora;
using namespace daisysp;


/** Our global hardware object */
Hardware hw;

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
		StereoSample Read(float delay) {
			return {
				stereoDelayLine[0].Read(delay),
				stereoDelayLine[1].Read(delay),
			};
		};
	
};

StereoDelay stereoDelay;
float timeDeadZone = .005;
float smoothTime = 0.01;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{

	/** This filters, and prepares all of the module's controls for us. */
	hw.ProcessAllControls();

	float delayTime = hw.GetKnobValue(KNOB_TIME);
	float feedback = hw.GetKnobValue(KNOB_REFLECT);
	float send = hw.GetKnobValue(KNOB_BLUR);
	float mix = hw.GetKnobValue(KNOB_MIX);
	float dryLeft, dryRight, wetLeft, wetRight;

	hw.SetLed(LED_1, delayTime, 0.0f, 0.0f);
	hw.WriteLeds();

	/** Loop through each sample of audio */
	for (size_t i = 0; i < size; i++)
	{

		dryLeft = in[0][i];
		dryRight = in[1][i];
		if(std::abs(smoothTime-delayTime)>timeDeadZone) {
        	fonepole(smoothTime, delayTime, 0.0001f);
		}


		float delayPosition0 = maxDelaySamples*smoothTime;
		StereoSample wetSample = stereoDelay.Read(delayPosition0);
		
		wetLeft = wetSample.left;
		wetRight = wetSample.right;

		stereoDelay.Write(dryLeft*send + wetLeft*feedback, dryRight*send + wetRight*feedback);

	// 	/** Left Channel */
		out[0][i] = (dryLeft*(1-mix) + wetLeft*mix);

	// 	/** Right Channel */
		out[1][i] = (dryRight*(1-mix) + wetRight*mix);
	}
}

int main(void)
{
	/** Initialize the Hardware */
	hw.Init();
	stereoDelay.Init();
	stereoDelay.SetDelay(maxDelaySamples);
	hw.SetLed(LED_FREEZE, 0.5f, 0.0f, 0.5f);
	hw.WriteLeds();

	/** Start the audio engine calling the function defined above periodically */
	hw.StartAudio(AudioCallback);


	/** Infinite Loop */
	while (1)
	{
	}
}
