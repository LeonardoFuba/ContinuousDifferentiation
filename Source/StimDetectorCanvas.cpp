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

#include "StimDetectorCanvas.h"

using namespace StimDetectorSpace;

StimDetectorCanvas::StimDetectorCanvas(StimDetector* sd) :
  processor(sd),
  viewport(new Viewport()),
  canvas(new Component("canvas")),
  canvasBounds	(0, 0, 990, 200)
{
  refreshRate = 2; //Hz
  juce::Rectangle<int> bounds;
  //canvasBounds = canvasBounds.getUnion(bounds);

  std::cout << "class.canvas start" << std::endl;

  /* Colors */
  colours.add(Colour(224, 185, 36));
  colours.add(Colour(214, 210, 182));
  colours.add(Colour(243, 119, 33));
  colours.add(Colour(186, 157, 168));
  colours.add(Colour(237, 37, 36));
  colours.add(Colour(179, 122, 79));
  colours.add(Colour(217, 46, 171));
  colours.add(Colour(217, 139, 196));
  colours.add(Colour(101, 31, 255));
  colours.add(Colour(141, 111, 181));
  colours.add(Colour(48, 117, 255));
  colours.add(Colour(184, 198, 224));
  colours.add(Colour(116, 227, 156));
  colours.add(Colour(150, 158, 155));
  colours.add(Colour(82, 173, 0));
  colours.add(Colour(125, 99, 32));

  font = Font("Small Text", 15, Font::plain);


  resetButton = new UtilityButton("Clear AVG", font);
  resetButton->addListener(this);
  addAndMakeVisible(resetButton);

  splitButton = new UtilityButton("Split AVG", font);
  splitButton->addListener(this);
  addAndMakeVisible(splitButton);

  //addAndMakeVisible(viewport);

  //stimDisplay = new StimDetectorDisplay(sd, this, viewport);
  //viewport->setViewedComponent(stimDisplay, false);

  flipCanvas();
  startCallbacks();
}

StimDetectorCanvas::~StimDetectorCanvas()
{
  stopCallbacks();
  //processor->removeStimPlots();
}

void StimDetectorCanvas::flipCanvas()
{
  std::cout << "class.canvas flip" << std::endl;
  canvasBounds.setBottom(canvasBounds.getBottom() + 10);
	canvas->setBounds(canvasBounds);

	viewport->setViewedComponent(canvas, false);
	viewport->setScrollBarsShown(true, true);

	addAndMakeVisible(viewport);
}

void StimDetectorCanvas::paint(Graphics& g)
{
  //std::cout << "class.canvas paint" << std::endl;
  g.fillAll(Colours::black); //background

  g.setColour(Colour(0, 18, 43));
  g.fillRoundedRectangle(2, PADDING_TOP + SCALE_WIDTH + 2, getWidth() - 4, getHeight() - 4 - (PADDING_TOP + SCALE_WIDTH), 6.0f); //graph background

  g.setColour(Colours::grey); //buttons separator
  g.fillRect(0, PADDING_TOP - 1, getWidth(), 1);


  int cols = last.size();       //6
  int rows = avgMatrix.size();  //avg lines

  g.setFont(font);

  // table header lines
  for (int x = 0; x < cols; x++)
  {
    g.setColour(Colours::grey);
    g.drawRect(150 + 130 * x, PADDING_TOP + 70, 130, 30, 1);
  }
  // table header text
  g.setColour(Colours::white);
  g.drawText("MIN"          , 150, PADDING_TOP + 70, 130, 30, Justification::centred, true);
  g.drawText("MAX"          , 280, PADDING_TOP + 70, 130, 30, Justification::centred, true);
  g.drawText("PEAK TO PEAK" , 410, PADDING_TOP + 70, 130, 30, Justification::centred, true);
  g.drawText("LATENCY"      , 540, PADDING_TOP + 70, 130, 30, Justification::centred, true);
  g.drawText("SLOPE"        , 670, PADDING_TOP + 70, 130, 30, Justification::centred, true);
  g.drawText("COUNT"        , 800, PADDING_TOP + 70, 130, 30, Justification::centred, true);

  g.drawText("LAST STIM "   , 50, PADDING_TOP + 100, 100, 30, Justification::centredRight, true);


  // first line of table
  g.setFont(Font("Small Text", 15, Font::bold));
  g.setColour(Colours::grey);
  for (int x = 0; x < cols; x++)
  {
    //last stim line
    g.drawRect(150 + 130 * x, PADDING_TOP + 100, 130, 30, 1);
    g.drawText(String(last[x]), 150 + 130 * x, PADDING_TOP + 100, 130, 30, Justification::centred, true);
  }


  // avg lines of table
  g.setFont(font);
  for (int y = 0; y < rows; y++)
  {
    // avg line labels
    g.setColour(Colours::white);
    g.drawText("AVG " + String(y + 1) + " ", 50, PADDING_TOP + 130 + 30 * y, 100, 30, Justification::centredRight, true);
    for (int x = 0; x < cols; x++)
    {
      //avg lines values
      g.setColour(Colours::grey);
      g.drawRect(150 + 130 * x, PADDING_TOP + 130 + 30 * y, 130, 30, 1);
      g.setColour(colours[y]);
      g.drawText(String(avgMatrix[y][x]), 150 + 130 * x, PADDING_TOP + 130 + 30 * y, 130, 30, Justification::centred, true);
    }
  }







  /*
  // Find pixel positions for start/end and top/bottom based on voltage data and length
  double xPos = 250;
  int xEnd = 1000;
  int yPosMid = (channelYStart + (chan + 1) * (channelYJump)) - channelYJump / 2;

  double step = (xEnd - xPos) / double(avgLFP[trigIndex][chan].size());

  for (int samp = 0; samp < avgLFP[trigIndex][chan].size(); samp++)
  {
    // Turn this into looping through ttl channels
    g.setColour(colorList[chan]);
    double ratio = (avgLFP[trigIndex][chan][samp] - midPoint) / totalY;
    double yPos = yPosMid + channelYJump * ratio;
    g.fillRect(xPos, yPos, 1, 1); // Draw pixel rects of data.. Maybe not the best way!
    xPos += step;
  }
  */

}

