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

#ifndef __STIMDETECTOR_H_DEFINED
#define __STIMDETECTOR_H_DEFINED

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <ProcessorHeaders.h>

#define NUM_INTERVALS 5
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

        double getThresholdValueForChannel(int chan);

        void saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelTypel) override;
        void loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)  override;

        void setDiff(bool state);
        bool applyDiff;

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

            bool isActive;
            bool wasTriggered;          //ttl interval
            bool startStim;             //stim interval
            bool detectorStim;          //internal ttl detector

            int startIndex;             //intput index
            int windowIndex;            //avg index
            int count;                  //avg count

            //double threshold;
            //double avg[AVG_LENGTH];
            ModuleType type;
            PhaseType phase;
        };

        Array<DetectorModule> modules;
        int activeModule;
        int lastNumInputs;

        Array<bool> shouldDiffChannel;
        Array<double> thresholds;
        double defaultThreshold;

        int count;
        double media[AVG_LENGTH];

        Array<const EventChannel*> moduleEventChannels;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StimDetector);
    };

}


#endif  // __PHASEDETECTOR_H_DEFINED
