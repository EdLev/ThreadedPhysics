#pragma once

enum MessagePumpResult
{
	NoMessages,
	MoreMessages,
	Quit
};

class PlatformManager
{
public:
	MessagePumpResult PumpMessage();
};