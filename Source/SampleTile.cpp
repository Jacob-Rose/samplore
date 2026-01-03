#include "SampleTile.h"

#include "SamplifyLookAndFeel.h"
#include "TagTile.h"
#include "SamplifyMainComponent.h"
#include "ThemeManager.h"
#include "UI/IconLibrary.h"
#include "PerformanceProfiler.h"
#include "CueManager.h"

#include <iomanip>
#include <sstream>

using namespace samplore;

// Static font cache for performance
Font SampleTile::getTitleFont()
{
	static Font titleFont(Font::getDefaultSansSerifFontName(), 15.0f, Font::bold);
	return titleFont;
}

Font SampleTile::getTimeFont()
{
	static Font timeFont(Font::getDefaultSansSerifFontName(), 14.0f, Font::plain);
	return timeFont;
}

SampleTile::SampleTile(Sample::Reference sample) : mTagContainer(false)
{
	setRepaintsOnMouseActivity(true);
	setSize(AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH, AppValues::getInstance().SAMPLE_TILE_MIN_WIDTH * AppValues::getInstance().SAMPLE_TILE_ASPECT_RATIO);
	setSample(sample);
    mTagContainer.addMouseListener(this, false);
	addAndMakeVisible(mTagContainer);
	addAndMakeVisible(m_InfoIcon);

	// Register with ThemeManager
	ThemeManager::getInstance().addListener(this);

	// Register with AudioPlayer to get notified when active sample changes
	if (auto player = SamplifyProperties::getInstance()->getAudioPlayer())
	{
		player->addChangeListener(this);
	}

	// Enable buffering by default (disabled dynamically when playing)
	setBufferedToImage(true);
}

