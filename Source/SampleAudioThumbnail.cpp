#include "SampleAudioThumbnail.h"
#include "SamplifyLookAndFeel.h"

using namespace samplore;

SampleAudioThumbnail::SampleAudioThumbnail(int sourceSamplesPerThumbnailSample,
	AudioFormatManager& formatManagerToUse,
	AudioThumbnailCache& cacheToUse) :
	AudioThumbnail(sourceSamplesPerThumbnailSample, formatManagerToUse, cacheToUse)
{

}

SampleAudioThumbnail::~SampleAudioThumbnail()
{
}

void SampleAudioThumbnail::drawChannel(Graphics & g, const Rectangle<int>& area, double startTimeSeconds, double endTimeSeconds, int channelNum, float verticalZoomFactor)
{
	drawChannel(g, area, startTimeSeconds, endTimeSeconds, channelNum, verticalZoomFactor, AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT);
}

void SampleAudioThumbnail::drawChannel(Graphics& g, const Rectangle<int>& area, double startTimeSeconds, double endTimeSeconds, int channelNum, float verticalZoomFactor, int lineCount)
{
	float peak = getApproximatePeak();

	float widthSegment = ((float)area.getWidth()) / lineCount;
	double segmentTimeLength = (endTimeSeconds - startTimeSeconds) / lineCount;

	for (int i = 0; i < lineCount; i++)
	{
		float max, min;
		getApproximateMinMax(startTimeSeconds + (segmentTimeLength * i), startTimeSeconds + (segmentTimeLength * i + 1), channelNum, min, max);
		//min /= peak; //normalize
		//max /= peak; //normalize
		float posX = area.getPosition().x + (widthSegment * i);
		float posY = ((area.getPosition().y) + (area.getHeight() / 2)) - (max * (area.getHeight() / 2));
		float sizeX = widthSegment * (AppValues::getInstance().AUDIO_THUMBNAIL_LINE_FILL_WIDTH / (AppValues::getInstance().AUDIO_THUMBNAIL_LINE_FILL_WIDTH + AppValues::getInstance().AUDIO_THUMBNAIL_LINE_GAP_WIDTH));
		float sizeY = ((area.getHeight() / 2) * (min * -1)) + (max * (area.getHeight() / 2));
		Rectangle<float> rectangleArea(posX, posY, sizeX, sizeY);
		//Rectangle<float> r(Point<float>(0,0), Point<float>(0,0))
		g.fillRoundedRectangle(rectangleArea, 2.0f);
	}
}

void SampleAudioThumbnail::drawChannels(Graphics& g, const Rectangle<int>& area, double startTimeSeconds, double endTimeSeconds, float verticalZoomFactor, int lines)
{
	float heightChunk = area.getHeight() / getNumChannels();
	for (int i = 0; i < getNumChannels(); i++)
	{
		Rectangle<int> channelArea(area.getTopLeft().x, area.getTopLeft().y + (i * heightChunk), area.getWidth(), heightChunk);
		drawChannel(g, channelArea, startTimeSeconds, endTimeSeconds, i, verticalZoomFactor, lines);
	}
}

void SampleAudioThumbnail::drawChannels(Graphics & g, const Rectangle<int>& area, double startTimeSeconds, double endTimeSeconds, float verticalZoomFactor)
{
	drawChannels(g, area, startTimeSeconds, endTimeSeconds, verticalZoomFactor, AppValues::getInstance().AUDIO_THUMBNAIL_LINE_COUNT);
}