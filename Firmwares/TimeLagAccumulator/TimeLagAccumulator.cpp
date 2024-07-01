/** Time Lag Accumulator
 */
#include "aurora.h"
#include "daisysp.h"
#include "stereodelay.h"

using namespace daisy;
using namespace aurora;
using namespace daisysp;


/** Our global hardware object */
Hardware hw;
static CrossFade outputMixer;

float mix = 0.5f;

StereoDelay stereoDelay;
float timeDeadZone = .005;
float smoothTime = 0.01;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{

	/** This filters, and prepares all of the module's controls for us. */
	hw.ProcessAllControls();

	float delayKnobValue = hw.GetKnobValue(KNOB_TIME);
	float feedback = hw.GetKnobValue(KNOB_REFLECT);
	
	float send = hw.GetKnobValue(KNOB_BLUR);
	fonepole(mix, hw.GetKnobValue(KNOB_MIX), 0.001f);
	outputMixer.SetPos(mix);
	float dryLeft, dryRight, wetLeft, wetRight;

	hw.SetLed(LED_1, 1-delayKnobValue, 0.0f, 1.0f);
	hw.WriteLeds();

	/** Loop through each sample of audio */
	for (size_t i = 0; i < size; i++)
	{

		dryLeft = in[0][i];
		dryRight = in[1][i];
	
		if(std::abs(smoothTime-delayKnobValue)>timeDeadZone) {
				fonepole(smoothTime, delayKnobValue, 0.0001f);
		}
		
		StereoSample wetSample = {0.0f, 0.0f};
		float delayPosition = maxDelaySamples*smoothTime;
		wetSample = stereoDelay.Read(delayPosition);

		float sendLeft = dryLeft*send + wetSample.left*feedback;
		float sendRight = dryRight*send + wetSample.right*feedback;
		stereoDelay.Write(
			sendLeft, 
			sendRight
			);
		
		wetLeft = wetSample.left;
		wetRight = wetSample.right;

	// 	/** Left Channel */
		out[0][i] = outputMixer.Process(dryLeft, wetLeft);

	// 	/** Right Channel */
		out[1][i] = outputMixer.Process(dryRight, wetRight);
	}
}

int main(void)
{
	/** Initialize the Hardware */
	hw.Init();
	stereoDelay.Init();
	stereoDelay.SetDelay(maxDelaySamples);

	outputMixer.Init(CROSSFADE_CPOW);

	hw.SetLed(LED_6, 0.5f, 0.0f, 0.5f);
	hw.SetLed(LED_FREEZE, 0.0f, 1.0f, 0.0f);

	hw.WriteLeds();

	/** Start the audio engine calling the function defined above periodically */
	hw.StartAudio(AudioCallback);


	/** Infinite Loop */
	while (1)
	{
	}
}
