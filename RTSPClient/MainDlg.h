// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "rtsp_client.hpp"
#include "Markup.h"
#include <vector>

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
    public CMessageFilter, public CIdleHandler
{
    std::vector< std::string > m_vrtspurl;
    std::vector< CRtspClient * > m_vrtsp;

    CRtspClient m_rtsp;
    std::map< std::string, std::string > m_mapNaming;

public:
    enum { IDD = IDD_MAINDLG };

    virtual BOOL PreTranslateMessage(MSG *pMsg)
    {
        return CWindow::IsDialogMessage(pMsg);
    }

    virtual BOOL OnIdle()
    {
        return FALSE;
    }

    BEGIN_UPDATE_UI_MAP(CMainDlg)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CMainDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
    COMMAND_ID_HANDLER(IDOK, OnOK)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_BUTTON1, OnButton1)
    END_MSG_MAP()

    // Handler prototypes (uncomment arguments if needed):
    //  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        // center the dialog on the screen
        CenterWindow();

        // set icons
        HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),
                                         IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
        SetIcon(hIcon, TRUE);
        HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),
                                              IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
        SetIcon(hIconSmall, FALSE);

        // register object for message filtering and idle updates
        CMessageLoop *pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);

        UIAddChildWindowContainer(m_hWnd);

        CMarkup xml;
        xml.Load("channel.xml");

        xml.FindElem("ROWDATA");
        xml.IntoElem();
        while (xml.FindElem("ROW")) {
            std::string sID, sNaming;
            xml.IntoElem();
            while (xml.FindElem()) {
                std::string sTag = xml.GetTagName();
                if (sTag == "STD_ID") sID = xml.GetData();
                else if (sTag == "NAMING") sNaming = xml.GetData();
            }
            m_mapNaming.insert(std::make_pair(sID, sNaming));
            xml.OutOfElem();
        }



        const char *s = "rtsp://192.168.199.153:554/service?ChanelNo-PUID=0000000000200000000000000200001:0000000000200000000000000150004:192.168.30.118:010000&PlayMethod=0&StreamingType=2";
        m_rtsp.Open(s, NULL);

        SetDlgItemText(IDC_EDIT3, "rtsp://192.168.199.153:554/service?ChanelNo-PUID=%s&PlayMethod=0&StreamingType=2");

        return TRUE;
    }

    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        // unregister message filtering and idle updates
        CMessageLoop *pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->RemoveMessageFilter(this);
        pLoop->RemoveIdleHandler(this);

        return 0;
    }

    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        CAboutDlg dlg;
        dlg.DoModal();
        return 0;
    }

    LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        CMarkup xml;
        xml.Load("rtsppuid.xml");

        char s1[32] = {0};
        char s2[32] = {0};
        char sFormat[2048] = {0};

        GetDlgItemText(IDC_EDIT1, s1, 31);
        GetDlgItemText(IDC_EDIT2, s2, 31);
        GetDlgItemText(IDC_EDIT3, sFormat, 2047);

        long lStart = atol(s1);
        long lEnd = atol(s2);

        long lCount = 0;
        xml.FindElem("ROWDATA");
        xml.IntoElem();
        while (xml.FindElem("ROW")) {
            lCount++;
            if (lCount < lStart)
                continue;
            if (lCount > lEnd)
                break;

            xml.IntoElem();
            xml.FindElem("STD_ID");

            char sBuffer[512] = {0};
            sprintf_s(sBuffer, 511, sFormat, this->m_mapNaming[ xml.GetData()].c_str());

            CRtspClient *pRtsp = new CRtspClient;
            //  pRtsp->Open( sBuffer, NULL );
            m_vrtsp.push_back(pRtsp);
            m_vrtspurl.push_back(sBuffer);

            xml.OutOfElem();
        }

        SetTimer(1111, 1000, NULL);

        return 0;
    }

    LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        static unsigned long lCount = 0UL;

        if (lCount < m_vrtsp.size()) {
            m_vrtsp[ lCount ]->Open(m_vrtspurl[ lCount ].c_str(), NULL);
            lCount++;
        }
        return 0;
    }


    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        CloseDialog(wID);
        return 0;
    }

    LRESULT OnButton1(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        CMarkup xml;
        xml.Load("rtsppuid.xml");

        long lCount = 0;
        xml.FindElem("ROWDATA");
        xml.IntoElem();
        while (xml.FindElem("ROW")) {
            lCount++;
        }

        SetDlgItemInt(IDC_STATIC_1, lCount);


        return 0;
    }

    void CloseDialog(int nVal)
    {
        std::vector< CRtspClient * >::iterator pos;
        for (pos = m_vrtsp.begin(); pos != m_vrtsp.end(); pos++) {
            (*pos)->TearDown();
            delete *pos;
        }

        DestroyWindow();
        ::PostQuitMessage(nVal);
    }
};
