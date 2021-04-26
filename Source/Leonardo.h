//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef LEONARDO_H_DEFINED
#define LEONARDO_H_DEFINED

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <ProcessorHeaders.h>

//namespace must be an unique name for your plugin
namespace LeonardoSpace
{
	class Leonardo : public GenericProcessor
	{
	public:
		/** The class constructor, used to initialize any members. */
		Leonardo();

		/** The class destructor, used to deallocate memory */
		~Leonardo();

		/** Indicates if the processor has a custom editor. Defaults to false */
		bool hasEditor() const override { return true; }

		/** If the processor has a custom editor, this method must be defined to instantiate it. */
		AudioProcessorEditor* createEditor() override;

		/** Optional method that informs the GUI if the processor is ready to function. If false acquisition cannot start. Defaults to true */
		//bool isReady();

		/** Defines the functionality of the processor.

		The process method is called every time a new data buffer is available.

		Processors can either use this method to add new data, manipulate existing
		data, or send data to an external target (such as a display or other hardware).

		Continuous signals arrive in the "buffer" variable, event data (such as TTLs
		and spikes) is contained in the "events" variable.
		*/
		void process(AudioSampleBuffer& buffer) override;

				/** Handles events received by the processor

		Called automatically for each received event whenever checkForEvents() is called from process()		
		*/
		//void handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition) override;

		/** Handles spikes received by the processor

		Called automatically for each received event whenever checkForEvents(true) is called from process()
		*/
		//void handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition) override;

		/** The method that standard controls on the editor will call.
		It is recommended that any variables used by the "process" function
		are modified only through this method while data acquisition is active. */
		void setParameter(int parameterIndex, float newValue) override;

		/** Saving custom settings to XML. */
		void saveCustomChannelParametersToXml(XmlElement* channelInfo, int channelNumber, InfoObjectCommon::InfoObjectType channelTypel) override;
		
		/** Load custom settings from XML*/
		void loadCustomChannelParametersFromXml(XmlElement* channelInfo, InfoObjectCommon::InfoObjectType channelType)  override;

	/** Optional method called every time the signal chain is refreshed or changed in any way.

		Allows the processor to handle variations in the channel configuration or any other parameter
		passed down the signal chain. The processor can also modify here the settings structure, which contains
		information regarding the input and output channels as well as other signal related parameters. Said
		structure shouldn't be manipulated outside of this method.

		*/
		void updateSettings() override;

		double getStartTimeValueForChannel  (int chan) const;
		double getEndTimeValueForChannel (int chan) const;
		double getDiffScaleValueForChannel(int chan) const;
		

		void setDiff(bool state);
		bool applyDiff;

	private:

		Array<double> startTime; //1 por ch
		Array<double> endTime;	//1 por ch
		Array<double> diffScale; //1 por ch

		double defaultStartTime;
		double defaultEndTime;
		double defaultDiffScale;

		Array<bool> shouldDiffChannel;
		
		int time;
		void setTime(int t);
		int getTime();

		// void setFilterParameters (double, double, double, int);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Leonardo);
	};
}

#endif