void StimDetectorCanvas::refreshState()
{
  std::cout << "class.canvas refreshState" << std::endl;
  // called when the component's tab becomes visible again
  resized();
}

void StimDetectorCanvas::resized()
{
  std::cout << "class.canvas resized" << std::endl;
  // called when the modify canvas dimensions
  viewport->setBounds(0, 50, getWidth(), getHeight() - 50); // leave space at top for buttons
  resetButton->setBounds(10, 10, 120, 30);
  splitButton->setBounds(140, 10, 120, 30);
}

void StimDetectorCanvas::update()
{
  std::cout << "class.canvas update" << std::endl;
  // called when canvas is loading
  resized();
  repaint();
}

void StimDetectorCanvas::refresh()
{
  // std::cout << "refresh canvas -> ";
  // called continuosly

  // -- Title -- //
  title = createLabel("Title", "STIM PARAMETERS", Justification::centred, { 5, 5, canvas->getWidth(), 50 });

  //processor data  
  last = processor->getLastWaveformParams();
  avgMatrix = processor->getAvgMatrixParams();
  //std::cout << "avgMatrix: " << avgMatrix.size() << ", " << avgMatrix[0].size() << std::endl;

  repaint(); //update graphics

}

void StimDetectorCanvas::beginAnimation()
{
  std::cout << "StimDetectorCanvas beginning animation." << std::endl;

  startCallbacks();
}

void StimDetectorCanvas::endAnimation()
{
  std::cout << "StimDetectorCanvas ending animation." << std::endl;

  stopCallbacks();
}

void StimDetectorCanvas::saveVisualizerParameters(XmlElement* xml) {}
void StimDetectorCanvas::loadVisualizerParameters(XmlElement* xml) {}

void StimDetectorCanvas::buttonClicked(Button* button)
{
  if (button == resetButton)
  {
    // Clears avg vector to start from scratch.
     processor->clearAgvArray();
    // update();
  }
  else if(button == splitButton)
  {
    // Add new line and restart new avg calc

  }
}

Label* StimDetectorCanvas::createLabel(const String& name, const String& text, const Justification& justification, juce::Rectangle<int> bounds)
{
  Label* label = new Label(name, text);
  label->setBounds(bounds);
  label->setFont(Font("Chan Labels", 24, Font::bold));
  label->setColour(Label::textColourId, Colours::white);
  label->setJustificationType(justification);
  canvas->addAndMakeVisible(label);
  canvasBounds = canvasBounds.getUnion(bounds);
  return label;
}

// ===================================================================
/**
DetectorDisplay::DetectorDisplay(StimDetector* sd, StimDetectorCanvas* sdc, Viewport* v) :
  processor(sd),
  canvas(sdc),
  viewport(v)
{

}

DetectorDisplay::~StimDetectorDisplay()
{

}

void DetectorDisplay::paint(Graphics& g)
{
  g.fillAll(Colours::grey);
}
*/