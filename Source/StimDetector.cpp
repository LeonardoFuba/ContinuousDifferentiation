/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "StimDetector.h"
#include "StimDetectorEditor.h"
#include <math.h>
#include <iostream>

using namespace StimDetectorSpace;

StimDetector::StimDetector()
    : GenericProcessor      ("Stim Detector")
    , activeModule          (-1)
    , defaultThreshold      (100.0f)
{
    setProcessorType (PROCESSOR_TYPE_FILTER);
	lastNumInputs = 1;
    count = -1;

    for (int i = 0; i < AVG_LENGTH; i++)
    {
        media[i] = 0;
    }

}

StimDetector::~StimDetector()
{
}

AudioProcessorEditor* StimDetector::createEditor()
{
    editor = new StimDetectorEditor (this, true);
    StimDetectorEditor* ed = (StimDetectorEditor*) getEditor();
    ed->setDefaults(defaultThreshold);
    std::cout << "Creating editor." << std::endl;

    return editor;
}

void StimDetector::addModule()
{
    DetectorModule m = DetectorModule();
    m.inputChan = -1;
    m.gateChan = -1;
    m.outputChan = -1;
    m.samplesSinceTrigger = 5000;
    m.lastSample = 0.0f;
    m.lastDiff = 0.0f;
    m.applyDiff = false;
    m.isActive = true;
    m.wasTriggered = false;
    m.startStim = false;
    m.detectorStim = true;
    m.startIndex = -1;
    m.windowIndex = -1;
    m.count = -1;

    //for (int i = 0; i < AVG_LENGTH; i++)
        //m.avg[i] = 0;

    modules.add (m);
}

void StimDetector::setActiveModule (int i)
{
    activeModule = i;
}

void StimDetector::setParameter (int parameterIndex, float newValue)
{
    DetectorModule& module = modules.getReference (activeModule);

    if (parameterIndex == 1) // empty
    {
        module.applyDiff = (bool)newValue;
    }
    else if (parameterIndex == 2)   // inputChan
    {
        module.inputChan = (int) newValue;
    }
    else if (parameterIndex == 3)   // outputChan
    {
        module.outputChan = (int) newValue;
    }
    else if (parameterIndex == 4)   // gateChan
    {
        module.gateChan = (int) newValue;
        if (module.gateChan < 0)
        {
            module.detectorStim = true;
        }
        else
        {
            module.detectorStim = false;
        }
    }
    else if(parameterIndex == 5) // threshold
    {
        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;
            
        thresholds.set (currentChannel,newValue);
        editor->updateParameterButtons (parameterIndex);
    }

}

//Usually, to be more ordered, we'd create the event channels overriding the createEventChannels() method.
//However, since in this case there a couple of things we need to do prior to creating the channels (resetting
//the modules input channels in case the channel count changes, to reflect the same change on the combo box)
//we think it's better to do all in this method, that gets always called after all the create*Channels.
void StimDetector::updateSettings()
{
    int numInputs = getNumInputs();
    if (numInputs < 1024)
    {
        Array<double> oldThresholds;
        oldThresholds = thresholds;
        thresholds.clear();

        for (int n = 0; n < getNumInputs(); ++n)
        {
            float newThreshold = 0.0f;

            if (oldThresholds.size() > n)
            {
                newThreshold = oldThresholds[n];
            }
            else
            {
                newThreshold = defaultThreshold;
            }

            thresholds.add  (newThreshold);
        }
    }

	moduleEventChannels.clear();
	for (int i = 0; i < modules.size(); i++)
	{
  		if (getNumInputs() != lastNumInputs)
			modules.getReference(i).inputChan = -1;
		
        const DataChannel* in = getDataChannel(modules[i].inputChan);
		EventChannel *ev;
        String identifier = "dataderived.phase.peak.positve";
        String typeDesc = "Positive peak";

		if (in)
			ev = new EventChannel(EventChannel::TTL, 8, 1, in, this);
		else
			ev = new EventChannel(EventChannel::TTL, 8, 1, -1, this);

		ev->setName("Stim detector output " + String(i + 1));
		ev->setDescription("Triggers when the input signal mets a given phase condition");
		ev->setIdentifier(identifier);
		MetaDataDescriptor md(MetaDataDescriptor::CHAR, 34, "Stim Type", "Description of the phase condition", "channelInfo.extra");
		MetaDataValue mv(md);
		mv.setValue(typeDesc);
		ev->addMetaData(md, mv);
		if (in)
		{
			md = MetaDataDescriptor(MetaDataDescriptor::UINT16, 3, "Source Channel",
				"Index at its source, Source processor ID and Sub Processor index of the channel that triggers this event", "source.channel.identifier.full");
			mv = MetaDataValue(md);
			uint16 sourceInfo[3];
			sourceInfo[0] = in->getSourceIndex();
			sourceInfo[1] = in->getSourceNodeID();
			sourceInfo[2] = in->getSubProcessorIdx();
			mv.setValue(static_cast<const uint16*>(sourceInfo));
			ev->addMetaData(md, mv);
		}
		eventChannelArray.add(ev);
		moduleEventChannels.add(ev);
	}
	lastNumInputs = getNumInputs();
}

