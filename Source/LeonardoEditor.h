#ifndef LEONARDOEDITOR_H_DEFINED
#define LEONARDOEDITOR_H_DEFINED

#ifdef _WIN32
  #include <Windows.h>
#endif

#include <EditorHeaders.h>

class FilterViewport;

namespace LeonardoSpace
{
  /**
    User interface for the Leonardo processor.
    @see Leonardo
  */

  class LeonardoEditor : public GenericEditor,
      public Label::Listener
  {
  public:
      LeonardoEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
      virtual ~LeonardoEditor();

      void buttonEvent(Button* button);
      void labelTextChanged(Label* label);

      void saveCustomParameters(XmlElement* xml);
      void loadCustomParameters(XmlElement* xml);

      void setDefaults(double startTime, double endTime, double diffAmp);
      void resetToSavedText();

      void channelChanged (int chan, bool newState);

  private:

      String lastStartTimeString;
      ScopedPointer<Label> startTimeLabel;
      ScopedPointer<Label> startTimeValue;

      String lastEndTimeString;
      ScopedPointer<Label> endTimeLabel;
      ScopedPointer<Label> endTimeValue;

      String lastDiffAmpString;
      ScopedPointer<Label> diffAmpLabel;
      ScopedPointer<Label> diffAmpValue;

      ScopedPointer<UtilityButton> applyDiff;
      // ScopedPointer<UtilityButton> applyDiff;

      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LeonardoEditor);

  };
}

#endif  // LEONARDOEDITOR_H_DEFINED
