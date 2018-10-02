#include <thread>
#include <chrono>

#include "../ServerNetLib/ServerNetErrorCode.h"
#include "../ServerNetLib/Define.h"
#include "../ServerNetLib/TcpNetwork.h"
#include "ConsoleLogger.h"
#include "LobbyManager.h"
#include "PacketProcess.h"
#include "UserManager.h"
#include "Main.h"

using LOG_TYPE = NServerNetLib::LOG_TYPE;
using NET_ERROR_CODE = NServerNetLib::NET_ERROR_CODE;

namespace NLogicLib
{
	Main::Main()
	{
	}

	Main::~Main()
	{
		Release();
	}

	ERROR_CODE Main::Init()
	{
		m_pLogger = std::make_unique<ConsoleLog>();

		LoadConfig();

		m_pNetwork = std::make_unique<NServerNetLib::TcpNetwork>();
		auto result = m_pNetwork->Init(m_pServerConfig.get(), m_pLogger.get());
			
		if (result != NET_ERROR_CODE::NONE)
		{
			m_pLogger->Write(LOG_TYPE::L_ERROR, "%s | Init Fail. NetErrorCode(%s)", __FUNCTION__, (short)result);
			return ERROR_CODE::MAIN_INIT_NETWORK_INIT_FAIL;
		}

		
		m_pUserMgr = std::make_unique<UserManager>();
		m_pUserMgr->Init(m_pServerConfig->MaxClientCount);

		m_pLobbyMgr = std::make_unique<LobbyManager>();
		m_pLobbyMgr->Init({ m_pServerConfig->MaxLobbyCount, 
							m_pServerConfig->MaxLobbyUserCount,
							m_pServerConfig->MaxRoomCountByLobby, 
							m_pServerConfig->MaxRoomUserCount },
						m_pNetwork.get(), m_pLogger.get());

		m_pPacketProc = std::make_unique<PacketProcess>();
		m_pPacketProc->Init(m_pNetwork.get(), m_pUserMgr.get(), m_pLobbyMgr.get(), m_pServerConfig.get(), m_pLogger.get());

		m_IsRun = true;

		m_pLogger->Write(LOG_TYPE::L_INFO, "%s | Init Success. Server Run", __FUNCTION__);
		return ERROR_CODE::NONE;
	}

	void Main::Release() 
	{
		if (m_pNetwork) {
			m_pNetwork->Release();
		}
	}

	void Main::Stop()
	{
		m_IsRun = false;
	}

	void Main::Run()
	{
		while (m_IsRun)
		{
			m_pNetwork->Run();

			while (true)
			{				
				auto packetInfo = m_pNetwork->GetPacketInfo();

				if (packetInfo.PacketId == 0)
				{
					break;
				}
				else
				{
					m_pPacketProc->Process(packetInfo);
				}
			}

			m_pPacketProc->StateCheck();
		}
	}

	ERROR_CODE Main::LoadConfig()
	{
		m_pServerConfig = std::make_unique<NServerNetLib::ServerConfig>();
		
		wchar_t sPath[MAX_PATH] = { 0, };
		::GetCurrentDirectory(MAX_PATH, sPath);

		wchar_t inipath[MAX_PATH] = { 0, };
		_snwprintf_s(inipath, _countof(inipath), _TRUNCATE, L"%s\\ServerConfig.ini", sPath);

		m_pServerConfig->Port = (unsigned short)GetPrivateProfileInt(L"Config", L"Port", 0, inipath);
		m_pServerConfig->BackLogCount = GetPrivateProfileInt(L"Config", L"BackLogCount", 0, inipath);
		m_pServerConfig->MaxClientCount = GetPrivateProfileInt(L"Config", L"MaxClientCount", 0, inipath);

		m_pServerConfig->MaxClientSockOptRecvBufferSize = (short)GetPrivateProfileInt(L"Config", L"MaxClientSockOptRecvBufferSize", 0, inipath);
		m_pServerConfig->MaxClientSockOptSendBufferSize = (short)GetPrivateProfileInt(L"Config", L"MaxClientSockOptSendBufferSize", 0, inipath);
		m_pServerConfig->MaxClientRecvBufferSize = (short)GetPrivateProfileInt(L"Config", L"MaxClientRecvBufferSize", 0, inipath);
		m_pServerConfig->MaxClientSendBufferSize = (short)GetPrivateProfileInt(L"Config", L"MaxClientSendBufferSize", 0, inipath);

		m_pServerConfig->IsLoginCheck = GetPrivateProfileInt(L"Config", L"IsLoginCheck", 0, inipath) == 1 ? true : false;

		m_pServerConfig->ExtraClientCount = GetPrivateProfileInt(L"Config", L"ExtraClientCount", 0, inipath);
		m_pServerConfig->MaxLobbyCount = GetPrivateProfileInt(L"Config", L"MaxLobbyCount", 0, inipath);
		m_pServerConfig->MaxLobbyUserCount = GetPrivateProfileInt(L"Config", L"MaxLobbyUserCount", 0, inipath);
		m_pServerConfig->MaxRoomCountByLobby = GetPrivateProfileInt(L"Config", L"MaxRoomCountByLobby", 0, inipath);
		m_pServerConfig->MaxRoomUserCount = GetPrivateProfileInt(L"Config", L"MaxRoomUserCount", 0, inipath);

		m_pLogger->Write(NServerNetLib::LOG_TYPE::L_INFO, "%s | Port(%d), Backlog(%d)", __FUNCTION__, m_pServerConfig->Port, m_pServerConfig->BackLogCount);
		m_pLogger->Write(NServerNetLib::LOG_TYPE::L_INFO, "%s | IsLoginCheck(%d)", __FUNCTION__, m_pServerConfig->IsLoginCheck);
		return ERROR_CODE::NONE;
	}
		
}