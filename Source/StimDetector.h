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

//#define AVG_LENGTH 487
//#define TTL_LENGTH 10

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

    void splitAvgArray();
    void clearAgvArray();
    
    int getActiveModule();
    double getThresholdValueForActiveModule();
    Array<double> getLastWaveformParams(int module); //paramIndex
    Array<Array<double>> getAvgMatrixParams(); //AvgSection.paramIndex

    

    //void saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelTypel) override;
    //void loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)  override;

  private:
    void handleEvent (const EventChannel* channelInfo, const MidiMessage& event, int sampleNum) override;

    void updateWaveformParams();
    void updateActiveAvgLineParams();

    //enum ModuleType
    //{
    //  NONE, PEAK
    //};

    //enum PhaseType
    //{
    //  NO_PHASE, FALLING_POS
    //};
    struct DetectorModule
    {
      int inputChan;              //electrode input channel
      int gateChan;               //digital input channel
      int outputChan;             //digital output channel
      int samplesSinceTrigger;    //ttl interval count

      float lastSample;           //last input original data
      float lastDiff;             //last input diff data
      
      double threshold;           //threshold of detection

      bool applyDiff;             //overwrite input chan data
      bool isActive;              //channels to display in canvas
      bool wasTriggered;          //ttl interval
      bool wasTriggered_buffer;   //ttl interval inside the entire buffer
      bool startStim;             //stim interval
      bool detectorStim;          //internal ttl detector
      bool ignoreFirst;           //fix first diff value

      int startIndex;             //intput index
      int windowIndex;            //avg index
      int count;                  //avg count
 
      Array<double> stim;         //original stim
      Array<int64> timestamps;    //last stim timestamps
      Array<double> stimMean;     //moving mean array for max and min calculation
      double yMax;                //max of stim
      double yMin;                //min of stim
      int64 xMax;                 //time of max
      int64 xMin;                 //time of min

      Array<double> avg;            //avg of stims
      Array<Array<double>> matrix;  //matrix of avg params 
      int activeRow;                //last row of avg
      Array<double> yAvgMax;        //max of avg stim
      Array<double> yAvgMin;        //min of avg stim
      Array<double> avgLatency;     //latency of avg stim
      Array<double> avgSlope;       //slope of avg stim

      //StimPlot* stimPlot;         //Canvas Component
      //ModuleType type;
      //PhaseType phase;
    };

    Array<DetectorModule> modules;
    int activeModule;
    int lastNumInputs;
    double defaultThreshold;

    int avgLength;
    int ttlLength;
    int movMean;
    int buffetMin;

    //CriticalSection onlineReset;

    Array<const EventChannel*> moduleEventChannels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StimDetector);
  };

}


#endif  // __STIMDETECTOR_H_DEFINED
