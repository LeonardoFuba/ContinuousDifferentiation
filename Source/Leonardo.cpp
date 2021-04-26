#include <stdio.h>
#include <iostream>
#include "Leonardo.h"
#include "LeonardoEditor.h"

using namespace LeonardoSpace;

//Change all names for the relevant ones, including "Processor Name"
Leonardo::Leonardo()
  : GenericProcessor ("EP detection")
  , defaultStartTime (-5.0f)
  , defaultEndTime   (55.0f)
  , defaultDiffScale (1000.0f)
{
  setProcessorType(PROCESSOR_TYPE_FILTER);
  applyDiff = false;
  time = 0;
}

Leonardo::~Leonardo() {
}

AudioProcessorEditor* Leonardo::createEditor()
{
    editor = new LeonardoEditor (this, true);

    LeonardoEditor* ed = (LeonardoEditor*) getEditor();
    ed->setDefaults (defaultStartTime, defaultEndTime, defaultDiffScale);

    return editor;
}

void Leonardo::setDiff(bool state)
{
    applyDiff = state;
};

void Leonardo::setTime(int t)
{
  time = t;
};

int Leonardo::getTime()
{
  return time;
};

/** Optional method called every time the signal chain is refreshed or changed in any way.

  Allows the processor to handle variations in the channel configuration or any other parameter
  passed down the signal chain. The processor can also modify here the settings structure, which contains
  information regarding the input and output channels as well as other signal related parameters. Said
  structure shouldn't be manipulated outside of this method.

  */
void Leonardo::updateSettings()
{

}

double Leonardo::getStartTimeValueForChannel (int chan) const
{
    return startTime[chan];
}

double Leonardo::getEndTimeValueForChannel (int chan) const
{
    return endTime[chan];
}

double Leonardo::getDiffScaleValueForChannel(int chan) const
{
    return diffScale[chan];
}


void Leonardo::setParameter (int parameterIndex, float newValue)
{
  if (parameterIndex < 2) // change time settings
    {
        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;

        if (parameterIndex == 0)
        {
            startTime.set (currentChannel,newValue);
        }
        else if (parameterIndex == 1)
        {
            endTime.set (currentChannel,newValue);
        }

        editor->updateParameterButtons (parameterIndex);
    }

  editor->updateParameterButtons (parameterIndex);

  if (currentChannel >= 0)
  {
    Parameter* p =  parameters[parameterIndex];
    p->setValue (newValue, currentChannel);
  }
}

void Leonardo::process(AudioSampleBuffer& buffer)
{
  /** 
  If the processor needs to handle events, this method must be called onyl once per process call
  If spike processing is also needing, set the argument to true
  */
  //checkForEvents(false);
  const int nChannels = buffer.getNumChannels(); //16  
  

  for (int ch = 0; ch < nChannels ; ++ch)
  {

    const int numSamples = buffer.getNumSamples();
    const int64 timestamp = getTimestamp(ch);
    float* bufPtr = buffer.getWritePointer(ch); //ponteiro para escrever no canal ch em uV
    int time = getTime();
    double y = 0;

    if(false){
      for (int n = 0; n < numSamples; ++n)
        
      {
        y = n ;

        if (time % 2 )
          *(bufPtr + n) = y;
        else
          *(bufPtr + n) = numSamples - y ;
      }
    }

    //derivada
    if (applyDiff && ch < 4)
    {
      // const int numSamples = buffer.getNumSamples(); //tamanho do buffer (1024)
      // float* bufPtr = buffer.getWritePointer (ch); //ponteiro para escrever no canal ch em uV
      for (int n = 0; n < numSamples-1; n++)
      {
          *(bufPtr + n) = ( *(bufPtr + n + 1) - ( *(bufPtr + n) )) * 1; //mudar ordem de grandeza
          // 
      }
      *(bufPtr + numSamples - 1) = 0;
    }
    
  }
   
  setTime(getTime() + 1);

} // end process

void Leonardo::saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelType)
{
    if (channelType == InfoObjectCommon::DATA_CHANNEL
        && channelNumber > -1
        && channelNumber < startTime.size())
    {
        //std::cout << "Saving custom parameters for filter node." << std::endl;

        XmlElement* channelParams = channelInfo->createNewChildElement ("PARAMETERS");
        channelParams->setAttribute ("startTime",       startTime[channelNumber]);
        channelParams->setAttribute ("endTime",         endTime[channelNumber]);
        channelParams->setAttribute ("diffScale",        diffScale[channelNumber]);
    }
}


void Leonardo::loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)
{
    int channelNum = channelInfo->getIntAttribute ("number");

    if (channelType == InfoObjectCommon::DATA_CHANNEL)
    {
        // restore high and low cut text in case they were changed by channelChanged
        static_cast<LeonardoEditor*>(getEditor())->resetToSavedText();

        forEachXmlChildElement (*channelInfo, subNode)
        {
            if (subNode->hasTagName ("PARAMETERS"))
            {
                startTime.set (channelNum, subNode->getDoubleAttribute ("startTime", defaultStartTime));
                endTime.set  (channelNum, subNode->getDoubleAttribute ("endTime",  defaultEndTime));
                diffScale.set (channelNum, subNode->getDoubleAttribute ("diffScale", defaultDiffScale));
                shouldDiffChannel.set(channelNum, subNode->getBoolAttribute("applyDiff", true));

                // setFilterParameters(startTime[channelNum], endTime[channelNum], diffScale[channelNum], channelNum);
            }
        }
    }
}
