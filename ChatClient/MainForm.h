#pragma once
#include <memory>
#include <nana/gui/wvl.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui.hpp>


#ifdef NDEBUG
#pragma comment(lib, "nana_v141_Release_x64.lib")
#else
#pragma comment(lib, "nana_v141_Debug_x64.lib")
#endif

using namespace nana;

class TcpNetwork;

class IClientSceen;
class ClientSceenLogIn;
class ClientSceenSelLobby;
class ClientSceenLobby;


class MainForm
{
public:
	MainForm();
	~MainForm();

	void Init();

	void CreateGUI();

	void ShowModal();


private:
	void PacketProcess();
	
private:
	std::unique_ptr<TcpNetwork> m_Network;

	bool m_IsLogined = false;


private:
	std::unique_ptr<form> m_fm;

	timer m_timer;

	std::unique_ptr<textbox> m_ptxtCurState;
	
	std::shared_ptr<listbox> m_RoomUserList;

	std::shared_ptr<ClientSceenLogIn> m_pClientSceenLogIn;
	std::shared_ptr<ClientSceenSelLobby> m_pClientSceenSelLobby;
	std::shared_ptr<ClientSceenLobby> m_pClientSceenLobby;
};