#pragma once

#include <list>
#include <string>
#include <vector>
#include <algorithm>

#include "IClientSceen.h"



class ClientSceenLobby : public IClientSceen
{
public:
	ClientSceenLobby() {}
	virtual ~ClientSceenLobby() {}

	virtual void Update() override 
	{
	}

	bool ProcessPacket(const short packetId, char* pData) override 
	{ 
		switch (packetId)
		{
		case (short)PACKET_ID::LOBBY_ENTER_RES:
		{
			auto pktRes = (NCommon::PktLobbyEnterRes*)pData;

			if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
			{
				Init(pktRes->MaxUserCount);

				RequestRoomList(0);
				RequestUserList(0);
			}
			else
			{
				std::cout << "[LOBBY_ENTER_RES] ErrorCode: " << pktRes->ErrorCode << std::endl;
			}
		}
			break;
		case (short)PACKET_ID::LOBBY_ENTER_ROOM_LIST_RES:
		{
			auto pktRes = (NCommon::PktLobbyRoomListRes*)pData;

			if (pktRes->IsEnd == false)
			{
				for (int i = 0; i < pktRes->Count; ++i)
				{
					UpdateRoomInfo(&pktRes->RoomInfo[i]);
				}

				RequestRoomList(pktRes->RoomInfo[pktRes->Count - 1].RoomIndex + 1);
			}
			else
			{
				SetRoomListGui();
			}
		}
			break;
		case (short)PACKET_ID::LOBBY_ENTER_USER_LIST_RES:
		{
			auto pktRes = (NCommon::PktLobbyUserListRes*)pData;

			if (pktRes->IsEnd == false)
			{
				for (int i = 0; i < pktRes->Count; ++i)
				{
					UpdateUserInfo(false, pktRes->UserInfo[i].UserID);
				}
				
				RequestUserList(pktRes->UserInfo[pktRes->Count - 1].LobbyUserIndex + 1);
			}
			else
			{
				SetUserListGui();
			}
		}
			break;
		case (short)PACKET_ID::ROOM_CHANGED_INFO_NTF:
		{
			auto pktRes = (NCommon::PktChangedRoomInfoNtf*)pData;
			UpdateRoomInfo(&pktRes->RoomInfo);
		}
			break;
		case (short)PACKET_ID::LOBBY_ENTER_USER_NTF:
		{
			auto pktRes = (NCommon::PktLobbyNewUserInfoNtf*)pData;
			UpdateUserInfo(false, pktRes->UserID);
		}
			break;
		case (short)PACKET_ID::LOBBY_LEAVE_USER_NTF:
		{
			auto pktRes = (NCommon::PktLobbyLeaveUserInfoNtf*)pData;
			UpdateUserInfo(true, pktRes->UserID);
		}
			break;
		default:
			return false;
		}

		return true;
	}

	void CreateUI(form* pform)
	{
		m_pForm = pform;

		m_LobbyRoomList = std::make_shared<listbox>((form&)*m_pForm, nana::rectangle(204, 106, 345, 383));
		m_LobbyRoomList->append_header(L"RoomId", 50);
		m_LobbyRoomList->append_header(L"Title", 165);
		m_LobbyRoomList->append_header(L"Cur", 30);
		m_LobbyRoomList->append_header(L"Max", 30);

		m_LobbyUserList = std::make_shared<listbox>((form&)*m_pForm, nana::rectangle(550, 106, 120, 383));
		m_LobbyUserList->append_header("UserID", 90);
	}

	void Init(const int maxUserCount)
	{
		m_MaxUserCount = maxUserCount;

		m_IsRoomListWorking = true;
		m_IsUserListWorking = true;

		m_RoomList.clear();
		m_UserList.clear();
	}
		
	void RequestRoomList(const short startIndex)
	{
		NCommon::PktLobbyRoomListReq reqPkt;
		reqPkt.StartRoomIndex = startIndex;
		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_ENTER_ROOM_LIST_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}

