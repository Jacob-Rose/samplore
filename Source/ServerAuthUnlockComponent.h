/*
  ==============================================================================

    ServerAuthUnlockComponent.h
    Created: 19 Dec 2019 1:33:41pm
    Author:  jacob

  ==============================================================================
*/

#ifndef SERVERAUTHUNLOCKCOMP_H
#define SERVERAUTHUNLOCKCOMP_H
#include <JuceHeader.h>
#include "ServerAuthStatus.h"
namespace samplore
{
	class ServerAuthUnlockComponent : public OnlineUnlockForm
	{
	public:
		ServerAuthUnlockComponent(ServerAuthStatus& status)
			: OnlineUnlockForm(status, "Please provide your email and password.")
		{}

		void dismiss() override
		{
			setVisible(false);
		}
	private:
	};
}
#endif
