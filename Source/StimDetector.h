/*
  ------------------------------------------------------------------

  This file is part of the Open Ephys GUI
  Copyright (C) 2013 Open Ephys

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

//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef __STIMDETECTOR_H_DEFINED
#define __STIMDETECTOR_H_DEFINED

#ifdef _WIN32
  #include <Windows.h>
#endif

#include <ProcessorHeaders.h>

#define AVG_LENGTH 400

namespace StimDetectorSpace {

  /**

    Uses peaks to estimate the phase of a continuous signal.

    @see GenericProcessor, StimDetectorEditor
  */
  class StimDetector : public GenericProcessor
  {
  public:
    StimDetector();
    ~StimDetector();

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    void addModule();
    void setActiveModule (int);
    void setParameter (int parameterIndex, float newValue) override;
    void updateSettings() override;
    bool enable() override;
    void process (AudioSampleBuffer& buffer) override;

    double getThresholdValueForActiveModule();
    Array<Array<double>> getWaveformParams(); //moduleIndex.paramIndex

    //void saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelTypel) override;
    //void loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)  override;

  private:
    void handleEvent (const EventChannel* channelInfo, const MidiMessage& event, int sampleNum) override;

    enum ModuleType
    {
      NONE, PEAK
    };

    enum PhaseType
    {
      NO_PHASE, FALLING_POS
    };
    struct DetectorModule
    {
      int inputChan;              //electrode input channel
      int gateChan;               //digital input channel
      int outputChan;             //digital output channel
      int samplesSinceTrigger;    //ttl interval count

      float lastSample;           //last input original data
      float lastDiff;             //last input diff data

      bool applyDiff;             //overwrite input chan data
      bool isActive;              //channels to display in canvas
      bool wasTriggered;          //ttl interval
      bool startStim;             //stim interval
      bool detectorStim;          //internal ttl detector

      int startIndex;             //intput index
      int windowIndex;            //avg index
      int count;                  //avg count

      double threshold;           //threshold of detection

      double max;                 //max of stim
      double min;                 //min of stim
      //double avg[AVG_LENGTH];
      ModuleType type;
      PhaseType phase;
    };

    Array<DetectorModule> modules;
    int activeModule;
    int lastNumInputs;
    double defaultThreshold;

    double media[AVG_LENGTH];

    Array<const EventChannel*> moduleEventChannels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StimDetector);
  };

}


#endif  // __PHASEDETECTOR_H_DEFINED
