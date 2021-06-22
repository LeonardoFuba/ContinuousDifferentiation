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


#ifndef __STIMDETECTOREDITOR_H_DEFINED
#define __STIMDETECTOREDITOR_H_DEFINED


#ifdef _WIN32
#include <Windows.h>
#endif

#include <VisualizerEditorHeaders.h>


namespace StimDetectorSpace {

  class DetectorInterface;
  class StimDetector;
  // class FilterViewport;

/**

  User interface for the StimDetector processor.

  @see StimDetector

*/

  class StimDetectorEditor : public VisualizerEditor,
    public ComboBox::Listener,
    public Label::Listener
  {
  public:
    StimDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    ~StimDetectorEditor();

    Visualizer* createNewCanvas() override;

    void buttonEvent(Button* button);
    void comboBoxChanged(ComboBox* c);
  
    void labelTextChanged(Label* label);
    void channelChanged(int chan, bool /*newState*/);

    void updateSettings();

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);

    void startAcquisition() override;
    void stopAcquisition() override;

  private:

    ScopedPointer<ComboBox> detectorSelector;
    ScopedPointer<UtilityButton> plusButton;

    void addDetector();

    // ScopedPointer<ComboBox> inputChannelSelectionBox;
    // ScopedPointer<ComboBox> outputChannelSelectionBox;

    // ScopedPointer<Label> intputChannelLabel;
    // ScopedPointer<Label> outputChannelLabel;

    OwnedArray<DetectorInterface> interfaces;

    int previousChannelCount;

    Array<Colour> backgroundColours;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StimDetectorEditor);

  };

  class DetectorInterface : public Component,
    public Button::Listener,
    public ComboBox::Listener,
    public Label::Listener
  {
  public:
    DetectorInterface(StimDetector*, Colour, int);
    ~DetectorInterface();

    void paint(Graphics& g);

    void comboBoxChanged(ComboBox*);
    //void buttonEvent(Button*);
    void buttonClicked(Button*);
    void labelTextChanged(Label*);

    void updateChannels(int);

    void setInputChan(int);
    void setOutputChan(int);
    void setGateChan(int);
    void setThreshold(double);

    int getInputChan();
    int getOutputChan();
    int getGateChan();
    double getThreshold();

    void setEnableStatus(bool status);

  private:
    StimDetector* processor;
  
    Colour backgroundColour;
    Font font;

    int idNum;

    String lastThresholdString;

    ScopedPointer<ComboBox> inputSelector;
    ScopedPointer<ComboBox> gateSelector;
    ScopedPointer<ComboBox> outputSelector;

    ScopedPointer<UtilityButton> applyDiff;

    ScopedPointer<Label> thresholdLabel;
    ScopedPointer<Label> thresholdValue;
  };

}

#endif  // __PHASEDETECTOREDITOR_H_DEFINED