SampleTile::~SampleTile()
{
	// Stop animation timer
	mRainbowTimer.stopTimer();

	// Remove ourselves as listener from the sample before destruction
	if (!mSample.isNull())
		mSample.removeChangeListener(this);

	// Remove from AudioPlayer
	if (auto player = SamplifyProperties::getInstance()->getAudioPlayer())
	{
		player->removeChangeListener(this);
	}

	// Clear sample reference to ensure thumbnail cleanup
	mSample = nullptr;

	ThemeManager::getInstance().removeListener(this);
}
void SampleTile::paint (Graphics& g)
{
	PROFILE_PAINT("SampleTile::paint");

	if (!mSample.isNull())
	{
		auto& theme = ThemeManager::getInstance();
		const float cornerRadius = 12.0f;
		const int padding = 12;

		// Setup colors
		Colour backgroundColor;
		Colour foregroundColor;
		Colour titleColor;
		bool isHovered = isMouseOver(true);

		if (isHovered)
		{
			backgroundColor = theme.getColorForRole(ThemeManager::ColorRole::SurfaceHover);
			foregroundColor = theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary);
		}
		else
		{
			backgroundColor = theme.getColorForRole(ThemeManager::ColorRole::Surface);
			foregroundColor = theme.getColorForRole(ThemeManager::ColorRole::WaveformPrimary);
		}

		titleColor = theme.getColorForRole(ThemeManager::ColorRole::TextPrimary);

		// Draw background
		{
			PROFILE_SCOPE("SampleTile::paint::background");
			g.setColour(backgroundColor);
			g.fillRoundedRectangle(getLocalBounds().toFloat(), cornerRadius);

			// Subtle border instead of expensive DropShadow
			if (isHovered)
			{
				g.setColour(theme.getColorForRole(ThemeManager::ColorRole::AccentPrimary).withAlpha(0.3f));
				g.drawRoundedRectangle(getLocalBounds().toFloat(), cornerRadius, 2.0f);
			}
			else
			{
				g.setColour(theme.getColorForRole(ThemeManager::ColorRole::Background).withAlpha(0.2f));
				g.drawRoundedRectangle(getLocalBounds().toFloat(), cornerRadius, 1.0f);
			}
		}

		// Draw info icon with padding
		Rectangle<int> titleRect = m_TitleRect.reduced(padding, padding / 2);
		if (mSample.getInfoText() != "" || mSample.getColor().getAlpha() != 0.0f)
		{
			PROFILE_SCOPE("SampleTile::paint::infoIcon");
			if (mSample.getColor().getFloatAlpha() > 0.0f)
			{
				auto iconBounds = m_InfoIcon.getBounds().reduced(INFO_ICON_PADDING + 2).toFloat();
				g.setColour(mSample.getColor());
				g.fillEllipse(iconBounds);
				g.setColour(mSample.getColor().darker(0.3f));
				g.drawEllipse(iconBounds, 1.5f);
			}

			titleRect = titleRect.withTrimmedLeft(m_InfoIcon.getWidth());
		}

		// Draw title with cached font
		{
			PROFILE_SCOPE("SampleTile::paint::title");
			g.setFont(getTitleFont());
			g.setColour(titleColor);
			g.drawText(mSample.getFile().getFileName(), titleRect, Justification::centredLeft, true);
		}

		// Draw time with cached font
		{
			PROFILE_SCOPE("SampleTile::paint::timeLabels");
			g.setFont(FontOptions(15.0f));
			g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));

			// Cache time calculation
			double length = mSample.getLength();
			int minutes = static_cast<int>(length) / 60;
			int seconds = static_cast<int>(length - (60.0 * minutes));

			// Format as "3min24sec"
			String timeString = String(minutes) + "min" + String(seconds) + "sec";

			auto timeRect = m_TimeRect.reduced(padding / 2, padding / 2);
			g.drawText(timeString, timeRect, Justification::centredLeft);
		}

		// Draw waveform thumbnail with modern styling
		{
			PROFILE_SCOPE("SampleTile::paint::waveform");
			std::shared_ptr<SampleAudioThumbnail> thumbnail = mSample.getThumbnail();
			if (thumbnail && thumbnail->isFullyLoaded())
			{
				if (thumbnail->getNumChannels() != 0)
				{
					// Use waveform color from theme with opacity based on amplitude
					g.setColour(foregroundColor.withAlpha(0.9f));
					auto waveformRect = m_ThumbnailRect.reduced(padding / 2, 0);
					thumbnail->drawChannel(g, waveformRect.toNearestInt(),
					                       0.0, thumbnail->getTotalLength(), 0, 1.0f);
				}
			}
		}

		// Draw cue point indicators (like Ableton hot cues)
		{
			PROFILE_SCOPE("SampleTile::paint::cueIndicators");
			auto& cueManager = CueManager::getInstance();
			const auto& bindings = cueManager.getBindings();
			auto waveformRect = m_ThumbnailRect.reduced(padding / 2, 0).toFloat();

			for (const auto& [key, binding] : bindings)
			{
				// Skip if not for this sample
				if (binding.mSample.isNull() || binding.mSample != mSample)
					continue;

				// Calculate x position from start time
				float cueX = waveformRect.getX() + (waveformRect.getWidth() * static_cast<float>(binding.mStartTime));
				float y1 = waveformRect.getY();
				float y2 = waveformRect.getBottom();

				// Get cue color
				Colour cueColor = binding.getColor();

				// Draw vertical line
				g.setColour(cueColor.withAlpha(0.9f));
				g.drawLine(cueX, y1, cueX, y2, 2.0f);

				// Draw play triangle at top
				const float triangleSize = 8.0f;
				Path triangle;
				triangle.addTriangle(
					cueX, y1,                                    // Top point
					cueX - triangleSize * 0.6f, y1 + triangleSize,  // Bottom left
					cueX + triangleSize * 0.6f, y1 + triangleSize   // Bottom right
				);
				g.setColour(cueColor);
				g.fillPath(triangle);

				// Draw key label below triangle
				g.setColour(cueColor.darker(0.2f));
				auto keyStr = CueManager::getKeyDisplayString(key);
				auto labelRect = Rectangle<float>(cueX - 8.0f, y1 + triangleSize, 16.0f, 12.0f);
				g.setFont(FontOptions(10.0f).withStyle("Bold"));
				g.drawText(keyStr, labelRect, Justification::centred, false);
			}
		}

		// Draw playback position indicators
		{
			PROFILE_SCOPE("SampleTile::paint::playbackIndicators");
			std::shared_ptr<AudioPlayer> auxPlayer = SamplifyProperties::getInstance()->getAudioPlayer();
			bool isCurrentlyPlaying = (auxPlayer->getSampleReference() == mSample) &&
			                          (auxPlayer->getState() == AudioPlayer::TransportState::Playing);

			// Dynamic buffering: disable when playing (frequent full repaints)
			if (isCurrentlyPlaying != mIsPlaying)
			{
				mIsPlaying = isCurrentlyPlaying;
				Component::SafePointer<SampleTile> safeThis(this);
				MessageManager::callAsync([safeThis, shouldBuffer = !isCurrentlyPlaying]() {
					if (safeThis != nullptr)
						safeThis->setBufferedToImage(shouldBuffer);
				});
			}

			// Draw playback indicator overlay when playing
			if (mIsPlaying)
			{
				PROFILE_SCOPE("SampleTile::paint::playbackIndicator");
				auto waveformRect = m_ThumbnailRect.reduced(padding / 2, 0).toFloat();
				auto& appValues = AppValues::getInstance();

				if (appValues.PLAYBACK_INDICATOR_MODE == PlaybackIndicatorMode::StaticColor)
				{
					// Static color mode
					g.setColour(appValues.PLAYBACK_INDICATOR_COLOR.withAlpha(0.15f));
					g.fillRoundedRectangle(waveformRect, 4.0f);
				}
				else
				{
					// Rainbow modes (animated or static)
					float animPhase = 0.0f;
					if (appValues.PLAYBACK_INDICATOR_MODE == PlaybackIndicatorMode::AnimatedRainbow)
					{
						double currentTime = Time::getMillisecondCounterHiRes();
						animPhase = static_cast<float>(std::fmod(currentTime * 0.0002, 1.0));
					}

					// Create rainbow gradient
					ColourGradient rainbow;
					rainbow.isRadial = false;
					rainbow.point1 = Point<float>(waveformRect.getX(), waveformRect.getCentreY());
					rainbow.point2 = Point<float>(waveformRect.getRight(), waveformRect.getCentreY());

					const int numColors = 7;
					for (int i = 0; i < numColors; ++i)
					{
						float position = static_cast<float>(i) / (numColors - 1);
						float hue = std::fmod(position + animPhase, 1.0f);
						Colour rainbowColor = Colour::fromHSV(hue, 0.7f, 1.0f, 0.15f);
						rainbow.addColour(position, rainbowColor);
					}

					g.setGradientFill(rainbow);
					g.fillRoundedRectangle(waveformRect, 4.0f);
				}
			}

			auto waveformRect = m_ThumbnailRect.reduced(padding / 2, 0);
			float y1 = static_cast<float>(waveformRect.getY());
			float y2 = static_cast<float>(waveformRect.getBottom());

			// Draw start position and playback position when this sample is loaded in player
			bool isCurrentSample = (auxPlayer->getSampleReference() == mSample);

			// Start/stop rainbow animation timer based on active state
			if (isCurrentSample != mIsActiveSample)
			{
				mIsActiveSample = isCurrentSample;
				if (mIsActiveSample)
					mRainbowTimer.startTimerHz(30);
				else
					mRainbowTimer.stopTimer();
			}

			if (isCurrentSample)
			{
				float startT = auxPlayer->getStartCueRelative();
				float startX = waveformRect.getX() + (waveformRect.getWidth() * startT);

				// Draw start position with animated rainbow color and triangle
				{
					double currentTime = Time::getMillisecondCounterHiRes();
					float animPhase = static_cast<float>(std::fmod(currentTime * 0.0003, 1.0));
					Colour rainbowColor = Colour::fromHSV(animPhase, 0.8f, 0.9f, 1.0f);

					// Draw vertical line with rainbow color
					g.setColour(rainbowColor.withAlpha(0.7f));
					g.drawLine(startX, y1, startX, y2, 2.0f);

					// Draw rainbow triangle at top
					const float triangleSize = 8.0f;
					Path triangle;
					triangle.addTriangle(
						startX, y1,
						startX - triangleSize * 0.6f, y1 + triangleSize,
						startX + triangleSize * 0.6f, y1 + triangleSize
					);
					g.setColour(rainbowColor);
					g.fillPath(triangle);
				}

				// Draw current playback position with accent color when playing
				if (mIsPlaying)
				{
					float currentT = auxPlayer->getRelativeTime();
					float currentX = waveformRect.getX() + (waveformRect.getWidth() * currentT);

					g.setColour(theme.getColorForRole(ThemeManager::ColorRole::AccentSecondary));
					g.drawLine(currentX, y1, currentX, y2, 2.0f);
				}
			}
		}
	}
}

