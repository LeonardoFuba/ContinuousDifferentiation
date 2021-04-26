#include <stdio.h>
#include <iostream>
#include "Leonardo.h"

using namespace LeonardoSpace;

//Change all names for the relevant ones, including "Processor Name"
Leonardo::Leonardo() : GenericProcessor("EP detection")
{
	setProcessorType(PROCESSOR_TYPE_FILTER);
}

Leonardo::~Leonardo()
{

}

void Leonardo::process(AudioSampleBuffer& buffer)
{
	/** 
	If the processor needs to handle events, this method must be called onyl once per process call
	If spike processing is also needing, set the argument to true
	*/
	//checkForEvents(false);
	int numChannels = getNumOutputs();

	for (int chan = 0; chan < numChannels; chan++)
	{
		int numSamples = getNumSamples(chan);
		int64 timestamp = getTimestamp(chan);
		float* bufPtr = buffer.getWritePointer(chan);

		for (int n = 0; n < numSamples; ++n)  
      *(bufPtr + n) = n;
	}

}

