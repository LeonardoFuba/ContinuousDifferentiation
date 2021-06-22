
/*
  ------------------------------------------------------------------

  This file is part of the Open Ephys GUI
  Copyright (C) 2021 Open Ephys

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

#ifndef STIMDETECTORCANVAS_H_DEFINED
#define STIMDETECTORCANVAS_H_DEFINED

#ifdef _WIN32
#include <Windows.h>
#endif

#include <VisualizerWindowHeaders.h>
#include "../../Source/Processors/Visualization/MatlabLikePlot.h"
#include "StimDetector.h"
#include <vector>

namespace StimDetectorSpace {

  /**
  Displays stim waveforms.
  @see StimDetector, StimDetectorEditor, Visualizer
  */

  class StimDetectorCanvas : 
    public Visualizer
    //public Button::Listener,
    //public ComboBox::Listener,
    //public Label::Listener

  {
    public:
      StimDetectorCanvas(StimDetector* sd);
      ~StimDetectorCanvas();
      void refreshState();
      void resized();
      void update();
      void refresh();
      void beginAnimation();
      void endAnimation();
      void paint(Graphics& g);

      void saveVisualizerParameters(XmlElement* xml);
      void loadVisualizerParameters(XmlElement* xml);

      void setParameter(int, float) {}
      void setParameter(int, int, int, float) {}

    private:
      StimDetector* processor;

      void flipCanvas();
      Label* createLabel(const String& name, const String& text, juce::Rectangle<int> bounds);

      /* Window */
      ScopedPointer<Viewport> viewport;
      ScopedPointer<Component> canvas;
      juce::Rectangle<int> canvasBounds;
      //int scrollBarThickness;

      /* Window content */
      ScopedPointer<Label> title;

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StimDetectorCanvas);
  };
}

#endif  // STIMDETECTORCANVAS_H_DEFINED