bool StimDetector::enable()
{
    return true;
}

void StimDetector::handleEvent(const EventChannel* channelInfo, const MidiMessage& event, int sampleNum)
{
    // MOVED GATING TO PULSE PAL OUTPUT!
    // now use to randomize phase for next trial

    //std::cout << "GOT EVENT." << std::endl;

    if (Event::getEventType(event) == EventChannel::TTL)
    {
        TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, channelInfo);

        // int eventNodeId = *(dataptr+1);
        const int eventId = ttl->getState() ? 1 : 0;
        const int eventChannel = ttl->getChannel();

        for (int i = 0; i < modules.size(); ++i)
        {
            DetectorModule& module = modules.getReference(i);

            if (module.gateChan == eventChannel)
            {
                if (eventId)
                {
                    module.startStim = true;
                    module.detectorStim = false;
                }
                else {
                    module.startStim = false;
                }
            }
        }
    }
}

void StimDetector::process(AudioSampleBuffer& buffer)
{
    checkForEvents();

    // loop through the modules
    for (int m = 0; m < modules.size(); ++m)
    {
        DetectorModule& module = modules.getReference(m);

        // check to see if it's active and has a channel
        if (module.outputChan >= 0
            && module.inputChan >= 0
            && module.inputChan < buffer.getNumChannels())
        {
            int bufferLength = getNumSamples(module.inputChan);
            for (int i = 0; i < bufferLength; ++i)
            {
                const float sample = *buffer.getReadPointer(module.inputChan, i);
                const float diffSample = sample - module.lastSample;
                if (module.applyDiff)
                {
                    *buffer.getWritePointer(module.inputChan, i) = diffSample;
                }

                if (module.detectorStim)      //gate disableded
                {
                    if (diffSample > module.lastDiff                    //variacao brusca
                        && diffSample > thresholds[module.inputChan]    //acima do limiar
                        && !module.startStim)                           //fora do TTL
                    {
                        //inicia TTL
                        uint8 ttlData = 1 << module.outputChan;
                        TTLEventPtr event = TTLEvent::createTTLEvent(moduleEventChannels[m], getTimestamp(module.inputChan) + i, &ttlData, sizeof(uint8), module.outputChan);
                        addEvent(moduleEventChannels[m], event, i);
                        module.samplesSinceTrigger = 0;
                        module.wasTriggered = true;
                        module.startStim = true;

                        //config avg
                        module.startIndex = i;
                        module.windowIndex = 0;
                        count++;
                    }
                    module.lastSample = sample;
                    module.lastDiff = diffSample;

                    //durante TTL
                    if (module.wasTriggered)
                    {
                        //finalizacao do TTL
                        if (module.samplesSinceTrigger > 10)
                        {
                            uint8 ttlData = 0;
                            TTLEventPtr event = TTLEvent::createTTLEvent(moduleEventChannels[m], getTimestamp(module.inputChan) + i, &ttlData, sizeof(uint8), module.outputChan);
                            addEvent(moduleEventChannels[m], event, i);
                            module.wasTriggered = false;
                        }
                        else
                        {
                            module.samplesSinceTrigger++;
                        }
                    }
                }

                /* TTL gate enableded */
                if (!module.detectorStim && module.startStim)
                {
                    module.startIndex = i;
                    module.windowIndex = 0;
                    count++;
                    module.startStim = false;
                }

                // esta dentro da janela
                if (module.startIndex >= 0 && module.windowIndex < AVG_LENGTH)
                {
                    media[module.windowIndex] = (media[module.windowIndex] * count + sample) / (double)(count + 1);
                    //std::cout << "count=;" << count << ";i="<<i<< ";media[" << module.windowIndex << "] ;" << media[module.windowIndex] << std::endl;
                    module.windowIndex++;

                    //output
                    //*buffer.getWritePointer(module.inputChan, i) = module.avg[module.windowIndex];
                }
                else {
                    // desativa referencias
                    module.startIndex = -1;
                    module.windowIndex = -1;
                    module.startStim = false;
                }
            }
        }
    }
}

double StimDetector::getThresholdValueForChannel(int chan)
{
    return thresholds[chan];
}

void StimDetector::saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelType)
{
    if (channelType == InfoObjectCommon::DATA_CHANNEL
        && channelNumber > -1
        && channelNumber < thresholds.size())
    {
        // std::cout << "Saving custom parameters for filter node." << std::endl;

        XmlElement* channelParams = channelInfo->createNewChildElement ("PARAMETERS");
        channelParams->setAttribute ("threshold", thresholds[channelNumber]);
    }
}

void StimDetector::loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)
{
    int channelNum = channelInfo->getIntAttribute ("number");

    if (channelType == InfoObjectCommon::DATA_CHANNEL)
    {
        // restore high and low cut text in case they were changed by channelChanged
        static_cast<StimDetectorEditor*>(getEditor())->resetToSavedText();

        forEachXmlChildElement (*channelInfo, subNode)
        {
            if (subNode->hasTagName ("PARAMETERS"))
            {
                thresholds.set (channelNum, subNode->getDoubleAttribute ("threshold", defaultThreshold));
                
            }
        }
    }
}
