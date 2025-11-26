/*
  ==============================================================================

    TagTile.h
    Created: 31 May 2018 1:21:39pm
    Author:  Jake Rose

  ==============================================================================
*/

#ifndef TAGTILE_H
#define TAGTILE_H

#include "JuceHeader.h"
#include "Animation/AnimationManager.h"

namespace samplify
{
	class TagTile : public Component, public DragAndDropContainer, private AnimatedComponent
	{
	public:
		//==================================================
		TagTile(juce::String tag, Font& font);
		~TagTile();
		//==================================================
		void paint(Graphics&) override;
		void resized() override;
		//==================================================
		void mouseDown(const MouseEvent& e) override;
		void mouseUp(const MouseEvent& e) override;
		void mouseDrag(const MouseEvent& e) override;
		void mouseEnter(const MouseEvent& e) override;
		void mouseExit(const MouseEvent& e) override;
		//==================================================
		void setTag(juce::String tag);
		juce::String getTag() { return mTag; }

	protected:
		// AnimatedComponent interface
		void onAnimationUpdate() override { repaint(); }

	private:
		//==================================================
		juce::String mTag;
		Font* mFont;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TagTile)
	};
}
#endif
