#include "AudioPlayer.h"

using namespace samplore;

AudioPlayer::AudioPlayer()
{
	formatManager.registerBasicFormats();
	transportSource.addChangeListener(this);
	state = TransportState::Stopped;
}

AudioPlayer::~AudioPlayer()
{
	// Remove ourselves as a listener from the transport source
	transportSource.removeChangeListener(this);
}

void AudioPlayer::play()
{
	setRelativeTime(mSampleStartT);
	changeState(TransportState::Starting);
}

void AudioPlayer::reset()
{
	setRelativeTime(mSampleStartT);
}

void AudioPlayer::stop()
{
	transportSource.stop();
	sendChangeMessage();
}

void samplore::AudioPlayer::toggle()
{
	if (state == TransportState::Playing)
	{
		changeState(TransportState::Stopping);
	}
	else if (state == TransportState::Stopped)
	{
		reset();
		changeState(TransportState::Starting);
	}
}

void AudioPlayer::setSample(Sample::Reference sample)
{
	if (getSampleReference() != sample)
	{
		loadFile(sample);
	}
}

void AudioPlayer::playSample()
{
	playSample(0.0f);
}

void samplore::AudioPlayer::playSample(float t)
{
	if (!mCurrentSample.isNull())
	{
		stop();
		reset();
		setRelativeTime(t);
		play();
	}
}

void AudioPlayer::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == &transportSource)
	{
		if (transportSource.isPlaying())
			changeState(TransportState::Playing);
		else
			changeState(TransportState::Stopped);
	}
}

void AudioPlayer::releaseResources()
{
	transportSource.releaseResources();
}

void AudioPlayer::setRelativeTime(double t)
{
	t = std::clamp(t, 0.0, 1.0);
	mSampleStartT = t;
	//float currentPos = transportSource.getCurrentPosition();
	//int64 currentReadPos = transportSource.getNextReadPosition();
	float sampleLength = transportSource.getLengthInSeconds();
	float newPos = sampleLength * t;
	transportSource.setPosition(newPos);
	sendChangeMessage();
}
	

void AudioPlayer::changeState(TransportState newState)
{
	if (state != newState)
	{
		state = newState;

		switch (state)
		{
		case TransportState::Stopped:
			setRelativeTime(mSampleStartT);
			break;
		case TransportState::Stopping:
			transportSource.stop();
			break;
		case TransportState::Playing:
			break;
		case TransportState::Starting:
			transportSource.start();
			changeState(TransportState::Playing);
			break;
		}
	}
	sendChangeMessage();
}


void AudioPlayer::loadFile(Sample::Reference ref)
{
	AudioFormatReader* reader = formatManager.createReaderFor(ref.getFile());
	mCurrentSample = ref;
	juce::Time currentTime = juce::Time::getCurrentTime();
	if (reader != nullptr && (currentTime - mTimeSinceLoaded).inSeconds() > 0.2f && state != TransportState::Starting)
	{
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
		readerSource.reset(newSource.release());
	}
	sendChangeMessage();
}

void AudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}


void AudioPlayer::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	if (readerSource.get() == nullptr || state == TransportState::Stopped || transportSource.hasStreamFinished())
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}
	transportSource.getNextAudioBlock(bufferToFill) ;
}
