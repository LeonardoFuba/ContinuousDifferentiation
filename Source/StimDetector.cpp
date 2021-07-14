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

#define PI 3.1415926535897932384626433

using namespace StimDetectorSpace;

StimDetector::StimDetector()
  : GenericProcessor      ("Stim Detector")
  , activeModule          (-1)
  , defaultThreshold      (100.0f)
{
  setProcessorType (PROCESSOR_TYPE_FILTER);
  lastNumInputs = 1;

  avgLength = 1;
  ttlLength = 1;
  buffetMin = 1;
  movMean = 1;
}

StimDetector::~StimDetector()
{
}

AudioProcessorEditor* StimDetector::createEditor()
{
  editor = new StimDetectorEditor (this, true);
  //StimDetectorEditor* ed = (StimDetectorEditor*) getEditor();
  //ed->setDefaults(defaultThreshold);
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
  m.threshold = 0.0f;
  m.yMin = 0.0f;
  m.yMax = 0.0f;
  m.xMin = 0;
  m.xMax = 0;
  m.applyDiff = false;
  m.isActive = true;
  m.wasTriggered = false;
  m.wasTriggered_buffer = false;
  m.startStim = false;
  m.detectorStim = true;
  m.ignoreFirst = true;
  m.startIndex = -1;
  m.windowIndex = -1;
  m.count = 0;
  m.timestamps.resize(avgLength);
  m.stim.resize(avgLength);
  m.avg.resize(avgLength);
  m.stimMean.resize(avgLength);
 
  m.matrix.add (m.avg);
  m.activeRow = 0;
  m.yAvgMin.add(0.0f);
  m.yAvgMax.add(0.0f);
  m.avgSlope.add(0.0f);
  m.avgLatency.add(0.0f);

  modules.add (m);

}

void StimDetector::setActiveModule (int i)
{
  activeModule = i;
}

