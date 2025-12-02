#include "SampleTile.h"

#include "SamplifyLookAndFeel.h"
#include "TagTile.h"
#include "SamplifyMainComponent.h"
#include "ThemeManager.h"
#include "UI/IconLibrary.h"

#include <iomanip>
#include <sstream>

using namespace samplore;

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
}

SampleTile::~SampleTile()
{
	// Remove ourselves as listener from the sample before destruction
	if (!mSample.isNull())
		mSample.removeChangeListener(this);
	
	ThemeManager::getInstance().removeListener(this);
}
void SampleTile::paint (Graphics& g)
{
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

		// Draw shadow (elevation level 1)
		if (!isHovered)
		{
			DropShadow shadow(theme.getColorForRole(ThemeManager::ColorRole::Background).withAlpha(0.5f),
			                  8, Point<int>(0, 2));
			shadow.drawForRectangle(g, getLocalBounds().toNearestInt());
		}
		else
		{
			// Larger shadow on hover (elevation level 2)
			DropShadow shadow(theme.getColorForRole(ThemeManager::ColorRole::Background).withAlpha(0.6f),
			                  12, Point<int>(0, 4));
			shadow.drawForRectangle(g, getLocalBounds().toNearestInt());
		}

		// Draw background
		g.setColour(backgroundColor);
		g.fillRoundedRectangle(getLocalBounds().toFloat(), cornerRadius);

		// Draw info icon with padding
		Rectangle<int> titleRect = m_TitleRect.reduced(padding, padding / 2);
		if (mSample.getInfoText() != "" || mSample.getColor().getAlpha() != 0.0f)
		{
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

		// Draw title with modern typography (20px)
		g.setFont(FontOptions(20.0f, Font::bold));
		g.setColour(titleColor);
		g.drawText(mSample.getFile().getFileName(), titleRect, Justification::centredLeft, true);

		// Draw time with secondary text color
		g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary));
		g.setFont(FontOptions(14.0f));
		std::stringstream secondsStr;
		std::stringstream minutesStr;
		int minutes = ((int)mSample.getLength()) / 60;
		secondsStr << std::fixed << std::setprecision(1) << (mSample.getLength() - (60.0*minutes));
		minutesStr << std::fixed << minutes;

		auto timeRect = m_TimeRect.reduced(padding / 2, padding / 2);
		g.drawText(String(secondsStr.str()) + " sec", timeRect, Justification::bottom);
		g.drawText(String(minutesStr.str()) + " min", timeRect, Justification::top);

		// Draw waveform thumbnail with modern styling
		std::shared_ptr<SampleAudioThumbnail> thumbnail = mSample.getThumbnail();
		if (thumbnail->isFullyLoaded())
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

		// Draw playback position indicators
		std::shared_ptr<AudioPlayer> auxPlayer = SamplifyProperties::getInstance()->getAudioPlayer();
		if (auxPlayer->getSampleReference() == mSample)
		{
			auto waveformRect = m_ThumbnailRect.reduced(padding / 2, 0);
			float startT = auxPlayer->getStartCueRelative();
			float currentT = auxPlayer->getRelativeTime();
			float startX = waveformRect.getX() + (waveformRect.getWidth() * startT);
			float currentX = waveformRect.getX() + (waveformRect.getWidth() * currentT);
			float y1 = waveformRect.getY();
			float y2 = waveformRect.getBottom();

			// Draw start position with subtle color
			g.setColour(theme.getColorForRole(ThemeManager::ColorRole::TextSecondary).withAlpha(0.5f));
			g.drawLine(startX, y1, startX, y2, 1.5f);

			// Draw current position with accent color
			if (auxPlayer->getState() == AudioPlayer::TransportState::Playing)
			{
				g.setColour(theme.getColorForRole(ThemeManager::ColorRole::AccentSecondary));
				g.drawLine(currentX, y1, currentX, y2, 2.0f);
				repaint();
			}
		}

		// Update tags
		mTagContainer.setTags(mSample.getTags());
	}
	else
	{
		//reset all info if no sample currently active
		mTagContainer.setTags(StringArray());
	}
}

void SampleTile::resized()
{
	const int padding = 12;
	const int titleHeight = 32; // Height for 20px font + spacing

	// Core Rects with modern spacing
	m_TitleRect = Rectangle<int>(0, 0, getWidth(), titleHeight);
	m_TypeRect = Rectangle<int>(0, getHeight() - (getWidth() / 5), getWidth() / 5, getWidth() / 5);
	m_TimeRect = Rectangle<int>(getWidth() / 5, getHeight() - (getWidth() / 5), getWidth() / 5, getWidth() / 5);

	// Derivative Rects
	int startY = m_TitleRect.getHeight();
	m_ThumbnailRect = Rectangle<int>(0, startY, getWidth(), getHeight() - (startY + (getWidth() / 5)));

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
				float rectWidth = m_ThumbnailRect.getWidth();
				float mouseDownX = e.getMouseDownX();
				SamplifyProperties::getInstance()->getAudioPlayer()->loadFile(mSample);
				SamplifyProperties::getInstance()->getAudioPlayer()->playSample(mouseDownX / rectWidth);
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
				menu.addItem((int)RightClickOptions::openExplorer, "Open in Explorer", true, false); //QEDITOR IS THE PLACE TO BREAK A SAMPLE
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
		std::shared_ptr<AudioPlayer> aux = SamplifyProperties::getInstance()->getAudioPlayer();
		if (aux->getSampleReference() == mSample)
		{
			if (!(aux->getState() == AudioPlayer::TransportState::Starting ||
				aux->getState() == AudioPlayer::TransportState::Stopped || 
				aux->getState() == AudioPlayer::TransportState::Stopping))
			{
				aux->removeChangeListener(this);
			}
		}
		m_InfoIcon.setTooltip(mSample.getInfoText());
	}
	resized();
	repaint();
}

void SampleTile::setSample(Sample::Reference sample)
{
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
				mSample.removeChangeListener(this);
			}
		}
		if (!alreadyThis)
		{
			
			sample.generateThumbnailAndCache();
			m_InfoIcon.setTooltip(sample.getInfoText());
			sample.addChangeListener(this);
		}
	}
	else
	{
		m_InfoIcon.setTooltip("");
	}
	mSample = sample;
	repaint();
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
