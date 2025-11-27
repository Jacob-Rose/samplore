/*
  ==============================================================================

    ServerAuthStatus.cpp
    Created: 19 Dec 2019 1:22:31pm
    Author:  jacob

  ==============================================================================
*/

#include "ServerAuthStatus.h"

using namespace samplore;

String samplore::ServerAuthStatus::readReplyFromWebserver(const String& email, const String& password)
{
	URL url(getServerAuthenticationURL()
		.withParameter("product", getProductID())
		.withParameter("email", email)
		.withParameter("pw", password)
		.withParameter("os", SystemStats::getOperatingSystemName())
		.withParameter("mach", getLocalMachineIDs()[0]));

	DBG("Trying to unlock via URL: " << url.toString(true));

	{
		ScopedLock lock(streamCreationLock);
		stream.reset(new WebInputStream(url, true));
	}

	if (stream->connect(nullptr))
	{
		auto thread = Thread::getCurrentThread();
		MemoryOutputStream result;

		while (!(stream->isExhausted() || stream->isError()
			|| (thread != nullptr && thread->threadShouldExit())))
		{
			auto bytesRead = result.writeFromInputStream(*stream, 8192);

			if (bytesRead < 0)
				break;
		}
		
		return result.toString();

	}

	return {};
}

String samplore::ServerAuthStatus::getState()
{
    //TODO: get the key from the save location 
    return "";
}

void samplore::ServerAuthStatus::saveState(const String&)
{
    //TODO: save the key to the save location
}

void samplore::ServerAuthStatus::userCancelled()
{
    ScopedLock lock(streamCreationLock);

    if (stream != nullptr)
        stream->cancel();
}