void SampleTile::resized()
{
	PROFILE_SCOPE("SampleTile::resized");

	const int padding = 12;
	const int titleHeight = 24; // Reduced height for 15px font + minimal spacing
	const int bottomInfoHeight = getWidth() / 4; // Increased from 1/5 to 1/4 for more space

	// Core Rects with modern spacing
	m_TitleRect = Rectangle<int>(0, 0, getWidth(), titleHeight);
	m_TypeRect = Rectangle<int>(0, getHeight() - bottomInfoHeight, getWidth() / 5, bottomInfoHeight);

	// TimeRect now spans both positions 0 and 1 (2/5 of width)
	m_TimeRect = Rectangle<int>(0, getHeight() - bottomInfoHeight, (getWidth() * 2) / 5, bottomInfoHeight);

	// Derivative Rects
	int startY = m_TitleRect.getHeight();
	m_ThumbnailRect = Rectangle<int>(0, startY, getWidth(), getHeight() - (startY + bottomInfoHeight));

	int offset = (m_TitleRect.getHeight() + m_ThumbnailRect.getHeight());
	m_TagRect = Rectangle<int>(getWidth() / 2, offset, getWidth() / 2, getHeight() - offset);
	mTagContainer.setBounds(m_TagRect.reduced(padding / 2));

	// Info icon in top left corner
	m_InfoIcon.setBounds(padding / 2, padding / 2, titleHeight - padding, titleHeight - padding);
}