void StimDetector::setParameter (int parameterIndex, float newValue)
{
  DetectorModule& module = modules.getReference (activeModule);

  if (parameterIndex == 1) // applyDiff
  {
    module.applyDiff = (bool) newValue;
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
  else if (parameterIndex == 5) // threshold
  {
    if (newValue <= 0.01 || newValue >= 10000.0f)
      return;
    
    module.threshold = (double) newValue;
    editor->updateParameterButtons (parameterIndex);
  }
  else if (parameterIndex == 6) // activeRow
  {
    module.activeRow = (int) newValue;
  }
}

//Usually, to be more ordered, we'd create the event channels overriding the createEventChannels() method.
//However, since in this case there a couple of things we need to do prior to creating the channels (resetting
//the modules input channels in case the channel count changes, to reflect the same change on the combo box)
//we think it's better to do all in this method, that gets always called after all the create*Channels.
void StimDetector::updateSettings()
{

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

      if (module.gateChan == eventChannel && module.startIndex < 0) //gate receive TTL outside stim
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

    avgLength = (int)ceil(getDataChannel(module.inputChan)->getSampleRate() * 0.040); //total de pontos que precisamos para olharr o potencial na janela de 40 ms
    ttlLength = (int)ceil(getDataChannel(module.inputChan)->getSampleRate() * 0.005); //dura��o m�xima do TTL (timestamps para ignorar): 5 ms
    movMean = (int)ceil(getDataChannel(module.inputChan)->getSampleRate() * 0.005); //MOVING MEAN WINDOW SIZE 
    //buffetMin = ceil(getDataChannel(module.inputChan)->getSampleRate() / 1024); //Quantos buffers precisamos por segundo de registro (na frente multiplicamos pelo tamanho da janela de 40 ms)

    //std::cout << "AVG_LENGTH " << avgLength << std::endl;
    //std::cout << "TTL_LENGTH " << ttlLength << std::endl;
    //std::cout << "MOV_MEAN "   << movMean << std::endl;

    module.timestamps.resize(avgLength);
    module.stim.resize(avgLength);
    module.avg.resize(avgLength);
    module.stimMean.resize(avgLength);


    // check to see if it's active and has a channel
    if (module.outputChan >= 0
      && module.inputChan >= 0
      && module.inputChan < buffer.getNumChannels())
    {
      int bufferLength = getNumSamples(module.inputChan);
      for (int i = 0; i < bufferLength; ++i)
      {
        const float sample = *buffer.getReadPointer(module.inputChan, i);
        const float diffSample = abs(sample - module.lastSample);

        module.ignoreFirst = (getTimestamp(module.inputChan) == 0 && i == 0);

        if (module.applyDiff)
        {
          *buffer.getWritePointer(module.inputChan, i) = diffSample;
        }

        if (module.detectorStim)                // Gate disableded
        {
          if (diffSample > module.lastDiff      //variacao brusca
          && diffSample > module.threshold      //acima do limiar
          && diffSample < 5 * module.threshold  //ignorar valores muito maiores do limiar
          && !module.startStim                  //fora do TTL
          && !module.ignoreFirst)               //nao e o primeiro
          {
            //start TTL
            uint8 ttlData = 1 << module.outputChan;
            TTLEventPtr event = TTLEvent::createTTLEvent(moduleEventChannels[m], getTimestamp(module.inputChan) + i, &ttlData, sizeof(uint8), module.outputChan);
            addEvent(moduleEventChannels[m], event, i);
            module.samplesSinceTrigger = 0;
            module.wasTriggered = true;
            //module.wasTriggered_buffer = true;
            module.startStim = true;

            //config avg
            module.startIndex = i;
            module.windowIndex = 0;
            module.count++;
          }

          //durante TTL
          if (module.wasTriggered)
          {
          //finalizacao do TTL
            if (module.samplesSinceTrigger > ttlLength)
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
        } // end gate disableded

        /* TTL gate enableded */
        if (!module.detectorStim && module.startStim) //gate receive TTL
        {
          module.startIndex = i;
          module.windowIndex = 0;
          module.count++;
          module.startStim = false;
          //module.wasTriggered_buffer = true;
        }

        // inside window
        if (module.startIndex >= 0 && module.windowIndex < avgLength)
        {
          module.stim.set(module.windowIndex, sample/(0.1950*1000));
          module.timestamps.set(module.windowIndex, getTimestamp(module.inputChan) + i); ///conferir

          // std::cout << module.avg[module.windowIndex] << "*" << module.count - 1 << "+" << sample << "/" << module.count << std::endl;
          module.avg.set(module.windowIndex, (module.avg[module.windowIndex] * ((double)module.count - 1) + sample) / (double)(module.count));
        
          //output
          //*buffer.getWritePointer(module.inputChan, i) = module.avg[module.windowIndex];
          //if(module.windowIndex == AVG_LENGTH - 1) // last window loop
          //{
          //  std::cout << std::endl;
          //} 

          module.windowIndex++;
        }
        else { //avgLength ended
          
          if (module.startStim)
          {
            updateWaveformParams();
            updateActiveAvgLineParams();
            //std::cout << module.xMin << ", " << module.yMin << ", " << module.xMax << ", " << module.yMax << ", " << (((module.yMax - module.yMin) / abs(module.xMax - module.xMin))) << ", " << (module.xMin - module.timestamps[1] + ttlLength) / (getDataChannel(module.inputChan)->getSampleRate()) * 1000 << std::endl;
          }

          // disabled references
          module.startIndex = -1;
          module.windowIndex = -1;
          module.startStim = false;
          module.wasTriggered_buffer = false;
        }

        module.lastSample = sample;
        module.lastDiff = diffSample;
      }
    }
  }
}


//void StimDetector::saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelType)
/*{
  if (channelType == InfoObjectCommon::DATA_CHANNEL
  && channelNumber > -1)
  {
  // std::cout << "Saving custom parameters for stim detector node." << std::endl;

  XmlElement* channelParams = channelInfo->createNewChildElement ("PARAMETERS");
  channelParams->setAttribute ("threshold", thresholds[channelNumber]);
  }
}*/

//void StimDetector::loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)
/*{
  int channelNum = channelInfo->getIntAttribute ("number");

  if (channelType == InfoObjectCommon::DATA_CHANNEL)
  {
  forEachXmlChildElement (*channelInfo, subNode)
  {
    if (subNode->hasTagName ("PARAMETERS"))
    {
    thresholds.set (channelNum, subNode->getDoubleAttribute ("threshold", defaultThreshold));

    }
  }
  }
}*/


void StimDetector::splitAvgArray()
{
  //ScopedLock resetLock(onlineReset);
  //alocar uma nova linha na matriz
  DetectorModule& m = modules.getReference(activeModule);
  
  m.yAvgMin.add(0.0f);
  m.yAvgMax.add(0.0f);
  m.avgSlope.add(0.0f);
  m.avgLatency.add(0.0f);

  m.matrix.resize(m.activeRow + 2);
  m.matrix[m.activeRow + 1].resize(m.matrix[0].size());

  m.count = 0;
  m.activeRow++;
}

void StimDetector::clearAgvArray()
{
  //ScopedLock resetLock(onlineReset);

  DetectorModule& m = modules.getReference(activeModule);
  m.matrix.resize(1);
  m.matrix[0].clear();
  m.matrix[0].resize(avgLength);
  m.activeRow = 0;

  m.count = 0;

  m.avg.clear();
  m.avg.resize(avgLength);

  m.yAvgMin.clear();
  m.yAvgMin.add(0.0f);
  
  m.yAvgMax.clear();
  m.yAvgMax.add(0.0f);
  
  m.avgSlope.clear();
  m.avgSlope.add(0.0f);
  
  m.avgLatency.clear();
  m.avgLatency.add(0.0f);

}

void StimDetector::updateWaveformParams()
{
  for (int m = 0; m < modules.size(); ++m)
  {
    DetectorModule& dm = modules.getReference(m);

    dm.stimMean = dm.stim; //Array
    dm.xMin = 0;
    dm.yMin = 0;
    int tMin = 0;

    //suavisar a curva
    for (int t = movMean; t < (avgLength - movMean); t++)
    {
      double aux_mean = 0;
      for (int tt = -(movMean / 2); tt < ((movMean / 2)); tt++)
      {
        aux_mean = aux_mean + dm.stim[(t + tt)];
        //std::cout << (tt) << std::endl;
      }

      dm.stimMean.set(t, (aux_mean / (movMean)));
      //std::cout << dm.stimMean[t] << std::endl;
    }

    //std::cout << dm.stimMean << ", " << dm.stim[tMin - 2] << ", " << dm.stim[tMin - 1] << std::endl;

    //MIN
    for (int t = 0; t < avgLength; t++)
    {
      if (dm.stimMean[t] < dm.yMin && t >= ttlLength)
      {
        dm.xMin = dm.timestamps[t];
        dm.yMin = dm.stim[t];
        tMin = t; //ref
      }
    }
    //std::cout << dm.yMin << ", " << dm.xMin << std::endl;


    //MAX
    dm.xMax = dm.xMin;
    dm.yMax = dm.yMin;
    for (int tMax = tMin; dm.stimMean[tMax - 1] > dm.stimMean[tMax]; tMax--)
    {
      dm.xMax = dm.timestamps[tMax];
      dm.yMax = dm.stim[tMax];
      // std::cout << yMax << ", " << dm.stim[yMax - 1] << ", " << dm.stim[yMax] << std::endl;
    }
    //std::cout << yMax << ", " << dm.stim[yMax - 2] << ", " << dm.stim[yMax - 1] << std::endl;
  }
}

void StimDetector::updateActiveAvgLineParams()
{
  for (int m = 0; m < modules.size(); ++m)
  {
    Array<double> last = getLastWaveformParams(m);
    DetectorModule& dm = modules.getReference(m);

    if(dm.count > 0) {
      dm.yAvgMin.set(dm.activeRow, (dm.yAvgMin[dm.activeRow] * ((double)dm.count - 1) + last[0]) / (double)(dm.count));
      dm.yAvgMax.set(dm.activeRow, (dm.yAvgMax[dm.activeRow] * ((double)dm.count - 1) + last[1]) / (double)(dm.count));

      dm.avgLatency.set(dm.activeRow, (dm.avgLatency[dm.activeRow] * ((double)dm.count - 1) + last[3]) / (double)(dm.count));
      dm.avgSlope.set(dm.activeRow, (dm.avgSlope[dm.activeRow] * ((double)dm.count - 1) + last[4]) / (double)(dm.count));
    }
  }
}

int StimDetector::getActiveModule() {
  return activeModule;
}

double StimDetector::getThresholdValueForActiveModule()
{
  DetectorModule& module = modules.getReference(activeModule);
  return module.threshold;
}

Array<double> StimDetector::getLastWaveformParams(int module=-1)
{
  DetectorModule& dm = modules.getReference(module==-1 ? activeModule : module);

  double slope = dm.xMax - dm.xMin == 0 ? 0
    : (((dm.yMax - dm.yMin) / abs(dm.xMax - dm.xMin)) / (double)(getDataChannel(dm.inputChan)->getSampleRate()));

  double latency = dm.count == 0 ? 0
    : (double)(ttlLength + dm.xMin - dm.timestamps[0]) / (double)(getDataChannel(dm.inputChan)->getSampleRate()) * 1000;

  Array<double> moduleParams;
  moduleParams.add(dm.yMin);              //MIN
  moduleParams.add(dm.yMax);              //MAX
  moduleParams.add(dm.yMax - dm.yMin);    //PEAK TO PEAK
  moduleParams.add(latency);              //LATENCY
  moduleParams.add(slope);                //SLOPE
  moduleParams.add(dm.count);             //AVG COUNT

  return moduleParams;
}

Array<Array<double>> StimDetector::getAvgMatrixParams()
{
  DetectorModule& dm = modules.getReference(activeModule);

  for (int r = 0; r <= dm.activeRow; r++)
  {

    Array<double> rowParams;
    rowParams.add(dm.yAvgMin[r]);                   //MIN
    rowParams.add(dm.yAvgMax[r]);                   //MAX
    rowParams.add(dm.yAvgMax[r] - dm.yAvgMin[r]);   //PEAK TO PEAK
    rowParams.add(dm.avgLatency[r]);               //LATENCY
    rowParams.add(dm.avgSlope[r]);                 //SLOPE
    rowParams.add(dm.count);                        //AVG COUNT

    dm.matrix.set(dm.activeRow,rowParams);          //row of table
  }

  return dm.matrix;
}
