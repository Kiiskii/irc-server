#pragma once

enum	channelMsg
{
	NO_MSG,
	JOIN_OK,
	ALREADY_ON_CHAN,
	CHANGE_TOPIC_MSG,
	//below not use
	INVITE_ONLY_CHAN,

	// mode response, should move out??
	SET_MODE_OK,
	NO_ACTION,
	UNKNOWN_MODE,
	

};