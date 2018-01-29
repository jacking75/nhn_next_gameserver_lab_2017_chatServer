#pragma once

#include <iostream>
#include <chrono>
#include "IClientSceen.h"

class ClientSceenSelLobby : public IClientSceen
{
public:
	ClientSceenSelLobby() {}
	virtual ~ClientSceenSelLobby() {}

	virtual void Update() override
	{
		if (GetCurSceenType() != CLIENT_SCEEN_TYPE::LOGIN) {
			return;
		}

		auto curTime = std::chrono::system_clock::now();

		auto diffTimeSec = std::chrono::duration_cast<std::chrono::seconds>(curTime - m_TimeLastedReqLobbyList);

		if (diffTimeSec.count() > 10)
		{
			m_TimeLastedReqLobbyList = curTime;

			RequestLobbyList();
		}
	}

	
	void CreateUI(form* pform)
	{
		m_pForm = pform;

		m_LobbyList = std::make_unique<listbox>((form&)*m_pForm, nana::rectangle(22, 106, 165, 383));
		m_LobbyList->append_header("LobbyId", 60);//L"·ÎºñID"
		m_LobbyList->append_header("Cur", 40);
		m_LobbyList->append_header("Max", 40);

		m_btnEnterLobby = std::make_unique<button>((form&)*m_pForm, nana::rectangle(22, 490, 102, 23));
		m_btnEnterLobby->caption("Enter Channel");
		m_btnEnterLobby->events().click([&]() {
			this->RequestEnterLobby();
		});
		//
	}


	bool ProcessPacket(const short packetId, char* pData) override
	{
		switch (packetId)
		{
		case (short)PACKET_ID::LOBBY_LIST_RES:
		{
			auto pktRes = (NCommon::PktLobbyListRes*)pData;

			if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
			{
				m_LobbyList->clear();

				for (int i = 0; i < pktRes->LobbyCount; ++i)
				{
					auto& pLobby = pktRes->LobbyList[i];

					m_LobbyList->at(0).append({ std::to_string(pLobby.LobbyId),
													std::to_string(pLobby.LobbyUserCount), 
													std::to_string(50) });
				}
			}
			else
			{
				std::cout << "[LOBBY_LIST_RES] ErrorCode: " << pktRes->ErrorCode << std::endl;
			}
		}
		break;
		default:
			return false;
		}

		return true;
	}


private:
	void RequestLobbyList()
	{
		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_LIST_REQ, 0, nullptr);
	}

	void RequestEnterLobby()
	{
		if (GetCurSceenType() != CLIENT_SCEEN_TYPE::LOGIN) 
		{
			nana::msgbox m((form&)*m_pForm, "Require LogIn", nana::msgbox::ok);
			m.icon(m.icon_warning).show();
			return;
		}

		auto selItem = m_LobbyList->selected();
		if (selItem.empty()) 
		{
			nana::msgbox m((form&)*m_pForm, "Fail Don't Select Lobby", nana::msgbox::ok);
			m.icon(m.icon_warning).show();
			return;
		}

		auto index = selItem[0].item;
		auto lobbyId = std::atoi(m_LobbyList->at(0).at(index).text(0).c_str());
		NCommon::PktLobbyEnterReq reqPkt;
		reqPkt.LobbyId = 0;
		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_ENTER_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}

private:
	form* m_pForm = nullptr;

	std::unique_ptr<button> m_btnEnterLobby;
	std::unique_ptr<listbox> m_LobbyList;

	std::chrono::system_clock::time_point m_TimeLastedReqLobbyList = std::chrono::system_clock::now();
};