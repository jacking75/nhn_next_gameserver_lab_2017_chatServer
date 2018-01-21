
#include "TcpNetwork.h"
#include "ClientSceenLogIn.h"
#include "ClientSceenSelLobby.h"
#include "ClientSceenLobby.h"
#include "MainForm.h"


using PACKET_ID = NCommon::PACKET_ID;


MainForm::MainForm() {}

MainForm::~MainForm() 
{
	if (m_Network)
	{
		m_Network->DisConnect();
	}
}

void MainForm::Init()
{
	m_Network = std::make_unique<TcpNetwork>();

	m_pClientSceenLogIn = std::make_shared<ClientSceenLogIn>();
	m_pClientSceenLogIn->SetNetwork(m_Network.get());

	m_pClientSceenSelLobby = std::make_shared<ClientSceenSelLobby>();
	m_pClientSceenSelLobby->SetNetwork(m_Network.get());

	m_pClientSceenLobby = std::make_shared<ClientSceenLobby>();
	m_pClientSceenLobby->SetNetwork(m_Network.get());
}

void MainForm::CreateGUI()
{
	// https://moqups.com/   여기에서 디자인 하자

	m_fm = std::make_unique<form>(API::make_center(900, 700));
	m_fm->caption("Chat Client");

	m_pClientSceenLogIn->CreateUI(m_fm.get());
	
	m_pClientSceenSelLobby->CreateUI(m_fm.get());

	m_pClientSceenLobby->CreateUI(m_fm.get());

	
	m_ptxtCurState = std::make_unique<textbox>((form&)*m_fm.get(), nana::rectangle(450, 15, 120, 20));
	m_ptxtCurState->caption("State: Disconnect");
	
	m_RoomUserList = std::make_shared<listbox>((form&)*m_fm.get(), nana::rectangle(22, 522, 120, 166));
	m_RoomUserList->append_header("UserID", 90);

	m_timer.elapse([&]() { PacketProcess();});
	m_timer.interval(32);
	m_timer.start();
}

void MainForm::ShowModal()
{
	m_fm->show();

	exec();
}

void MainForm::PacketProcess()
{
	if (!m_Network) {
		return;
	}

	
	auto packet = m_Network->GetPacket();

	if (packet.PacketId != 0)
	{
		m_pClientSceenLogIn->ProcessPacket(packet.PacketId, packet.pData);
		m_pClientSceenSelLobby->ProcessPacket(packet.PacketId, packet.pData);
		m_pClientSceenLobby->ProcessPacket(packet.PacketId, packet.pData);

		if (packet.pData != nullptr) {
			delete[] packet.pData;
		}
	}
	

	m_pClientSceenLogIn->Update();
	m_pClientSceenSelLobby->Update();
	m_pClientSceenLobby->Update();
}