bool SampleTile::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
	if (dragSourceDetails.description == "Tags")
	{
		return true;
	}
	else
	{
		return false;
	}
}


void SampleTile::mouseUp(const MouseEvent& e)
{
	if (!mSample.isNull())
	{
		if (e.mods.isLeftButtonDown())
		{
			SamplifyProperties::getInstance()->getAudioPlayer()->loadFile(mSample);
			if (m_ThumbnailRect.contains(e.getMouseDownPosition()))
			{
				SamplifyProperties::getInstance()->getAudioPlayer()->playSample();
			}
		}
		else if (e.mods.isRightButtonDown())
		{
			if (m_ThumbnailRect.contains(e.getMouseDownPosition()) && AppValues::getInstance().RIGHTCLICKPLAYFROMPOINT)
			{
				// Use the same reduced waveform rect as paint() for accurate positioning
				const int padding = 12;
				auto waveformRect = m_ThumbnailRect.reduced(padding / 2, 0);
				float rectWidth = static_cast<float>(waveformRect.getWidth());
				float mouseDownX = static_cast<float>(e.getMouseDownX() - waveformRect.getX());
				float startPos = juce::jlimit(0.0f, 1.0f, mouseDownX / rectWidth);
				SamplifyProperties::getInstance()->getAudioPlayer()->loadFile(mSample);
				SamplifyProperties::getInstance()->getAudioPlayer()->playSample(startPos);
			}
			/*
			else if (m_TitleRect.contains(e.getMouseDownPosition().toFloat()) && e.mods.isLeftButtonDown())
			{
				PopupMenu menu;
				StringArray parentDirs = mSample.getRelativeParentFolders();
				for (int i = 0; i < parentDirs.size(); i++)
				{
					menu.addItem(i + 1, parentDirs[i]);
				}
				int choice = menu.show();
			}
			*/
			else
			{
				PopupMenu menu;
				menu.addItem((int)RightClickOptions::openExplorer, "Open in Explorer", true, false);
				menu.addSeparator();
				menu.addItem((int)RightClickOptions::renameSample, "Rename", true, false);
				menu.addItem((int)RightClickOptions::deleteSample, "Move To Trash", true, false);

				auto sampleFile = mSample.getFile();
				menu.showMenuAsync(PopupMenu::Options(), [this, sampleFile](int selection)
				{
					if (selection == (int)RightClickOptions::openExplorer)
					{
						sampleFile.revealToUser();
					}
					else if (selection == (int)RightClickOptions::renameSample)
					{
						mFileChooser = std::make_unique<FileChooser>("rename file", sampleFile);
						mFileChooser->launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles,
							[this, sampleFile](const FileChooser& fc)
							{
								auto result = fc.getResult();
								if (result != File() && sampleFile.moveFileTo(result))
								{
									SamplifyProperties::getInstance()->getSampleLibrary()->refreshCurrentSamples();
								}
							});
					}
					else if (selection == (int)RightClickOptions::deleteSample)
					{
						auto options = MessageBoxOptions()
							.withIconType(MessageBoxIconType::WarningIcon)
							.withTitle("Delete Sample?")
							.withMessage("Are you sure you want to delete this sample?")
							.withButton("Yes")
							.withButton("No");
						NativeMessageBox::showAsync(options, [sampleFile](int result)
						{
							if (result == 1) // Yes
							{
								if (sampleFile.moveToTrash())
								{
									SamplifyProperties::getInstance()->getSampleLibrary()->refreshCurrentSamples();
								}
								else
								{
									auto errorOptions = MessageBoxOptions()
										.withIconType(MessageBoxIconType::WarningIcon)
										.withTitle("Error in Throwing Away")
										.withMessage("Failed to move item to trash, check if it is full!")
										.withButton("OK");
									NativeMessageBox::showAsync(errorOptions, nullptr);
								}
							}
						});
					}
				});
			}
		}
	}
}

