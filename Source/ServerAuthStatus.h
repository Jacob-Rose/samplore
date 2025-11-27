/*
  ==============================================================================

    ServerAuthStatus.h
    Created: 19 Dec 2019 1:22:31pm
    Author:  jacob

  ==============================================================================
*/

#ifndef SERVERAUTHSTATUS_H
#define SERVERAUTHSTATUS_H
#pragma once

#include <JuceHeader.h>

namespace samplore
{
	class ServerAuthStatus : public OnlineUnlockStatus
	{
	public:
		String getProductID() override
		{
			return "Samplore";
		}
		bool doesProductIDMatch(const String& returnedIDFromServer) override
		{
			return getProductID() == returnedIDFromServer;
		}
		String getWebsiteName() override
		{
			return "samplore.app";
		}
		URL getServerAuthenticationURL() override
		{
			return URL("https://www.samplore.app/auth.php");
		}
		RSAKey getPublicKey() override
		{
			return RSAKey("5,7d4324cd7eeccd9f7260a2e8aebc41ad3a347f305e1189d3be423228c3a55803");
		}
		String readReplyFromWebserver(const String& email, const String& password) override;

		String getState() override;

		void saveState(const String&) override;

		void userCancelled() override;

	private:
		CriticalSection streamCreationLock;
		std::unique_ptr<WebInputStream> stream;
	};
}
#endif
