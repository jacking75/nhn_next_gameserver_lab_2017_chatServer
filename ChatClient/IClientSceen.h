#pragma once

#include <nana/gui/wvl.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui.hpp>


using namespace nana;

#include "../Common/PacketID.h"

class TcpNetwork;

using PACKET_ID = NCommon::PACKET_ID;


enum class CLIENT_SCEEN_TYPE
{
	CONNECT = 0,
	LOGIN = 1,
	LOBBY = 2,
	ROOM = 3,
};

class IClientSceen
{
public:
	IClientSceen() {}
	virtual ~IClientSceen() {}

	virtual void Update() {}

	virtual bool ProcessPacket(const short packetId, char* pData) { return false; }


	void SetNetwork(TcpNetwork* pNetwork) { m_pRefNetwork = pNetwork; }

	
	static void UnicodeToAnsi(const wchar_t* pszText, const int destSize, char* pszDest)
	{
		_snprintf_s(pszDest, destSize, _TRUNCATE, "%S", pszText);
	}
	
	static CLIENT_SCEEN_TYPE GetCurSceenType() { return m_CurSceenType; }
	static void SetCurSceenType(const CLIENT_SCEEN_TYPE sceenType) { m_CurSceenType = sceenType; }


protected:
	TcpNetwork* m_pRefNetwork;

	static CLIENT_SCEEN_TYPE m_CurSceenType;
};

CLIENT_SCEEN_TYPE IClientSceen::m_CurSceenType = CLIENT_SCEEN_TYPE::CONNECT;