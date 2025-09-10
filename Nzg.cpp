
// Nzg.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "Nzg.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "NzgDoc.h"
#include "NzgTabView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNzgApp

BEGIN_MESSAGE_MAP(CNzgApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CNzgApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
	ON_COMMAND(ID_NEW_NON, &CNzgApp::OnNewNzg)
END_MESSAGE_MAP()


// CNzgApp construction

CNzgApp::CNzgApp() noexcept
{
	m_bHiColorIcons = TRUE;


	m_nAppLook = 0;
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("Nzg.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CNzgApp object

CNzgApp theApp;

void CNzgApp::showNzgView(nzg::NzgNode* node)
{
	CNzgTabView* pView = findNzgView(node);
	if (pView == nullptr)
	{
		CNzgDoc* pDoc = (CNzgDoc*)m_ptemplNzg->CreateNewDocument();
		pDoc->m_node = node;

		CFrameWnd* pFrame = m_ptemplNzg->CreateNewFrame(pDoc, NULL);
		m_ptemplNzg->InitialUpdateFrame(pFrame, pDoc);
	}
	else
	{
		CMDIChildWnd* pChild = (CMDIChildWnd*)pView->GetParentFrame();
		pChild->MDIActivate();
	}

}

CNzgTabView* CNzgApp::findNzgView(const nzg::NzgNode* pn) const
{
	POSITION pos = m_ptemplNzg->GetFirstDocPosition();
	while (pos != NULL)
	{
		CNzgDoc* pDoc = (CNzgDoc*)m_ptemplNzg->GetNextDoc(pos);
		if (pDoc->m_node == pn)
		{
			POSITION posView = pDoc->GetFirstViewPosition();
			while (posView != NULL)
			{
				CNzgTabView* pView = (CNzgTabView*)pDoc->GetNextView(posView);
				return pView;
			}
			return NULL;
		}
	}

	return NULL;
}


// CNzgApp initialization

BOOL CNzgApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_NzgTYPE,
		RUNTIME_CLASS(CNzgDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CNzgTabView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	m_ptemplNzg = pDocTemplate;

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	// Initialize screen parameters
	{
		HDC hdc = GetDC(NULL);

		int ncxScreen = GetSystemMetrics(SM_CXSCREEN);
		int ncyScreen = GetSystemMetrics(SM_CYSCREEN);
		int nHorRes = GetDeviceCaps(hdc, HORZRES);
		int hSize = GetDeviceCaps(hdc, HORZSIZE);
		int nVerRes = GetDeviceCaps(hdc, VERTRES);
		int vSize = GetDeviceCaps(hdc, VERTSIZE);

		double v = (double)nVerRes / (double)vSize;
		double h = (double)nHorRes / (double)hSize;

		int xLogPx = GetDeviceCaps(hdc, LOGPIXELSX);
		int yLogPx = GetDeviceCaps(hdc, LOGPIXELSY);
		int xLogPxMM = (int)(xLogPx / 25.4);
		int yLogPxMM = (int)(yLogPx / 25.4);


		m_sp.ppmm = (xLogPx + yLogPx) * 0.5 / 25.4;
		m_sp.ppinch = (xLogPx + yLogPx) * 0.5;
		m_sp.pixInPoint = m_sp.ppinch / 72;
		ReleaseDC(NULL, hdc);
	}

	std::srand((DWORD)std::time({}));
	return TRUE;
}

int CNzgApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CNzgApp message handlers
BOOL CNzgApp::ProcessShellCommand(CCommandLineInfo& rCmdInfo)
{
	return TRUE;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CNzgApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CNzgApp customization load/save methods

void CNzgApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CNzgApp::LoadCustomState()
{
}

void CNzgApp::SaveCustomState()
{
}

// CNzgApp message handlers
void CNzgApp::OnNewNzg()
{
	std::shared_ptr<nzg::NzgNode> p(new nzg::NzgNode());
	m_nodes.push_back(p);
	getFileView()->newNzg(p.get());
}

void CNzgApp::onViewNzg(nzg::NzgNode* pn)
{
	CNzgTabView* pView = findNzgView(pn);
	if (pView == nullptr)
	{
		CNzgDoc* pDoc = (CNzgDoc*)m_ptemplNzg->CreateNewDocument();
		pDoc->m_node = pn;

		CFrameWnd* pFrame = m_ptemplNzg->CreateNewFrame(pDoc, NULL);
		m_ptemplNzg->InitialUpdateFrame(pFrame, pDoc);
	}
	else
	{
		CMDIChildWnd* pChild = (CMDIChildWnd*)pView->GetParentFrame();
		pChild->MDIActivate();
	}


}