void SampleTile::mouseDrag(const MouseEvent& e)
{
	if (!mSample.isNull())
	{
		StringArray files = StringArray();
		files.add(mSample.getFile().getFullPathName());
		DragAndDropContainer::performExternalDragDropOfFiles(files, false);
		SamplifyProperties::getInstance()->getAudioPlayer()->stop();
	}
}

void SampleTile::mouseExit(const MouseEvent& e)
{
	repaint();
}

void SampleTile::itemDropped(const SourceDetails & dragSourceDetails)
{
	if (!mSample.isNull())
	{
		if (TagTile * tagComp = dynamic_cast<TagTile*>(dragSourceDetails.sourceComponent.get()))
		{
			mSample.addTag(tagComp->getTag());
			mTagContainer.setTags(mSample.getTags());
		}
	}
}

void SampleTile::changeListenerCallback(ChangeBroadcaster* source)
{
	if (!mSample.isNull())
	{
		m_InfoIcon.setTooltip(mSample.getInfoText());
		// Update tags when sample changes (e.g., tags added/removed)
		mTagContainer.setTags(mSample.getTags());
	}
	// Always repaint - handles AudioPlayer changes (active sample, playback state)
	repaint();
}

void SampleTile::setSample(Sample::Reference sample)
{
	PROFILE_SCOPE("SampleTile::setSample");

	if (!sample.isNull())
	{
		bool alreadyThis = false;
		if (!mSample.isNull())
		{
			if (mSample == sample)
			{
				alreadyThis = true;
			}
			else
			{
				PROFILE_SCOPE("SampleTile::setSample::removeListener");
				mSample.removeChangeListener(this);
			}
		}
		if (!alreadyThis)
		{
			{
				PROFILE_SCOPE("SampleTile::setSample::generateThumbnail");
				sample.generateThumbnailAndCache();
			}
			{
				PROFILE_SCOPE("SampleTile::setSample::setTooltip");
				m_InfoIcon.setTooltip(sample.getInfoText());
			}
			{
				PROFILE_SCOPE("SampleTile::setSample::addListener");
				sample.addChangeListener(this);
			}
			{
				PROFILE_SCOPE("SampleTile::setSample::updateTags");
				mTagContainer.setTags(sample.getTags());
			}
		}
	}
	else
	{
		m_InfoIcon.setTooltip("");
		mTagContainer.setTags(StringArray());
	}
	mSample = sample;
	{
		PROFILE_SCOPE("SampleTile::setSample::repaint");
		repaint();
	}
}

Sample::Reference SampleTile::getSample()
{
	return mSample;
}

SampleTile::InfoIcon::InfoIcon()
{

}

String SampleTile::InfoIcon::getTooltip()
{
	return mTooltip;
}

void SampleTile::InfoIcon::setTooltip(String newTooltip)
{
	mTooltip = newTooltip;
	repaint();
}

void SampleTile::InfoIcon::paint(Graphics& g)
{
	if (mTooltip != "")
	{
		auto& theme = ThemeManager::getInstance();
		IconLibrary::getInstance().drawIcon(g, IconLibrary::Icon::Info,
		                                     getBounds().reduced(2.0f).toFloat(),
		                                     theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
	}
}

//==============================================================================
// ThemeManager::Listener implementation
void SampleTile::themeChanged(ThemeManager::Theme newTheme)
{
	repaint();
}

void SampleTile::colorChanged(ThemeManager::ColorRole role, Colour newColor)
{
	repaint();
}
