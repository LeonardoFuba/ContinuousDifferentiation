#include <stdio.h>
#include <iostream>
#include "Leonardo.h"
#include "LeonardoEditor.h"

using namespace LeonardoSpace;

LeonardoEditor::LeonardoEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)
{
    desiredWidth = 200;

    lastStartTimeString = " ";
    lastEndTimeString = " ";
    lastDiffAmpString = " ";

    startTimeLabel = new Label("start time label", "start time");
    startTimeLabel->setBounds(10,25,80,20);
    startTimeLabel->setFont(Font("Small Text", 12, Font::plain));
    startTimeLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(startTimeLabel);

    startTimeValue = new Label("start time value", lastStartTimeString);
    startTimeValue->setBounds(15,42,60,18);
    startTimeValue->setFont(Font("Default", 15, Font::plain));
    startTimeValue->setColour(Label::textColourId, Colours::white);
    startTimeValue->setColour(Label::backgroundColourId, Colours::grey);
    startTimeValue->setEditable(true);
    startTimeValue->addListener(this);
    startTimeValue->setTooltip("Set the start time for the trigger selected channels");
    addAndMakeVisible(startTimeValue);

    endTimeLabel = new Label("end time label", "end time");
    endTimeLabel->setBounds(10,65,80,20);
    endTimeLabel->setFont(Font("Small Text", 12, Font::plain));
    endTimeLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(endTimeLabel);

    endTimeValue = new Label("end time value", lastEndTimeString);
    endTimeValue->setBounds(15,82,60,18);
    endTimeValue->setFont(Font("Default", 15, Font::plain));
    endTimeValue->setColour(Label::textColourId, Colours::white);
    endTimeValue->setColour(Label::backgroundColourId, Colours::grey);
    endTimeValue->setEditable(true);
    endTimeValue->addListener(this);
    endTimeValue->setTooltip("Set the end time for the trigger selected channels");
    addAndMakeVisible(endTimeValue);

    diffAmpLabel = new Label("diff amp label", "diff amp");
    diffAmpLabel->setBounds(100,25,80,20);
    diffAmpLabel->setFont(Font("Small Text", 12, Font::plain));
    diffAmpLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(diffAmpLabel);

    diffAmpValue = new Label("diff amp value", lastEndTimeString);
    diffAmpValue->setBounds(105,42,60,18);
    diffAmpValue->setFont(Font("Default", 15, Font::plain));
    diffAmpValue->setColour(Label::textColourId, Colours::white);
    diffAmpValue->setColour(Label::backgroundColourId, Colours::grey);
    diffAmpValue->setEditable(true);
    diffAmpValue->addListener(this);
    diffAmpValue->setTooltip("Set the diff amp for the trigged selected channels");
    addAndMakeVisible(diffAmpValue);

    applyDiff = new UtilityButton("Diff",Font("Default", 10, Font::plain));
    applyDiff->addListener(this);
    applyDiff->setBounds(105,70,40,18); //(x,y,w,h)
    applyDiff->setClickingTogglesState(true);
    applyDiff->setTooltip("When this button is off, selected channels will do not show differentiation");
    addAndMakeVisible(applyDiff);

    // applyFilterOnChan = new UtilityButton("Button",Font("Default", 10, Font::plain));
    // applyFilterOnChan->addListener(this);
    // applyFilterOnChan->setBounds(90,95,40,18);
    // applyFilterOnChan->setClickingTogglesState(true);
    // applyFilterOnChan->setToggleState(true, dontSendNotification);
    // applyFilterOnChan->setTooltip("When this button is off, selected channels will do not ...");
    // addAndMakeVisible(applyFilterOnChan);
}

LeonardoEditor::~LeonardoEditor()
{

}

void LeonardoEditor::setDefaults(double startTime, double endTime, double diffAmp)
{
    lastStartTimeString = String(roundFloatToInt(startTime));
    lastEndTimeString = String(roundFloatToInt(endTime));
    lastDiffAmpString = String(roundFloatToInt(diffAmp));

    resetToSavedText();
}

