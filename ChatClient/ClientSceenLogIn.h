#pragma once

#include "TcpNetwork.h"
#include "IClientSceen.h"


class ClientSceenLogIn : public IClientSceen
{
public:
	ClientSceenLogIn() {}
	virtual ~ClientSceenLogIn() {}

	virtual void Update() override
	{
		if (GetCurSceenType() != CLIENT_SCEEN_TYPE::CONNECT) {
			return;
		}
	}

	void CreateUI(form* pform)
	{
		m_pForm = pform;

		m_lbl1 = std::make_shared<label>((form&)*m_pForm, nana::rectangle(22, 17, 18, 18));
		m_lbl1->caption("IP:");
		m_IPtxt = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(43, 15, 128, 20));
		m_IPtxt->caption("127.0.0.1");

		m_lbl2 = std::make_shared<label>((form&)*m_pForm, nana::rectangle(187, 17, 30, 18));
		m_lbl2->caption("Port:");
		m_Porttxt = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(214, 15, 60, 20));
		m_Porttxt->caption("23452");

		m_Connectbtn = std::make_shared<button>((form&)*m_pForm, nana::rectangle(283, 14, 102, 23));
		m_Connectbtn->caption("Connent");
		m_Connectbtn->events().click([&]() {
			this->ConnectOrDisConnect();
		});


		m_lbl3 = std::make_shared<label>((form&)*m_pForm, nana::rectangle(22, 58, 18, 18));
		m_lbl3->caption("ID:");
		m_IDtxt = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(43, 56, 115, 20));
		m_IDtxt->caption("jacking");

		m_lbl4 = std::make_shared<label>((form&)*m_pForm, nana::rectangle(170, 58, 69, 18));
		m_lbl4->caption("PassWord:");
		m_PWtxt = std::make_shared<textbox>((form&)*m_pForm, nana::rectangle(230, 56, 115, 20));
		m_PWtxt->caption("1234");

		m_Loginbtn = std::make_shared<button>((form&)*m_pForm, nana::rectangle(353, 54, 102, 23));
		m_Loginbtn->caption("Login");
		m_Loginbtn->events().click([&]() {
			this->LogInOut();
		});
		m_Loginbtn->enabled(false);
	}


	bool ProcessPacket(const short packetId, char* pData) override
	{
		switch (packetId)
		{
		case (short)PACKET_ID::LOGIN_IN_RES:
			{
				m_Loginbtn->enabled(true);

				auto pktRes = (NCommon::PktLogInRes*)pData;

				if (pktRes->ErrorCode == (short)NCommon::ERROR_CODE::NONE)
				{
					m_Loginbtn->caption("LogOut");
					SetCurSceenType(CLIENT_SCEEN_TYPE::LOGIN);
				}
				else
				{
					nana::msgbox m((form&)*m_pForm, "Fail LOGIN_IN_REQ", nana::msgbox::ok);
					m.icon(m.icon_warning);
					m << "ErrorCode: " << pktRes->ErrorCode;
					m.show();
				}
			}
			break;
		default:
			return false;
		}
		
		return true;
	}

private:
	void ConnectOrDisConnect()
	{
		m_IsLogined = false;

		if (m_pRefNetwork->IsConnected() == false)
		{
			char szIP[64] = { 0, };
			UnicodeToAnsi(m_IPtxt->caption_wstring().c_str(), 64, szIP);

			if (m_pRefNetwork->ConnectTo(szIP, (unsigned short)m_Porttxt->to_int()))
			{
				m_Connectbtn->caption("DisConnect");
				m_Loginbtn->enabled(true);
			}
			else
			{
				nana::msgbox m((form&)*m_pForm, "Fail Connect", nana::msgbox::ok);
				m.icon(m.icon_warning);
				m.show();

				m_Loginbtn->enabled(false);
			}
		}
		else
		{
			m_pRefNetwork->DisConnect();
			
			m_Connectbtn->caption("Connect");
			m_Loginbtn->enabled(false);
		}

	}

	void LogInOut()
	{
		if (m_IsLogined == false)
		{
			char szID[64] = { 0, };
			UnicodeToAnsi(m_IDtxt->caption_wstring().c_str(), 64, szID);

			char szPW[64] = { 0, };
			UnicodeToAnsi(m_PWtxt->caption_wstring().c_str(), 64, szPW);

			NCommon::PktLogInReq reqPkt;
			strncpy_s(reqPkt.szID, NCommon::MAX_USER_ID_SIZE + 1, szID, NCommon::MAX_USER_ID_SIZE);
			strncpy_s(reqPkt.szPW, NCommon::MAX_USER_PASSWORD_SIZE + 1, szPW, NCommon::MAX_USER_PASSWORD_SIZE);

			m_pRefNetwork->SendPacket((short)PACKET_ID::LOGIN_IN_REQ, sizeof(reqPkt), (char*)&reqPkt);

			m_Loginbtn->enabled(false);
		}
		else
		{
			nana::msgbox m((form&)*m_pForm, "Unimplemented", nana::msgbox::ok);
			m.icon(m.icon_warning);
			m.show();
		}
	}


	bool m_IsLogined = false;

	form* m_pForm = nullptr;

	std::shared_ptr<label> m_lbl1;
	std::shared_ptr<textbox> m_IPtxt;

	std::shared_ptr<label> m_lbl2;
	std::shared_ptr<textbox> m_Porttxt;

	std::shared_ptr<button> m_Connectbtn;


	std::shared_ptr<label> m_lbl3;
	std::shared_ptr<textbox> m_IDtxt;

	std::shared_ptr<label> m_lbl4;
	std::shared_ptr<textbox> m_PWtxt;

	std::shared_ptr<button> m_Loginbtn;
};