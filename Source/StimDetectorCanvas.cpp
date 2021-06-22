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
  canvasBounds	(0, 0, 1, 1)
{
  refreshRate = 2; //Hz
  juce::Rectangle<int> bounds;

  // -- Title -- //
  title = createLabel("Title", "Hello World!!", { 5, 0, 280, 50 });



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
  canvasBounds.setBottom(canvasBounds.getBottom() + 10);
	//canvasBounds.setRight(1000);
	canvas->setBounds(canvasBounds);

	viewport->setViewedComponent(canvas, false);
	viewport->setScrollBarsShown(false, true);
	addAndMakeVisible(viewport);
}

Label* StimDetectorCanvas::createLabel(const String& name, const String& text,	juce::Rectangle<int> bounds)
{
	Label* label = new Label(name, text);
	label->setBounds(bounds);
	label->setFont(Font("Chan Labels", 20, Font::bold));
	label->setColour(Label::textColourId, Colours::white);
	canvas->addAndMakeVisible(label);
	canvasBounds = canvasBounds.getUnion(bounds);
	return label;
}

void StimDetectorCanvas::paint(Graphics& g)
{

  g.fillAll(Colours::black);

}

void StimDetectorCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    resized();
}

void StimDetectorCanvas::resized()
{
    viewport->setBounds(0, 0, getWidth(), getHeight() - 30); // leave space at bottom for buttons
}

void StimDetectorCanvas::update()
{

}

void StimDetectorCanvas::refresh()
{

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