void LeonardoEditor::resetToSavedText()
{
    startTimeValue->setText(lastStartTimeString, dontSendNotification);
    endTimeValue->setText(lastEndTimeString, dontSendNotification);
    diffAmpValue->setText(lastDiffAmpString, dontSendNotification);
}

void LeonardoEditor::labelTextChanged(Label* label)
{
    Leonardo* fn = (Leonardo*) getProcessor();

    Value val = label->getTextValue();
    double requestedValue = double(val.getValue());

    if (requestedValue < 0.01 || requestedValue > 10000)
    {
        CoreServices::sendStatusMessage("Value out of range.");

        if (label == startTimeValue)
        {
            label->setText(lastStartTimeString, dontSendNotification);
            lastStartTimeString = label->getText();
        }
        else if (label == endTimeValue)
        {
            label->setText(lastEndTimeString, dontSendNotification);
            lastEndTimeString = label->getText();
        }
        else
        {
            label->setText(lastDiffAmpString, dontSendNotification);
            lastDiffAmpString = label->getText();
            std::cout << lastDiffAmpString << std::endl;
        }
        

        return;
    }

    Array<int> chans = getActiveChannels();

    // This needs to change, since there's not enough feedback about whether
    // or not individual channel settings were altered:

    for (int n = 0; n < chans.size(); n++)
    {

        if (label == startTimeValue)
        {
            double minVal = fn->getStartTimeValueForChannel(chans[n]);

            if (requestedValue > minVal)
            {
                fn->setCurrentChannel(chans[n]);
                fn->setParameter(1, requestedValue);
            }

            lastStartTimeString = label->getText();

        }
        else if (label == endTimeValue)
        {
            double maxVal = fn->getEndTimeValueForChannel(chans[n]);

            if (requestedValue < maxVal)
            {
                fn->setCurrentChannel(chans[n]);
                fn->setParameter(0, requestedValue);
            }

            lastEndTimeString = label->getText();
        }
        else
        {
            double maxVal = fn->getDiffScaleValueForChannel(chans[n]);

            if (requestedValue < maxVal)
            {
                fn->setCurrentChannel(chans[n]);
                fn->setParameter(0, requestedValue);
            }

            lastDiffAmpString = label->getText();
        }
        

    }

}

void LeonardoEditor::channelChanged (int channel, bool /*newState*/)
{
    Leonardo* fn = (Leonardo*) getProcessor();

    startTimeValue->setText (String (fn->getStartTimeValueForChannel (channel)), dontSendNotification);
    endTimeValue->setText  (String (fn->getEndTimeValueForChannel  (channel)), dontSendNotification);
    diffAmpValue->setText(String(fn->getDiffScaleValueForChannel(channel)), dontSendNotification);
    // applyFilterOnChan->setToggleState (fn->getBypassStatusForChannel (channel), dontSendNotification);

}

void LeonardoEditor::buttonEvent(Button* button)
{

    if (button == applyDiff)
    {
        Leonardo* fn = (Leonardo*) getProcessor();
        fn->setDiff(applyDiff->getToggleState());

    }
    // else if (button == name)
    // {
        
    // }
}

void LeonardoEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "LeonardoEditor");

    lastStartTimeString = startTimeValue->getText();
    lastEndTimeString = endTimeValue->getText();
    lastDiffAmpString = diffAmpValue->getText();

    XmlElement* textLabelValues = xml->createNewChildElement("VALUES");
    textLabelValues->setAttribute("startTime",lastStartTimeString);
    textLabelValues->setAttribute("endTime",lastEndTimeString);
    textLabelValues->setAttribute("diffScale", lastDiffAmpString);
    textLabelValues->setAttribute("applyDiff", applyDiff->getToggleState());
}

void LeonardoEditor::loadCustomParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("VALUES"))
        {
            lastStartTimeString = xmlNode->getStringAttribute("startTime", lastStartTimeString);
            lastEndTimeString = xmlNode->getStringAttribute("endTime", lastEndTimeString);
            lastDiffAmpString = xmlNode->getStringAttribute("diffScale", lastDiffAmpString);
            resetToSavedText();

            applyDiff->setToggleState(xmlNode->getBoolAttribute("applyDiff", false), sendNotification);
        }
    }

}
