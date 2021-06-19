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


#include "StimDetectorEditor.h"
#include "StimDetector.h"

#include <stdio.h>
#include <cmath>
#include <iostream>

using namespace StimDetectorSpace;

StimDetectorEditor::StimDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)
    , previousChannelCount(-1)

{
    desiredWidth = 320;
    lastThresholdString = " ";

    std::cout << "Creating buttons" << std::endl;

    plusButton = new UtilityButton("+", titleFont);
    plusButton->addListener(this);
    plusButton->setRadius(3.0f);
    plusButton->setBounds(10, 30, 20, 20);
    addAndMakeVisible(plusButton);

    detectorSelector = new ComboBox();
    detectorSelector->setBounds(35,30,165,20);
    detectorSelector->addListener(this);
    addAndMakeVisible(detectorSelector);


    std::cout << "Creating inputs" << std::endl;

    thresholdLabel = new Label("threshold label", "Threshold (uV)");
    thresholdLabel->setBounds(210, 35, 100, 20);
    thresholdLabel->setFont(Font("Small Text", 12, Font::plain));
    thresholdLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(thresholdLabel);

    thresholdValue = new Label("threshold value", lastThresholdString);
    thresholdValue->setBounds(215, 52, 60, 18);
    thresholdValue->setFont(Font("Default", 15, Font::plain));
    thresholdValue->setColour(Label::textColourId, Colours::white);
    thresholdValue->setColour(Label::backgroundColourId, Colours::grey);
    thresholdValue->setEditable(true);
    thresholdValue->addListener(this);
    thresholdValue->setTooltip("Set the threshold of detection");
    addAndMakeVisible(thresholdValue);

    applyDiff = new UtilityButton("Diff", Font("Default", 10, Font::plain));
    applyDiff->addListener(this);
    applyDiff->setBounds(215, 80, 60, 18);
    applyDiff->setClickingTogglesState(true);
    applyDiff->setTooltip("When this button is off, selected channels will do not show differentiation");
    addAndMakeVisible(applyDiff);


    backgroundColours.add(Colours::red);
    //plusButton->setToggleState(true, sendNotification);
	addDetector();

    //interfaces.clear();

}

StimDetectorEditor::~StimDetectorEditor()
{
}

void StimDetectorEditor::setDefaults( double threshold)
{
    lastThresholdString = String(roundFloatToInt(threshold));

    resetToSavedText();
}

void StimDetectorEditor::resetToSavedText()
{
    thresholdValue->setText(lastThresholdString, dontSendNotification);
}

void StimDetectorEditor::startAcquisition()
{
	plusButton->setEnabled(false);
	for (int i = 0; i < interfaces.size(); i++)
		interfaces[i]->setEnableStatus(false);
}

void StimDetectorEditor::stopAcquisition()
{
	plusButton->setEnabled(true);
	for (int i = 0; i < interfaces.size(); i++)
		interfaces[i]->setEnableStatus(true);
}

void StimDetectorEditor::labelTextChanged(Label* label)
{
    StimDetector* fn = (StimDetector*) getProcessor();

    Value val = label->getTextValue();
    double requestedValue = double(val.getValue());

    std::cout << "threshold=" << requestedValue << std::endl;

    if (requestedValue < 0.01 || requestedValue > 10000)
    {
        CoreServices::sendStatusMessage("Value out of range.");

        if (label == thresholdValue) {

            label->setText(lastThresholdString, dontSendNotification);
            lastThresholdString = label->getText();
        }
        return;
    }

    Array<int> chans = getActiveChannels();

    for (int n = 0; n < chans.size(); n++)
    {

        if (label == thresholdValue)
        {
            double val = fn->getThresholdValueForChannel(chans[n]);

            if (requestedValue != val)
            {
                fn->setCurrentChannel(chans[n]);
                fn->setParameter(5, requestedValue);
            }

            lastThresholdString = label->getText();
        }
    }
}

void StimDetectorEditor::updateSettings()
{

    if (getProcessor()->getNumInputs() != previousChannelCount)
    {

        for (int i = 0; i < interfaces.size(); i++)
        {
            interfaces[i]->updateChannels(getProcessor()->getNumInputs());
        }
    }

    previousChannelCount = getProcessor()->getNumInputs();

}

void StimDetectorEditor::comboBoxChanged(ComboBox* c)
{

    for (int i = 0; i < interfaces.size(); i++)
    {

        if (i == c->getSelectedId()-1)
        {
            interfaces[i]->setVisible(true);
        }
        else
        {
            interfaces[i]->setVisible(false);
        }

    }

}

void StimDetectorEditor::buttonEvent(Button* button)
{
    if (button == plusButton && interfaces.size() < 8)
    {
        addDetector();
		CoreServices::updateSignalChain(this);
    }

    else if (button == applyDiff)
    {
        StimDetector* fn = (StimDetector*)getProcessor();
        fn->setDiff(applyDiff->getToggleState());
    }
    else
    {

    }
}

void StimDetectorEditor::addDetector()
{
    std::cout << "Adding detector" << std::endl;

    StimDetector* sd = (StimDetector*) getProcessor();

    int detectorNumber = interfaces.size()+1;

    DetectorInterface* di = new DetectorInterface(sd, backgroundColours[detectorNumber % 5], detectorNumber-1);
    di->setBounds(10,50,190,80); // module position and size

    addAndMakeVisible(di);

    interfaces.add(di);

    String itemName = "Detector ";
    itemName += detectorNumber;
    detectorSelector->addItem(itemName, detectorNumber);
    detectorSelector->setSelectedId(detectorNumber, dontSendNotification);

    for (int i = 0; i < interfaces.size()-1; i++)
    {
        interfaces[i]->setVisible(false);
    }

}