	void RequestUserList(const short startIndex)
	{
		NCommon::PktLobbyUserListReq reqPkt;
		reqPkt.StartUserIndex = startIndex;
		m_pRefNetwork->SendPacket((short)PACKET_ID::LOBBY_ENTER_USER_LIST_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}
	
	void SetRoomListGui()
	{
		m_IsRoomListWorking = false;

		for (auto & room : m_RoomList)
		{
			m_LobbyRoomList->at(0).append({ std::to_wstring(room.RoomIndex),
				room.RoomTitle,
				std::to_wstring(room.RoomUserCount),
				std::to_wstring(m_MaxUserCount) });
		}

		m_RoomList.clear();
	}

	void SetUserListGui()
	{
		m_IsUserListWorking = false;

		for (auto & userId : m_UserList)
		{
			m_LobbyUserList->at(0).append({ userId });
		}

		m_UserList.clear();
	}

	void UpdateRoomInfo(NCommon::RoomSmallInfo* pRoomInfo)
	{
		NCommon::RoomSmallInfo newRoom;
		memcpy(&newRoom, pRoomInfo, sizeof(NCommon::RoomSmallInfo));
		
		bool IsRemove = newRoom.RoomUserCount == 0 ? true : false;

		if (m_IsRoomListWorking)
		{
			if (IsRemove == false)
			{
				auto findIter = std::find_if(std::begin(m_RoomList), std::end(m_RoomList), [&newRoom](auto& room) { return room.RoomIndex == newRoom.RoomIndex; });

				if (findIter != std::end(m_RoomList))
				{
					wcsncpy_s(findIter->RoomTitle, NCommon::MAX_ROOM_TITLE_SIZE + 1, newRoom.RoomTitle, NCommon::MAX_ROOM_TITLE_SIZE);
					findIter->RoomUserCount = newRoom.RoomUserCount;
				}
				else
				{
					m_RoomList.push_back(newRoom);
				}
			}
			else
			{
				m_RoomList.remove_if([&newRoom](auto& room) { return room.RoomIndex == newRoom.RoomIndex; });
			}
		}
		else
		{
			std::string roomIndex(std::to_string(newRoom.RoomIndex));

			if (IsRemove == false)
			{
				for (auto& room : m_LobbyRoomList->at(0))
				{
					if (room.text(0) == roomIndex) 
					{
						room.text(1, newRoom.RoomTitle);
						room.text(2, std::to_wstring(newRoom.RoomUserCount));
						return;
					}
				}

				m_LobbyRoomList->at(0).append({ std::to_wstring(newRoom.RoomIndex),
											newRoom.RoomTitle,
										std::to_wstring(newRoom.RoomUserCount),
										std::to_wstring(m_MaxUserCount) });
			}
			else
			{
				for (auto& room : m_LobbyRoomList->at(0))
				{
					if (room.text(0) == roomIndex)
					{
						m_LobbyRoomList->erase(room);
						return;
					}
				}
			}
		}
	}

	void UpdateUserInfo(bool IsRemove, std::string userID)
	{		
		if (m_IsUserListWorking)
		{
			if (IsRemove == false)
			{
				auto findIter = std::find_if(std::begin(m_UserList), std::end(m_UserList), [&userID](auto& ID) { return ID == userID; });

				if (findIter == std::end(m_UserList))
				{
					m_UserList.push_back(userID);
				}
			}
			else
			{
				m_UserList.remove_if([&userID](auto& ID) { return ID == userID; });
			}
		}
		else
		{
			if (IsRemove == false)
			{
				for (auto& user : m_LobbyUserList->at(0))
				{
					if (user.text(0) == userID) {
						return;
					}
				}

				m_LobbyUserList->at(0).append(userID);
			}
			else
			{
				auto i = 0;
				for (auto& user : m_LobbyUserList->at(0))
				{
					if (user.text(0) == userID)
					{
						m_LobbyUserList->erase(user);
						return;
					}
				}
			}
		}
	}

private:
	form* m_pForm = nullptr;
	std::shared_ptr<listbox> m_LobbyRoomList;
	std::shared_ptr<listbox> m_LobbyUserList;
	
	int m_MaxUserCount = 0;

	bool m_IsRoomListWorking = false;
	std::list<NCommon::RoomSmallInfo> m_RoomList;

	bool m_IsUserListWorking = false;
	std::list<std::string> m_UserList;
};
