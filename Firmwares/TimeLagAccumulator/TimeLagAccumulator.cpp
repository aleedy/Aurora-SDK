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

const int MAX_DELAY_LINES = 2;
StereoDelay stereoDelay[MAX_DELAY_LINES];
float timeDeadZone = .005;
float smoothTime[MAX_DELAY_LINES] = {0.0f, 0.0f};
int numDelayLines = 0;
int delayLineEnabled[MAX_DELAY_LINES];

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{

	/** This filters, and prepares all of the module's controls for us. */
	hw.ProcessAllControls();
	bool state = hw.GetButton(SW_REVERSE).Pressed();
	if (state) {
		numDelayLines = (numDelayLines+1) % MAX_DELAY_LINES;
	}
	for (int i = 0; i < MAX_DELAY_LINES; i++) {
		if(i <= numDelayLines) {
			delayLineEnabled[i] = 1;
		} else {
			delayLineEnabled[i] = 0; 
		}
	}
	
	switch (numDelayLines) {
		case 0 :
			hw.SetLed(LED_REVERSE, 0.0f, 0.0f, 1.0f);
			break;
		case 1: 
			hw.SetLed(LED_REVERSE, 0.0f, 1.0f, 0.0f);
			break;
		default: 
			hw.SetLed(LED_REVERSE, 1.0f, 0.0f, 0.0f);
			break;
	}

	float delayTime[2] = {
		fmap(hw.GetKnobValue(KNOB_TIME), 0.0, 1.0, Mapping::LINEAR),
		fmap(hw.GetKnobValue(KNOB_TIME), 0.0, 0.5, Mapping::LINEAR),
	};
	float feedback = hw.GetKnobValue(KNOB_REFLECT);
	float send = hw.GetKnobValue(KNOB_BLUR);
	float mix = hw.GetKnobValue(KNOB_MIX);
	float dryLeft, dryRight, wetLeft, wetRight;

	hw.SetLed(LED_3, delayTime[0], 0.0f, 1.0f);
	hw.SetLed(LED_4, delayTime[1], 0.0f, 1.0f);
	hw.WriteLeds();

	/** Loop through each sample of audio */
	for (size_t i = 0; i < size; i++)
	{

		dryLeft = in[0][i];
		dryRight = in[1][i];
		if(std::abs(smoothTime[0]-delayTime[0])>timeDeadZone) {
        	fonepole(smoothTime[0], delayTime[0], 0.0001f);
		}
		float delayPosition0 = maxDelaySamples*smoothTime[0];

		if(std::abs(smoothTime[1]-delayTime[1])>timeDeadZone) {
        	fonepole(smoothTime[1], delayTime[1], 0.0001f);
		}
		float delayPosition1 = maxDelaySamples*smoothTime[1];

		StereoSample wetSample[2] = {
			stereoDelay[0].Read(delayPosition0),
			stereoDelay[1].Read(delayPosition1),
		};
		
		wetLeft = wetSample[0].left*delayLineEnabled[0] + wetSample[1].left*delayLineEnabled[1];
		wetRight = wetSample[0].right*delayLineEnabled[0] + wetSample[1].right*delayLineEnabled[1];

		stereoDelay[0].Write(dryLeft*send + wetSample[0].left*feedback, dryRight*send + wetSample[0].right*feedback);
		stereoDelay[1].Write(dryLeft*send + wetSample[1].left*feedback, dryRight*send + wetSample[1].right*feedback);

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
	stereoDelay[0].Init();
	stereoDelay[0].SetDelay(maxDelaySamples);
	stereoDelay[1].Init();
	stereoDelay[1].SetDelay(maxDelaySamples);
	hw.SetLed(LED_FREEZE, 0.5f, 0.0f, 0.5f);
	hw.WriteLeds();

	/** Start the audio engine calling the function defined above periodically */
	hw.StartAudio(AudioCallback);


	/** Infinite Loop */
	while (1)
	{
	}
}