void StimDetectorEditor::channelChanged (int channel, bool /*newState*/)
{
    StimDetector* fn = (StimDetector*) getProcessor();
    thresholdValue->setText (String (fn->getThresholdValueForChannel (channel)), dontSendNotification);
}

void StimDetectorEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "StimDetectorEditor");

    lastThresholdString = thresholdValue->getText();

    for (int i = 0; i < interfaces.size(); i++)
    {
        XmlElement* d = xml->createNewChildElement("STIMDETECTOR");
        d->setAttribute("INPUT",interfaces[i]->getInputChan());
        d->setAttribute("OUTPUT",interfaces[i]->getOutputChan());
        d->setAttribute("THRESHOLD", lastThresholdString);
    }
}

void StimDetectorEditor::loadCustomParameters(XmlElement* xml)
{

    int i = 0;

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("STIMDETECTOR"))
        {

            if (i > 0)
            {
                addDetector();
            }
            interfaces[i]->setInputChan(xmlNode->getIntAttribute("INPUT"));
            interfaces[i]->setOutputChan(xmlNode->getIntAttribute("OUTPUT"));
            lastThresholdString = xmlNode->getStringAttribute("THRESHOLD", lastThresholdString);
            applyDiff->setToggleState(xmlNode->getBoolAttribute("applyDiff", false), sendNotification);
            i++;
        }
    }
}


// ===================================================================

DetectorInterface::DetectorInterface(StimDetector* pd, Colour c, int id) :
    backgroundColour(c), idNum(id), processor(pd)
{

    // lastThresholdString = " ";

    font = Font("Small Text", 10, Font::plain);

    std::cout << "Creating combo boxes" << std::endl;

    inputSelector = new ComboBox();
    inputSelector->setBounds(140,5,50,20);
    inputSelector->addItem("-",1);
    inputSelector->setSelectedId(1, dontSendNotification);
    inputSelector->addListener(this);
    addAndMakeVisible(inputSelector);

    gateSelector = new ComboBox();
    gateSelector->setBounds(140, 30, 50, 20);
    gateSelector->addItem("-", 1);
    gateSelector->addListener(this);
    for (int i = 1; i < 9; i++)
    {
        gateSelector->addItem(String(i), i + 1);
    }
    gateSelector->setSelectedId(1);
    addAndMakeVisible(gateSelector);

    outputSelector = new ComboBox();
    outputSelector->setBounds(140,55,50,20);
    outputSelector->addItem("-",1);
    outputSelector->addListener(this);
    
    for (int i = 1; i < 9; i++)
    {
        outputSelector->addItem(String(i),i+1);
    }
    outputSelector->setSelectedId(1);
    addAndMakeVisible(outputSelector);


    std::cout << "Updating channels" << std::endl;

    updateChannels(processor->getNumInputs());

    std::cout << "Updating processor" << std::endl;


    processor->addModule();

}

DetectorInterface::~DetectorInterface()
{

}

void DetectorInterface::comboBoxChanged(ComboBox* c)
{

    processor->setActiveModule(idNum);

    int parameterIndex = 0;

    if (c == inputSelector)
    {
        parameterIndex = 2;
    }
    else if (c == outputSelector)
    {
        parameterIndex = 3;
    }
    else if (c == gateSelector)
    {
        parameterIndex = 4;
    }
    else {
        
    }

    processor->setParameter(parameterIndex, (float) c->getSelectedId() - 2);
	if (c == inputSelector)
	{
		CoreServices::updateSignalChain(processor->getEditor());
	}
}

//void DetectorInterface::buttonClicked(Button* b)
//{
    /* processor->setActiveModule(idNum); */
//}

void DetectorInterface::updateChannels(int numChannels)
{

    inputSelector->clear();

    inputSelector->addItem("-", 1);

    if (numChannels > 2048) // channel count hasn't been updated yet
    {
        return;
    }

    for (int i = 0; i < numChannels; i++)
    {
        inputSelector->addItem(String(i+1), i+2);

    }

    inputSelector->setSelectedId(1, dontSendNotification);
}

void DetectorInterface::paint(Graphics& g)
{
    g.setColour(Colours::darkgrey);
    g.setFont(font);
    g.drawText("INPUT",50,10,85,10,Justification::right, true);
    g.drawText("GATE",50,35,85,10,Justification::right, true);
    g.drawText("OUTPUT",50,60,85,10,Justification::right, true);

}

void DetectorInterface::setInputChan(int chan)
{
    inputSelector->setSelectedId(chan+2);

    processor->setParameter(2, (float) chan);
}

void DetectorInterface::setOutputChan(int chan)
{
    outputSelector->setSelectedId(chan+2);

    processor->setParameter(3, (float) chan);
}

void DetectorInterface::setGateChan(int chan)
{
    gateSelector->setSelectedId(chan + 2);

    processor->setParameter(4, (float)chan);
}

int DetectorInterface::getInputChan()
{
    return inputSelector->getSelectedId()-2;
}

int DetectorInterface::getOutputChan()
{
    return outputSelector->getSelectedId()-2;
}

int DetectorInterface::getGateChan()
{
    return gateSelector->getSelectedId() - 2;
}

void DetectorInterface::setEnableStatus(bool status)
{
	inputSelector->setEnabled(status);
}
