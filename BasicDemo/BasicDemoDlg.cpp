
// BasicDemoDlg.cpp : implementation file
#include "stdafx.h"
#include "BasicDemo.h"
#include "BasicDemoDlg.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "CvvImage.h"
#include "opencv2/video/tracking_c.h"
#include <locale> 
#include <atlimage.h>
#include <iostream>  
#include "SoftKey.h"
#include <afxwin.h>
#include <Dbt.h>
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;
// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{

	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CBasicDemoDlg dialog
CBasicDemoDlg::CBasicDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBasicDemoDlg::IDD, pParent)
    , m_pcMyCamera(NULL)
    , m_nDeviceCombo(0)
    , m_bOpenDevice(FALSE)
    , m_bStartGrabbing(FALSE)
    , m_hGrabThread(NULL)
    , m_bThreadState(FALSE)
    , m_nTriggerMode(MV_TRIGGER_MODE_OFF)
    , m_dExposureEdit(0)
    , m_dGainEdit(0)
    , m_dFrameRateEdit(0)
    , m_bSoftWareTriggerCheck(FALSE)
    , m_nSaveImageType(MV_Image_Undefined)
    , m_nTriggerSource(MV_TRIGGER_SOURCE_SOFTWARE)
    , m_pSaveImageBuf(NULL)
    , m_nSaveImageBufSize(0)
    , m_pGrabBuf(NULL)
    , m_nGrabBufSize(0)
    , use_time(_T(""))
    , m_Threshold(30)
    , m_maxvalue(200)
    , m_editDelay(_T("0"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
}

void CBasicDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
    DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
    DDX_Text(pDX, IDC_EXPOSURE_EDIT, m_dExposureEdit);
    DDX_Text(pDX, IDC_GAIN_EDIT, m_dGainEdit);
    DDX_Text(pDX, IDC_FRAME_RATE_EDIT, m_dFrameRateEdit);
    DDX_Check(pDX, IDC_SOFTWARE_TRIGGER_CHECK, m_bSoftWareTriggerCheck);
    DDX_Text(pDX, IDC_STATIC_time, use_time);
    DDX_Text(pDX, IDC_EDIT_Threshold, m_Threshold);
    DDV_MinMaxInt(pDX, m_Threshold, 0, 255);
    DDX_Text(pDX, IDC_EDIT_maxvalue, m_maxvalue);
    DDV_MinMaxDouble(pDX, m_maxvalue, 0, 255);
    DDX_Control(pDX, IDC_STATIC_video, m_videoWnd);
    DDX_Text(pDX, IDC_EDIT1, m_editDelay);
    DDX_Control(pDX, IDC_SPIN1, m_SpinDelay);
    DDX_Control(pDX, IDC_DISPLAY_STATIC, m_diff_display);
}

BEGIN_MESSAGE_MAP(CBasicDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	// }}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ENUM_BUTTON, &CBasicDemoDlg::OnBnClickedEnumButton)
    ON_BN_CLICKED(IDC_OPEN_BUTTON, &CBasicDemoDlg::OnBnClickedOpenButton)
    ON_BN_CLICKED(IDC_CLOSE_BUTTON, &CBasicDemoDlg::OnBnClickedCloseButton)
    ON_BN_CLICKED(IDC_CONTINUS_MODE_RADIO, &CBasicDemoDlg::OnBnClickedContinusModeRadio)
    ON_BN_CLICKED(IDC_TRIGGER_MODE_RADIO, &CBasicDemoDlg::OnBnClickedTriggerModeRadio)
    ON_BN_CLICKED(IDC_START_GRABBING_BUTTON, &CBasicDemoDlg::OnBnClickedStartGrabbingButton)
    ON_BN_CLICKED(IDC_STOP_GRABBING_BUTTON, &CBasicDemoDlg::OnBnClickedStopGrabbingButton)
    ON_BN_CLICKED(IDC_GET_PARAMETER_BUTTON, &CBasicDemoDlg::OnBnClickedGetParameterButton)
    ON_BN_CLICKED(IDC_SET_PARAMETER_BUTTON, &CBasicDemoDlg::OnBnClickedSetParameterButton)
    ON_BN_CLICKED(IDC_SOFTWARE_TRIGGER_CHECK, &CBasicDemoDlg::OnBnClickedSoftwareTriggerCheck)
    ON_BN_CLICKED(IDC_SOFTWARE_ONCE_BUTTON, &CBasicDemoDlg::OnBnClickedSoftwareOnceButton)
  /*  ON_BN_CLICKED(IDC_SAVE_BMP_BUTTON, &CBasicDemoDlg::OnBnClickedSaveBmpButton)
    ON_BN_CLICKED(IDC_SAVE_JPG_BUTTON, &CBasicDemoDlg::OnBnClickedSaveJpgButton)*/
    ON_WM_CLOSE()
 /*   ON_BN_CLICKED(IDC_SAVE_TIFF_BUTTON, &CBasicDemoDlg::OnBnClickedSaveTiffButton)
    ON_BN_CLICKED(IDC_SAVE_PNG_BUTTON, &CBasicDemoDlg::OnBnClickedSavePngButton)*/
    ON_BN_CLICKED(IDC_BUTTON_DIFF, &CBasicDemoDlg::OnBnClickedButtonDiff)
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BUTTON_save, &CBasicDemoDlg::OnBnClickedButtonsave)
    ON_BN_CLICKED(IDC_BUTTON_Threshold, &CBasicDemoDlg::OnBnClickedButtonThreshold)
    ON_BN_CLICKED(IDC_BUTTON_maxvalue, &CBasicDemoDlg::OnBnClickedButtonmaxvalue)
    ON_BN_CLICKED(IDC_BUTTON_toleft, &CBasicDemoDlg::OnBnClickedButtontoleft)
    ON_BN_CLICKED(IDC_BUTTON_toright, &CBasicDemoDlg::OnBnClickedButtontoright)
    ON_WM_LBUTTONDBLCLK()
    ON_STN_DBLCLK(IDC_STATIC_video, &CBasicDemoDlg::OnDblclkStaticVideo)
    ON_STN_DBLCLK(IDC_DISPLAY_STATIC, &CBasicDemoDlg::OnDblclkDisplayStatic)
    ON_WM_GETMINMAXINFO()


    ON_WM_MOUSEWHEEL()
 //   ON_WM_NCLBUTTONDBLCLK(IDC_STATIC_video, &CBasicDemoDlg::OnPicNcLButtonDblClk)
	//ON_WM_LBUTTONDOWN(IDC_STATIC_video, &CBasicDemoDlg::OnPicLButtonDown)
	//ON_WM_LBUTTONUP(IDC_STATIC_video, &CBasicDemoDlg::OnPicLButtonUp)
	//ON_WM_MOUSEMOVE(IDC_STATIC_video, &CBasicDemoDlg::OnPicMouseMove)
	//ON_WM_ERASEBKGND(IDC_STATIC_video, & CBasicDemoDlg::OnPicEraseBkgnd)
#ifdef SETROI

#endif
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CBasicDemoDlg::OnDeltaposSpin1)
    ON_EN_CHANGE(IDC_EDIT1, &CBasicDemoDlg::OnChangeEdit1)
//    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDOWN()
    ON_BN_CLICKED(IDC_BUTTON_move_up, &CBasicDemoDlg::OnBnClickedButtonmoveup)
    ON_BN_CLICKED(IDC_BUTTON_move_left, &CBasicDemoDlg::OnBnClickedButtonmoveleft)
    ON_BN_CLICKED(IDC_BUTTON_move_right, &CBasicDemoDlg::OnBnClickedButtonmoveright)
    ON_BN_CLICKED(IDC_BUTTON_move_down, &CBasicDemoDlg::OnBnClickedButtonmovedown)
END_MESSAGE_MAP()

// ch:ȡ���߳� | en:Grabbing thread
unsigned int __stdcall GrabThread(void* pUser)
{
    if (NULL == pUser)
    {
        return -1;
    }
    try {

  
    CBasicDemoDlg* pCam = (CBasicDemoDlg*)pUser;

    pCam->GrabThreadProcess();
    }
    catch(Exception ex)
    {
       // MessageBox(errorMsg, TEXT("PROMPT"), MB_OK | MB_ICONWARNING);
        CBasicDemoDlg lg = new CBasicDemoDlg();
        CString cstr(ex.msg.c_str());
       lg.WriteLog(L"ERROR:  " +  cstr );
    }
    return 0;
}

// CBasicDemoDlg message handlers
BOOL CBasicDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
   
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);


    ReadIni();
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
    //CWnd* pControl = GetDlgItem(IDC_STATIC_video);
    //pControl->GetWindowRect(&rect_view);    //����ӿؼ�����Ļ���ꣻ     
    //this->ScreenToClient(&rect_view);   //�ӿؼ���Ļ����ӳ�䵽�ؼ��ͻ�����������һ��Ҫ��this
    //InvalidateRect(&rect_view, TRUE);  //ˢ�¿ؼ�����
   
    //m_picHeight = rect_view.bottom - rect_view.top;
   // m_picWeight = rect_view.right - rect_view.left;

    m_bLBDown = false;


  
    //rect_rect = rect_view;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	DisplayWindowInitial();     // ch:��ʾ���ʼ�� | en:Display Window Initialization
    m_draw_rect = FALSE;
    InitializeCriticalSection(&m_hSaveImageMux);
    CRect rect;
    GetClientRect(&rect);//ȡ�ͻ�����С
    Old.x = rect.right - rect.left;
    Old.y = rect.bottom - rect.top;
    //�����ӳ�����
	int itemp = _ttoi(m_editDelay);
	m_SpinDelay.SetBuddy(GetDlgItem(itemp));
    m_SpinDelay.SetRange(0, 1000000);
    //m_edt_DelayValue = 0;
    //������ֵ
    m_Threshold = 30;
    m_maxvalue = 200;
    bFullScreen = false;
    //��������
    static CFont Bfont;
    Bfont.CreatePointFont(300, _T("����"));
    GetDlgItem(IDC_BUTTON_showerror)->SetFont(&Bfont);
    //��ѯ���ܹ�
 ///////////////////////////////////////////////////////////////////////////////////	
     //��������ж�ϵͳ���Ƿ�����ż�����������Ҫ��ָ���ļ�����,
    if (ytsoftkey.FindPort(0, DevicePath) != 0)
    {
        MessageBox(L"δ�ҵ�������,�������������ٽ��в�����", TEXT("����"), MB_OK | MB_ICONERROR);
        PostQuitMessage(0);
    }
    if (!DoRegisterDeviceInterface())
        MessageBox(L"ע���¼�֪ͨʧ�ܡ�", TEXT("����"), MB_OK | MB_ICONERROR );//'ע��������¼��岦֪ͨ
//////////////////////////////////////////////////////////////////////////////////////
    ChickKey();
    ShowWindow(SW_SHOWMAXIMIZED);  //�����ʾ����
	//m_hAccel_add = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE("IDR_ACCELERATOR1"));
    m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));
	//m_hAccel_sub = LoadAccelerators(AfxGetResourceHandle(), MAKEINTRESOURCE(L"ID_subtract"));

    OnBnClickedEnumButton();

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CBasicDemoDlg::ChickKey()
{
    //ʹ����ͨ�㷨һ������Ƿ����ָ���ļ�����
    if (ytsoftkey.CheckKeyByFindort_2() == 0 && ytsoftkey.CheckKeyByReadEprom() ==0 && ytsoftkey.CheckKeyByEncstring() == 0 && ytsoftkey.CheckKeyByEncstring_New() ==0)
        WriteLog(L"�ɹ��ҵ���������");
        //MessageBox(L"�ҵ�ָ���ļ�����", TEXT("����"), MB_OK | MB_ICONINFORMATION);
    else
    {
        MessageBox(L"δ���ҵ�ָ���ļ�����", TEXT("����"), MB_OK | MB_ICONERROR);
        exit(0);
    }

   
}
/*
LPCTSTR lpAppName; //��INI�ļ��е�һ���ֶ���.

LPCTSTR lpKeyName;//��lpAppName�µ�һ������,ͨ�׽����Ǳ�����.

INT nDefault; //���INI�ļ���û��ǰ��������ָ�����ֶ��������,�򽫴�ֵ��������.

LPCTSTR lpFileName;//��������INI�ļ���.
*/
void CBasicDemoDlg::ReadIni()
{
    CString str;
    CFileFind finder;   //�����Ƿ����ini�ļ����������ڣ�������һ���µ�Ĭ�����õ�ini�ļ��������ͱ�֤�����Ǹ��ĺ������ÿ�ζ�����  
    BOOL ifFind = finder.FindFile(_T("config.ini"));
    if (!ifFind)
    {
        MessageBox(L"no config.ini file");
    }
    GetPrivateProfileString(_T("config"), _T("LOG"), CString("-1"), str.GetBuffer(MAX_PATH), MAX_PATH, _T(".//config.ini"));    
  
    m_WriteLog = (str == "1")?TRUE:FALSE;
  
}
BOOL CBasicDemoDlg::DoRegisterDeviceInterface()
{
    HDEVNOTIFY hDevNotify;
    GUID hid_guid = { 0x4D1E55B2,0xF16F,0x11CF,0x88,0xCB,0x00,0x11,0x11,0x00,0x00,0x30
    };
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
    NotificationFilter.dbcc_size =
        sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = hid_guid;
    hDevNotify = RegisterDeviceNotification(m_hWnd,
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE
    );

    if (!hDevNotify)
    {
        return FALSE;
    }

    return TRUE;
}
void CBasicDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CBasicDemoDlg::OnPaint()
{
    CRect prect;
	m_videoWnd.GetClientRect(&prect);   //��ȡ����
	FillRect(m_videoWnd.GetDC()->GetSafeHdc(), &prect, CBrush(RGB(50, 50, 50)));  //����ͼ������Ϊ��ɫ
    m_diff_display.GetClientRect(&prect);
    FillRect(m_diff_display.GetDC()->GetSafeHdc(), &prect, CBrush(RGB(50, 50, 50)));  //����ͼ������Ϊ��ɫ
	
 //   CPaintDC dc(this);
	////���Ʊ���   
	//CRect rect;
	//GetClientRect(&rect);

	//CBrush bruDB(RGB(255, 255, 255));//������ɫ  

	//dc.FillRect(&rect, &bruDB);

	////�����϶�����   
	//if (m_bLBDown)
	//{
	//	CRect rect(m_StartPoint, m_EndPoint);

	//	rect.NormalizeRect();//�淶������   

	//	CBrush bruPen(RGB(255, 0, 0));//���α߿���ɫ  

	//	dc.FrameRect(&rect, &bruPen);
	//}

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
       
      
		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBasicDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ch:��ťʹ�� | en:Enable control
int CBasicDemoDlg::EnableControls(BOOL bIsCameraReady)
{
    GetDlgItem(IDC_OPEN_BUTTON)->EnableWindow(m_bOpenDevice ? FALSE : (bIsCameraReady ? TRUE : FALSE));
    GetDlgItem(IDC_CLOSE_BUTTON)->EnableWindow((m_bOpenDevice && bIsCameraReady) ? TRUE : FALSE);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->EnableWindow((m_bStartGrabbing && bIsCameraReady) ? FALSE : (m_bOpenDevice ? TRUE : FALSE));
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SOFTWARE_TRIGGER_CHECK)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow((m_bStartGrabbing && m_bSoftWareTriggerCheck && ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->GetCheck())? TRUE : FALSE);
    /*GetDlgItem(IDC_SAVE_BMP_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_TIFF_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_PNG_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_JPG_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);*/
    GetDlgItem(IDC_EXPOSURE_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_GAIN_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_FRAME_RATE_EDIT)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_GET_PARAMETER_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_SET_PARAMETER_BUTTON)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_CONTINUS_MODE_RADIO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);
    GetDlgItem(IDC_TRIGGER_MODE_RADIO)->EnableWindow(m_bOpenDevice ? TRUE : FALSE);

    GetDlgItem(IDC_BUTTON_Threshold)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_BUTTON_maxvalue)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_BUTTON_save)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_BUTTON_DIFF)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_BUTTON_toleft)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_BUTTON_toright)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_EDIT_Threshold)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_EDIT_maxvalue)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_EDIT1)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SPIN1)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    
        
    return MV_OK;
}

// ch:�ʼʱ�Ĵ��ڳ�ʼ�� | en:Initial window initialization
int CBasicDemoDlg::DisplayWindowInitial(void)
{
    CWnd *pWnd = GetDlgItem(IDC_DISPLAY_STATIC);
    if (NULL == pWnd)
    {
        return STATUS_ERROR;
    }
    m_hwndDisplay = pWnd->GetSafeHwnd();
    if (NULL == m_hwndDisplay)
    {
        return STATUS_ERROR;
    }
    EnableControls(FALSE);

    return MV_OK;
}

// ch:��ʾ������Ϣ | en:Show error message
void CBasicDemoDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
{
    CString errorMsg;
    if (nErrorNum == 0)
    {
        errorMsg.Format(_T("%s"), csMessage);
    }
    else
    {
        errorMsg.Format(_T("%s: Error = %x: "), csMessage, nErrorNum);
    }

    switch(nErrorNum)
    {
    case MV_E_HANDLE:           errorMsg += "Error or invalid handle ";                                         break;
    case MV_E_SUPPORT:          errorMsg += "Not supported function ";                                          break;
    case MV_E_BUFOVER:          errorMsg += "Cache is full ";                                                   break;
    case MV_E_CALLORDER:        errorMsg += "Function calling order error ";                                    break;
    case MV_E_PARAMETER:        errorMsg += "Incorrect parameter ";                                             break;
    case MV_E_RESOURCE:         errorMsg += "Applying resource failed ";                                        break;
    case MV_E_NODATA:           errorMsg += "No data ";                                                         break;
    case MV_E_PRECONDITION:     errorMsg += "Precondition error, or running environment changed ";              break;
    case MV_E_VERSION:          errorMsg += "Version mismatches ";                                              break;
    case MV_E_NOENOUGH_BUF:     errorMsg += "Insufficient memory ";                                             break;
    case MV_E_ABNORMAL_IMAGE:   errorMsg += "Abnormal image, maybe incomplete image because of lost packet ";   break;
    case MV_E_UNKNOW:           errorMsg += "Unknown error ";                                                   break;
    case MV_E_GC_GENERIC:       errorMsg += "General error ";                                                   break;
    case MV_E_GC_ACCESS:        errorMsg += "Node accessing condition error ";                                  break;
    case MV_E_ACCESS_DENIED:	errorMsg += "No permission ";                                                   break;
    case MV_E_BUSY:             errorMsg += "Device is busy, or network disconnected ";                         break;
    case MV_E_NETER:            errorMsg += "Network error ";                                                   break;
    }

    MessageBox(errorMsg, TEXT("PROMPT"), MB_OK | MB_ICONWARNING);
}

// ch:���豸 | en:Open Device
int CBasicDemoDlg::OpenDevice(void)
{
    if (TRUE == m_bOpenDevice)
    {
        return STATUS_ERROR;
    }
    UpdateData(TRUE);

    int nIndex = m_nDeviceCombo;
    if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
    {
        ShowErrorMsg(TEXT("Please select device"), 0);
        return STATUS_ERROR;
    }

    // ch:���豸��Ϣ�����豸ʵ�� | en:Device instance created by device information
    if (NULL == m_stDevList.pDeviceInfo[nIndex])
    {
        ShowErrorMsg(TEXT("Device does not exist"), 0);
        return STATUS_ERROR;
    }

    if (NULL != m_pcMyCamera)
    {
        return STATUS_ERROR;
    }

    m_pcMyCamera = new CMvCamera;
    if (NULL == m_pcMyCamera)
    {
        return STATUS_ERROR;
    }

    int nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet)
    {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        return nRet;
    }

    // ch:̽��������Ѱ���С(ֻ��GigE�����Ч) | en:Detection network optimal package size(It only works for the GigE camera)
    if (m_stDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {
        unsigned int nPacketSize = 0;
        nRet = m_pcMyCamera->GetOptimalPacketSize(&nPacketSize);
        if (nRet == MV_OK)
        {
            nRet = m_pcMyCamera->SetIntValue("GevSCPSPacketSize",nPacketSize);
            if(nRet != MV_OK)
            {
                ShowErrorMsg(TEXT("Warning: Set Packet Size fail!"), nRet);
            }
        }
        else
        {
            ShowErrorMsg(TEXT("Warning: Get Packet Size fail!"), nRet);
        }
    }

    m_bOpenDevice = TRUE;

    return MV_OK;
}

// ch:�ر��豸 | en:Close Device
int CBasicDemoDlg::CloseDevice(void)
{  
    m_bThreadState = FALSE;
    if (m_hGrabThread)
    {
        WaitForSingleObject(m_hGrabThread, INFINITE);
        CloseHandle(m_hGrabThread);
        m_hGrabThread = NULL;
    }

    if (m_pcMyCamera)
    {
        m_pcMyCamera->Close();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }

    m_bStartGrabbing = FALSE;
    m_bOpenDevice = FALSE;

    if (m_pGrabBuf)
    {
        free(m_pGrabBuf);
        m_pGrabBuf = NULL;
    }
    m_nGrabBufSize = 0;

    if (m_pSaveImageBuf)
    {
        free(m_pSaveImageBuf);
        m_pSaveImageBuf = NULL;
    }
    m_nSaveImageBufSize  = 0;

    return MV_OK;
}

// ch:��ȡ����ģʽ | en:Get Trigger Mode
int CBasicDemoDlg::GetTriggerMode(void)
{
    MVCC_ENUMVALUE stEnumValue = {0};

    int nRet = m_pcMyCamera->GetEnumValue("TriggerMode", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_nTriggerMode = stEnumValue.nCurValue;
    if (MV_TRIGGER_MODE_ON ==  m_nTriggerMode)
    {
        OnBnClickedTriggerModeRadio();
    }
    else if (MV_TRIGGER_MODE_OFF == m_nTriggerMode)
    {
        OnBnClickedContinusModeRadio();
    }
    else
    {
        return STATUS_ERROR;
    }

    return MV_OK;
}

// ch:���ô���ģʽ | en:Set Trigger Mode
int CBasicDemoDlg::SetTriggerMode(void)
{
    int nRet = m_pcMyCamera->SetEnumValue("TriggerMode", m_nTriggerMode);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    return MV_OK;
}

// ch:��ȡ�ع�ʱ�� | en:Get Exposure Time
int CBasicDemoDlg::GetExposureTime(void)
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("ExposureTime", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    m_dExposureEdit = stFloatValue.fCurValue;

    return MV_OK;
}

// ch:�����ع�ʱ�� | en:Set Exposure Time
int CBasicDemoDlg::SetExposureTime(void)
{
    // ch:�����������ع�ģʽ���������ع�ʱ����Ч
    // en:Adjust these two exposure mode to allow exposure time effective
    int nRet = m_pcMyCamera->SetEnumValue("ExposureMode", MV_EXPOSURE_MODE_TIMED);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    nRet = m_pcMyCamera->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);

    nRet = m_pcMyCamera->SetFloatValue("ExposureTime", (float)m_dExposureEdit);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    return MV_OK;
}

// ch:��ȡ���� | en:Get Gain
int CBasicDemoDlg::GetGain(void)
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("Gain", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    m_dGainEdit = (int)stFloatValue.fCurValue;

    return MV_OK;
}

// ch:�������� | en:Set Gain
int CBasicDemoDlg::SetGain(void)
{
    // ch:��������ǰ�Ȱ��Զ�����رգ�ʧ�����践��
    //en:Set Gain after Auto Gain is turned off, this failure does not need to return
    int nRet = m_pcMyCamera->SetEnumValue("GainAuto", 0);

    return m_pcMyCamera->SetFloatValue("Gain", (float)m_dGainEdit);
}

// ch:��ȡ֡�� | en:Get Frame Rate
int CBasicDemoDlg::GetFrameRate(void)
{
    MVCC_FLOATVALUE stFloatValue = {0};

    int nRet = m_pcMyCamera->GetFloatValue("ResultingFrameRate", &stFloatValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }
    m_dFrameRateEdit = stFloatValue.fCurValue;

    return MV_OK;
}

// ch:����֡�� | en:Set Frame Rate
int CBasicDemoDlg::SetFrameRate(void)
{
    int nRet = m_pcMyCamera->SetBoolValue("AcquisitionFrameRateEnable", true);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    return m_pcMyCamera->SetFloatValue("AcquisitionFrameRate", (float)m_dFrameRateEdit);
}

// ch:��ȡ����Դ | en:Get Trigger Source
int CBasicDemoDlg::GetTriggerSource(void)
{
    MVCC_ENUMVALUE stEnumValue = {0};

    int nRet = m_pcMyCamera->GetEnumValue("TriggerSource", &stEnumValue);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    if ((unsigned int)MV_TRIGGER_SOURCE_SOFTWARE == stEnumValue.nCurValue)
    {
        m_bSoftWareTriggerCheck = TRUE;
    }
    else
    {
        m_bSoftWareTriggerCheck = FALSE;
    }

    return MV_OK;
}

// ch:���ô���Դ | en:Set Trigger Source
int CBasicDemoDlg::SetTriggerSource(void)
{
    if (m_bSoftWareTriggerCheck)
    {
        m_nTriggerSource = MV_TRIGGER_SOURCE_SOFTWARE;
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", m_nTriggerSource);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Software Trigger Fail"), nRet);
            return nRet;
        }
        GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON )->EnableWindow(TRUE);
    }
    else
    {
        m_nTriggerSource = MV_TRIGGER_SOURCE_LINE0;
        int nRet = m_pcMyCamera->SetEnumValue("TriggerSource", m_nTriggerSource);
        if (MV_OK != nRet)
        {
            ShowErrorMsg(TEXT("Set Hardware Trigger Fail"), nRet);
            return nRet;
        }
        GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON )->EnableWindow(FALSE);
    }

    return MV_OK;
}

// ch:����ͼƬ | en:Save Image
int CBasicDemoDlg::SaveImage()
{
    if (FALSE == m_bStartGrabbing)
    {
        return STATUS_ERROR;
    }

    // ch:Ԥ��BMPͷ | en:Reserved BMP header size
    unsigned int nSaveImageNeedSize = m_nGrabBufSize*3 + 2048;
    if (NULL == m_pSaveImageBuf || m_nSaveImageBufSize < nSaveImageNeedSize)
    {
        if (m_pSaveImageBuf)
        {
            free(m_pSaveImageBuf);
            m_pSaveImageBuf = NULL;
        }

        m_pSaveImageBuf = (unsigned char*)malloc(nSaveImageNeedSize);
        if (NULL == m_pSaveImageBuf)
        {
            return STATUS_ERROR;
        }
        m_nSaveImageBufSize = nSaveImageNeedSize;
    }

    MV_SAVE_IMG_TO_FILE_PARAM pstSaveFileParam;
    memset(&pstSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));

    EnterCriticalSection(&m_hSaveImageMux);
    pstSaveFileParam.enImageType = m_nSaveImageType; // ch:��Ҫ�����ͼ������ | en:Image format to save
    pstSaveFileParam.enPixelType = m_stImageInfo.enPixelType;  // ch:�����Ӧ�����ظ�ʽ | en:Camera pixel type
    pstSaveFileParam.nWidth      = m_stImageInfo.nWidth;         // ch:�����Ӧ�Ŀ� | en:Width
    pstSaveFileParam.nHeight     = m_stImageInfo.nHeight;          // ch:�����Ӧ�ĸ� | en:Height
    pstSaveFileParam.nDataLen    = m_stImageInfo.nFrameLen;
    pstSaveFileParam.pData       = m_pGrabBuf;

    // ch:jpgͼ��������ΧΪ(50-99], pngͼ��������ΧΪ[0-9] | en:jpg image nQuality range is (50-99], png image nQuality range is [0-9]
    if (m_nSaveImageType == MV_Image_Jpeg)
    {
        pstSaveFileParam.nQuality = 80;
    }
    else if(m_nSaveImageType == MV_Image_Png)
    {
        pstSaveFileParam.nQuality = 8;
    }
    unsigned int nFrameNum = m_stImageInfo.nFrameNum;
    pstSaveFileParam.iMethodValue = 0;

    char chImageName[IMAGE_NAME_LEN] = {0};

    if (MV_Image_Bmp == pstSaveFileParam.enImageType)
    {
        sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d.bmp",pstSaveFileParam.nWidth, pstSaveFileParam.nHeight, nFrameNum);
    }
    else if (MV_Image_Jpeg == pstSaveFileParam.enImageType)
    {
        sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d.jpg", pstSaveFileParam.nWidth, pstSaveFileParam.nHeight, nFrameNum);
    }
    else if (MV_Image_Tif == pstSaveFileParam.enImageType)
    {
        sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d.tif", pstSaveFileParam.nWidth, pstSaveFileParam.nHeight, nFrameNum);
    }
    else if (MV_Image_Png == pstSaveFileParam.enImageType)
    {
        strcpy(chImageName, "1.png");
       // sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d.png", pstSaveFileParam.nWidth, pstSaveFileParam.nHeight, nFrameNum);
    }
    //char chImageNamepath[IMAGE_NAME_LEN] = { 0 };
    //strcpy(chImageNamepath, "d:\\");
    //strcat(chImageNamepath, chImageName);
    //sprintf()
    ////chImageName = "c:\\" + chImageName;
    memcpy(pstSaveFileParam.pImagePath, chImageName, IMAGE_NAME_LEN);

    int nRet = m_pcMyCamera->SaveImageToFile(&pstSaveFileParam);
    LeaveCriticalSection(&m_hSaveImageMux);
    if(MV_OK != nRet)
    {
        return STATUS_ERROR;
    }
    return 0;
}

//void  CBasicDemoDlg::update_mhi(IplImage* img, IplImage* dst, int diff_threshold)
//{
//    IplImage* showimg = img;
//    double timestamp = (double)clock() / CLOCKS_PER_SEC; // get current time in seconds   
//    CvSize size = cvSize(img->width, img->height); // get current frame size   
//    int i, idx1 = NULL,last = 0, idx2;
//    IplImage* silh;
//    CvSeq* seq;
//    CvRect comp_rect;
//    double count;
//    double angle;
//    CvPoint center;
//    double magnitude;
//    CvScalar color;
//
//    // allocate images at the beginning or   
//    // reallocate them if the frame size is changed   
//    if (!mhi || mhi->width != size.width || mhi->height != size.height)
//    {
//        if (buf == 0)
//        {
//            buf = (IplImage**)malloc(3 * sizeof(buf[0]));
//            memset(buf, 0, 3 * sizeof(buf[0]));
//        }
//
//        for (i = 0; i < 3; i++)
//        {
//            cvReleaseImage(&buf[i]);
//            buf[i] = cvCreateImage(size, IPL_DEPTH_8U, 1);
//            cvZero(buf[i]);
//        }
//        cvReleaseImage(&mhi);
//        cvReleaseImage(&orient);
//        cvReleaseImage(&segmask);
//        cvReleaseImage(&mask);
//
//        mhi = cvCreateImage(size, IPL_DEPTH_32F, 1);
//        cvZero(mhi); // clear MHI at the beginning   
//        orient = cvCreateImage(size, IPL_DEPTH_32F, 1);
//        segmask = cvCreateImage(size, IPL_DEPTH_32F, 1);
//        mask = cvCreateImage(size, IPL_DEPTH_8U, 1);
//    }
//
//    cvCvtColor(img, buf[last], CV_BGR2GRAY); // ��֡����ת��Ϊ�Ҷ�ͼ��buf[0]ָ���һ֡ͼ��buf[1]�ڶ�֡   
//
//    idx2 = (last + 1) % 3; // ÿN֡��һСѭ������idx2��1��2��3����N��������N���ֱ��1��index of (last - (N-1))th frame   
//    last = idx2;//����last=0��1��2��3��0��1��2��3������   
//
//    silh = buf[idx2];
//    cvAbsDiff(buf[idx1], buf[idx2], silh); // ǰ��֡��silh = abs(buf[idx1] - buf[idx2])��//get difference between frames   
//
//    cvThreshold(silh, silh, diff_threshold, 1, CV_THRESH_BINARY); //������ͼ��������ֵ��ֵ�� //and threshold it   
//    cvUpdateMotionHistory(silh, mhi, timestamp, MHI_DURATION); //ȥ��"�˶�ʱ��������Ӱ�������һ���Ķ���"�Ը����˶���ʷͼ��// update MHI   
//                                                                //MHI��motion history image�� �����˶����������ص㱻����Ϊ��ǰʱ��������˶������Ͼõ����ص㱻�����   
//                                                                //mhiΪ��ֵ��ʷͼ   
//    // ����ֵ��ʷͼmhiת���ɻҶ�ͼ����ת����rgbͼ//convert MHI to blue 8u image   
//    cvCvtScale(mhi, mask, 255. / MHI_DURATION,
//        (MHI_DURATION - timestamp) * 255. / MHI_DURATION);// ����ֵ��ʷͼmhiת���ɻҶ�ͼ��
//                                                        //timestamp=1ʱ�պý���ʷ��������
//    cvZero(dst);
//    cvSplit(mask, 0, 0, 0, dst);//�Ӽ�����ͨ��������ϳɶ�ͨ������,�����Ҷ�ͼת��Ϊ��ɫͼ����BGR˳����������ɫ
//
//    // �����˶��ݶȷ��򣬺ͷ���꣬calculate motion gradient orientation and valid orientation mask   
//    cvCalcMotionGradient(mhi, mask, orient, MAX_TIME_DELTA, MIN_TIME_DELTA, 3);
//
//    if (!storage)
//        storage = cvCreateMemStorage(0);
//    else
//        cvClearMemStorage(storage);
//
//    // ��ø����˶��������������˶��ֳɼ����������˶����洢��storage�segment motion: get sequence of motion components   
//    //segmask���ڴ�ű���ǵ��˶�ͼ��ӳ��ͼ��// segmask is marked motion components map. It is not used further   
//    seq = cvSegmentMotion(mhi, segmask, storage, timestamp, MAX_TIME_DELTA);
//    /*mhi
//    �˶���ʷͼ��
//    seg_mask
//    ����Ӧ���洢�� mask ��ͼ��, ��ͨ��, 32bits�� ������.
//    storage
//    �����˶���ͨ�����е��ڴ�洢��
//    timestamp
//    ��ǰʱ�䣬���뵥λ
//    seg_thresh
//    �ָ���ֵ���Ƽ����ڻ�����˶���ʷ��ÿ����֮��ļ����
//    */
//
//    //cvNamedWindow( "segmask", 1 );   
//    //cvShowImage( "segmask", segmask );   
//    // ���������˶�����iterate through the motion components,   
//    // One more iteration (i == -1) corresponds to the whole image (global motion)  
//    int personCount = 0;
//    for (i = 0; i < seq->total; i++)
//    {
//
//        if (i < 0) { // case of the whole image   
//            comp_rect = cvRect(0, 0, size.width, size.height);
//            color = CV_RGB(255, 255, 255);//��ɫ��ʾ�����˶�ͼ   
//            magnitude = 100;//ָ��Բ�뾶   
//        }
//        else { // i-th motion component   
//            comp_rect = ((CvConnectedComp*)cvGetSeqElem(seq, i))->rect;
//            // if( comp_rect.width + comp_rect.height < 70 ) // ���Ժ�С����ͨ����reject very small components   
//            //     continue;   
//            if (comp_rect.height < 30 || comp_rect.width < 10)
//                continue;
//            color = CV_RGB(255, 0, 0);//��ɫ��ʾ�����˶�ͼ   
//            magnitude = 20;//ָ��Բ�뾶   
//            personCount++;
//        }
//
//        //ѡ�����Ȥ���˶���������ͨ���;���comp_rectƥ��õ��ģ� select component ROI������Ȥ����   
//        cvSetImageROI(silh, comp_rect);//���ڸ����ľ���comp_rect����ͼ��� ROI������Ȥ����   
//        cvSetImageROI(mhi, comp_rect);
//        cvSetImageROI(orient, comp_rect);
//        cvSetImageROI(mask, comp_rect);
//
//        // ���㷽�򣨽Ƕȣ�calculate orientation   
//        angle = cvCalcGlobalOrientation(orient, mask, mhi, timestamp, MHI_DURATION);//����һ��ѡ�񲿼����˶�����   
//        angle = 360.0 - angle;  // adjust for images with top-left origin   
//
//        count = cvNorm(silh, 0, CV_L1, 0); // �����Χ��������С���������ĸ���calculate number of points within silhouette�������� ROI   
//
//        cvResetImageROI(mhi);
//        cvResetImageROI(orient);
//        cvResetImageROI(mask);
//        cvResetImageROI(silh);
//
//        // check for the case of little motion   
//        if (count < comp_rect.width * comp_rect.height * 0.05)
//            continue;
//
//        // draw a clock with arrow indicating the direction   
//        center = cvPoint((comp_rect.x + comp_rect.width / 2),
//            (comp_rect.y + comp_rect.height / 2));
//
//        cvCircle(img, center, cvRound(magnitude * 1.2), color, 3, CV_AA, 0);    //cvRound: ��һ��double�͵���������������
//        cvLine(img, center, cvPoint(cvRound(center.x + magnitude * cos(angle * CV_PI / 180)),
//            cvRound(center.y - magnitude * sin(angle * CV_PI / 180))), color, 3, CV_AA, 0);
//    }
// //   std::cout << "��Ƶ���У� " << personCount << " ����" << std::endl;
//}
int CBasicDemoDlg::RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
    if (NULL == pRgbData)
    {
        return MV_E_PARAMETER;
    }
    try
    {

    
    for (unsigned int j = 0; j < nHeight; j++)
    {
        for (unsigned int i = 0; i < nWidth; i++)
        {
            unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
            pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
            pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
        }
    }
  //  WriteLog(L"" + __LINE__);
    }
    catch (Exception ex)
    {
        WriteLog(L"error:RGB2BGR");
    }
    return MV_OK;
}
void CBasicDemoDlg::DrawPicToHDC(IplImage* img, UINT ID)
{
    CDC* pDC = GetDlgItem(ID)->GetDC();
    HDC hDC = pDC->GetSafeHdc();
    CRect rect;
    GetDlgItem(ID)->GetClientRect(&rect);
    CvvImage cimg;
    cimg.CopyOf(img); // ����ͼƬ
    cimg.DrawToHDC(hDC, &rect); // ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������
    ReleaseDC(pDC);
}
/*

*/
bool CBasicDemoDlg::ImageMain(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData)
{
   
    clock_t start, finish;
    double  duration;
    double a;
   
    start = clock();
    if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
    {
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
    }
    else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
    {
        RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
    }
    else
    {
        printf("unsupported pixel format\n");
        MessageBox(L"��Ƶ��ʽ���ԣ���Ҫ������RGB8.");
        return false;
    }

    if (NULL == srcImage.data)
    {
        return false;
    }
    orgPic = srcImage;
    srcImage = srcImage(Rect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y));
    try {
        if (showdiff)
          {
            cv::Mat bmpImgGrayRoi;
            bmpImgGrayRoi = bmpImgGray(Rect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y));

                cvtColor(srcImage , srcImageGray, CV_BGR2GRAY);  //1 ԭͼ�� 2  ���ͼ��  �Ҷȴ���
                cv::absdiff(srcImageGray, bmpImgGrayRoi,diffImgGray);    //ͼ������
                cv::threshold(diffImgGray, diffImgGray, m_Threshold,m_maxvalue, CV_THRESH_BINARY);  //������ֵ���� ��ֵ��
                 
                // 4.��ʴ  
                Mat kernel_erode = getStructuringElement(MORPH_RECT, Size(3, 3));
                Mat kernel_dilate = getStructuringElement(MORPH_RECT, Size(18, 18));
                erode(diffImgGray, diffImgGray, kernel_erode);
               
                // 5.����  
               // dilate(imresult, imresult, kernel_dilate);

                vector<vector<Point>> contours;
                findContours(diffImgGray, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
                // ��result�ϻ�������
                //drawContours(srcImage, contours, -1, Scalar(0, 200, 255), 2);  //ԭͼ��
                // 7.��������Ӿ���  
                vector<Rect> boundRect(contours.size());
                for (int i = 0; i < contours.size(); i++)
                {
                    boundRect[i] = boundingRect(contours[i]);
                    // ��result�ϻ�������Ӿ���
                    rectangle(srcImage, boundRect[i], Scalar(0, 255, 0), 2);  //��ԭͼ�ϻ�����
                }

                if (contours.size() >= 0)
                {
                    CString str;
                    str.Format(_T("%d"), contours.size());
                    CStatic* pStatic = (CStatic*)GetDlgItem(IDC_BUTTON_showerror);
                    pStatic->SetWindowText(str);
                }
                double timestamp = (double)clock() / CLOCKS_PER_SEC; // get current time in seconds  

                //update_mhi(pBkImg, motion, 30);

                rotate(diffImgGray, diffImgGray, m_rotate);
                WriteLog(L"ͼ����ʾ");
               // DrawPicToHDC(&(IplImage(imresult)), IDC_DISPLAY_STATIC);  // ��ʾ�Ҳദ��ͼ
                ShowImage(diffImgGray, IDC_DISPLAY_STATIC);
            //}
        }
        //ͼ����ת 
        if (!isDiffScFull)  //����Ҳ�ȫ������ֹͣ��ʾ���ͼ��
        {
            rotate(srcImage, srcImage, m_rotate);
            ShowImage(srcImage, IDC_STATIC_video);
           // DrawPicToHDC(&(IplImage(srcImage)), IDC_STATIC_video);  //��ര��
        }
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        CString str;
        str.Format(_T("��ʱ��%f��"), duration);


        CStatic * pStatic = (CStatic*)GetDlgItem(IDC_STATIC_time);
        pStatic->SetWindowText(str);
      
        srcImage.release();
        
    }
    catch (cv::Exception& ex) {

          //  CBasicDemoDlg lg = new CBasicDemoDlg();
            CString cstr(ex.msg.c_str());
           WriteLog(L"ERROR:  " + __LINE__ + cstr);
         //  return false;
    }

   // srcImage.release();

    return true;
}
void CBasicDemoDlg::MatToCImage(cv::Mat mat, ATL::CImage& cimage)
{
    if (0 == mat.total())
    {
        return;
    }


    int nChannels = mat.channels();
    if ((1 != nChannels) && (3 != nChannels))
    {
        return;
    }
    int nWidth = mat.cols;
    int nHeight = mat.rows;


    //�ؽ�cimage
    cimage.Destroy();
    cimage.Create(nWidth, nHeight, 8 * nChannels);


    //��������


    uchar* pucRow;                                    //ָ������������ָ��
    uchar* pucImage = (uchar*)cimage.GetBits();        //ָ����������ָ��
    int nStep = cimage.GetPitch();                    //ÿ�е��ֽ���,ע���������ֵ�����и�


    if (1 == nChannels)                                //���ڵ�ͨ����ͼ����Ҫ��ʼ����ɫ��
    {
        RGBQUAD* rgbquadColorTable;
        int nMaxColors = 256;
        rgbquadColorTable = new RGBQUAD[nMaxColors];
        cimage.GetColorTable(0, nMaxColors, rgbquadColorTable);
        for (int nColor = 0; nColor < nMaxColors; nColor++)
        {
            rgbquadColorTable[nColor].rgbBlue = (uchar)nColor;
            rgbquadColorTable[nColor].rgbGreen = (uchar)nColor;
            rgbquadColorTable[nColor].rgbRed = (uchar)nColor;
        }
        cimage.SetColorTable(0, nMaxColors, rgbquadColorTable);
        delete[]rgbquadColorTable;
    }


    for (int nRow = 0; nRow < nHeight; nRow++)
    {
        pucRow = (mat.ptr<uchar>(nRow));
        for (int nCol = 0; nCol < nWidth; nCol++)
        {
            if (1 == nChannels)
            {
                *(pucImage + nRow * nStep + nCol) = pucRow[nCol];
            }
            else if (3 == nChannels)
            {
                for (int nCha = 0; nCha < 3; nCha++)
                {
                    *(pucImage + nRow * nStep + nCol * 3 + nCha) = pucRow[nCol * 3 + nCha];
                }
            }
        }
    }
}   //ͼ��ת�� mattocimage
/***
showImage Ϊ��ʵ�ְ��ձ�����ʾͼƬ
*/
void CBasicDemoDlg::ShowImage(cv::Mat img, UINT ID)// ID ��Picture Control�ؼ���ID��     
{
   // CRect   rect;
    ATL::CImage  q;
    MatToCImage(img, q);
   // CWnd* pWnd = NULL;
	pWnd = GetDlgItem(ID);//��ȡ�ؼ����
	pWnd->GetClientRect(&picRect);//��ȡPicture Control�ؼ��Ŀͻ���
    float cx = img.cols;
    float cy = img.rows;//��ȡͼƬ�Ŀ� ��
    float k = cy / cx;//���ͼƬ�Ŀ�߱�
   
    float dx = picRect.Width();
    float dy = picRect.Height();//��ÿؼ��Ŀ�߱�
    float t = dy / dx;//��ÿؼ��Ŀ�߱� 
    if (k >= t)
    {

        picRect.right = floor(picRect.bottom / k);
        picRect.left = (dx - picRect.right) / 2;
        picRect.right = floor(picRect.bottom / k) + (dx - picRect.right) / 2;
      //  picRect.right += 100;
    }
    else
    {
        picRect.bottom = floor(k * picRect.right);
        picRect.top = (dy - picRect.bottom) / 2;
        picRect.bottom = floor(k * picRect.right) + (dy - picRect.bottom) / 2;
       // picRect.bottom += 100;
    }
    //��صļ���Ϊ����ͼƬ�ڻ�ͼ�����а�������ʾ��ԭ��ܺö������ͼƬ�ܿ��ǲ��ߣ����������пհ��������ͼƬ�ܸ߶�������������пհ��������ұ������߿հ���һ����
    CDC* pDc = NULL;
    pDc = pWnd->GetDC();//��ȡpicture control��DC������ʲô������Ҳ��֪�����ٶȾ���
    int ModeOld = SetStretchBltMode(pDc->m_hDC, STRETCH_HALFTONE);//����ָ���豸�����е�λͼ����ģʽ
   
    q.StretchBlt(pDc->m_hDC, picRect, SRCCOPY);//��ʾ����
    SetStretchBltMode(pDc->m_hDC, ModeOld);
    
    ReleaseDC(pDc);//�ͷ�ָ��ռ�
}
//void CBasicDemoDlg::ResizeImage(cv::Mat img)
//{
//    // ��ȡͼƬ�Ŀ�͸�  
//    int w = img.cols;
//    int h = img.rows;
//
//    // �ҳ���͸��еĽϴ�ֵ��  
//    int max = (w > h) ? w : h;
//
//    // ���㽫ͼƬ���ŵ�TheImage��������ı�������  
//    float scale = (float)((float)max / 256.0f);
//
//    // ���ź�ͼƬ�Ŀ�͸�  
//    int nw = (int)(w / scale);
//    int nh = (int)(h / scale);
//
//    // Ϊ�˽����ź��ͼƬ���� TheImage �����в�λ�������ͼƬ�� TheImage ���Ͻǵ���������ֵ  
//    int tlx = (nw > nh) ? 0 : (int)(256 - nw) / 2;
//    int tly = (nw > nh) ? (int)(256 - nh) / 2 : 0;
//
//    // ���� TheImage �� ROI ������������ͼƬ img  
//    cvSetImageROI(TheImage, cvRect(tlx, tly, nw, nh));
//
//    // ��ͼƬ img �������ţ������뵽 TheImage ��  
//    cvResize(&img, TheImage);
//
//    // ���� TheImage �� ROI ׼��������һ��ͼƬ  
//    cvResetImageROI(TheImage);
//}
int CBasicDemoDlg::GrabThreadProcess()
{
    // ch:������л�ȡһ֡ͼ���С | en:Get size of one frame from camera
    try
    {


        MVCC_INTVALUE_EX stIntValue = { 0 };
       
        int nRet = m_pcMyCamera->GetIntValue("PayloadSize", &stIntValue);
        if (nRet != MV_OK)
        {
            ShowErrorMsg(TEXT("failed in get PayloadSize"), nRet);
            return nRet;
        }
        unsigned int nDataSize = (unsigned int)stIntValue.nCurValue;

        if (NULL == m_pGrabBuf || nDataSize > m_nGrabBufSize)
        {
            if (m_pGrabBuf)
            {
                free(m_pGrabBuf);
                m_pGrabBuf = NULL;
            }

            m_pGrabBuf = (unsigned char*)malloc(sizeof(unsigned char) * nDataSize);
            if (m_pGrabBuf == NULL)
            {
                return 0;
            }
            m_nGrabBufSize = nDataSize;
        }
        WriteLog(L"׼����ʼ����wileѭ��");
        MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
        MV_DISPLAY_FRAME_INFO stDisplayInfo = { 0 };
        bool bConvertRet = false;
        while (m_bThreadState)
        {
          //  WriteLog(L"1");
         //   Sleep(m_edt_DelayValue);
            
            EnterCriticalSection(&m_hSaveImageMux);
           // WriteLog(L"2");
            nRet = m_pcMyCamera->GetOneFrameTimeout(m_pGrabBuf, m_nGrabBufSize, &stImageInfo, 1000);
           // WriteLog(L"3");
            if (nRet == MV_OK)
            {
               // WriteLog(L"4");
                memcpy(&m_stImageInfo, &stImageInfo, sizeof(MV_FRAME_OUT_INFO_EX));
            }
			
          //  WriteLog(L"5");
            LeaveCriticalSection(&m_hSaveImageMux);

            if (nRet == MV_OK)
            {
                if (RemoveCustomPixelFormats(stImageInfo.enPixelType))
                {
                    continue;
                }
                //ת��ΪMAT ��ʽ����ʾ
                WriteLog(L"׼������convert2Mat");
                bConvertRet = ImageMain(&stImageInfo, m_pGrabBuf);
                if (!bConvertRet)
                {
                    break;
                }
          
            }
            else
            {
                if (MV_TRIGGER_MODE_ON == m_nTriggerMode)
                {
                    Sleep(5);
                }
            }
        }
    }
    catch (Exception ex)        
    {
   //     CBasicDemoDlg lg = new CBasicDemoDlg();
        CString cstr(ex.msg.c_str());
        WriteLog(L"ERROR:  " + __LINE__ +  cstr);
    }
    return MV_OK;
}

// convert data stream in Mat format

// ch:���²����豸��ť:ö�� | en:Click Find Device button:Enumeration 
void CBasicDemoDlg::OnBnClickedEnumButton()
{
    CString strMsg;
    CString strIndex;
    try
    {

    m_ctrlDeviceCombo.ResetContent();
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // ch:ö�������������豸 | en:Enumerate all devices within subnet
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("�����������"), 0);
        return;
    }

    // ch:��ֵ���뵽��Ϣ�б���в���ʾ���� | en:Add value to the information list box and display
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }

        wchar_t* pUserName = NULL;
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0)
            {
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName), -1, pUserName, dwLenUserName);
            }
            else
            {
                char strUserName[256] = {0};
                sprintf_s(strUserName, "%s %s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName,
                    pDeviceInfo->SpecialInfo.stGigEInfo.chModelName,
                    pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            strMsg.Format(_T("[%d]GigE:    %s  (%d.%d.%d.%d)"), i, pUserName, nIp1, nIp2, nIp3, nIp4);
            strIndex.Format(_T("[%d]����� (%d.%d.%d.%d)"), i, nIp1, nIp2, nIp3, nIp4);
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName) != 0)
            {
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName), -1, pUserName, dwLenUserName);
            }
            else
            {
                char strUserName[256];
                sprintf_s(strUserName, "%s %s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName,
                    pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName,
                    pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
                pUserName = new wchar_t[dwLenUserName];
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
            }
            strMsg.Format(_T("[%d]UsbV3:  %s"), i, pUserName);
            strIndex.Format(_T("[%d]�����"), i);
        }
        else
        {
            ShowErrorMsg(TEXT("Unknown device enumerated"), 0);;
        }
        m_ctrlDeviceCombo.AddString(strIndex);

        if (pUserName)
        {
            delete[] pUserName;
            pUserName = NULL;
        }
    }

    if (0 == m_stDevList.nDeviceNum)
    {
        ShowErrorMsg(TEXT("No device"), 0);
        return;
    }
    m_ctrlDeviceCombo.SetCurSel(0);

    EnableControls(TRUE);

    }
    catch (Exception ex)
    {
        CString cstr(ex.msg.c_str());
        WriteLog(cstr);
        ShowErrorMsg(cstr, 0);
        return;
    }
    return;
}

// ch:���´��豸��ť�����豸 | en:Click Open button: Open Device
void CBasicDemoDlg::OnBnClickedOpenButton()
{
    int nRet = OpenDevice();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Open Fail"), nRet);
        return;
    }

    //ת����rgb8ͼ��
    nRet = m_pcMyCamera->SetEnumValue("PixelFormat", 0x02180014);
    if (MV_OK != nRet)
    {
        MessageBox(L"ת����RGB����");
        return;
    }

    OnBnClickedGetParameterButton(); // ch:��ȡ���� | en:Get Parameter
    EnableControls(TRUE);
    MVCC_INTVALUE_EX height = { 0 }, width = { 0 };
	m_pcMyCamera->GetIntValue("Height", &height);
    m_pcMyCamera->GetIntValue("Width", &width);
    m_picHeight = height.nCurValue;
    m_picWeight = width.nCurValue;
	
    startPoint.x = 0;
	startPoint.y = 0;
	endPoint.x = m_picWeight;
	endPoint.y = m_picHeight;
	
	h_w_float = float(m_picHeight) / float(m_picWeight);//���ͼƬ�Ŀ�߱�
//	rc.SetRect(0, 0, m_picWeight, m_picHeight);//���þ��ε������ǵ�

    return;
}

// ch:���¹ر��豸��ť���ر��豸 | en:Click Close button: Close Device
void CBasicDemoDlg::OnBnClickedCloseButton()
{
    OnBnClickedStopGrabbingButton();
    CloseDevice();
    EnableControls(TRUE);

    return;
}

// ch:��������ģʽ��ť | en:Click Continues button
void CBasicDemoDlg::OnBnClickedContinusModeRadio()
{
    ((CButton *)GetDlgItem(IDC_CONTINUS_MODE_RADIO))->SetCheck(TRUE);
    ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->SetCheck(FALSE);
    m_nTriggerMode = MV_TRIGGER_MODE_OFF;
    int nRet = SetTriggerMode();
    if (MV_OK != nRet)
    {
        return;
    }
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->EnableWindow(FALSE);

    return;
}

// ch:���´���ģʽ��ť | en:Click Trigger Mode button
void CBasicDemoDlg::OnBnClickedTriggerModeRadio()
{
    UpdateData(TRUE);
    ((CButton *)GetDlgItem(IDC_CONTINUS_MODE_RADIO))->SetCheck(FALSE);
    ((CButton *)GetDlgItem(IDC_TRIGGER_MODE_RADIO))->SetCheck(TRUE);
    m_nTriggerMode = MV_TRIGGER_MODE_ON;
    int nRet = SetTriggerMode();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Set Trigger Mode Fail"), nRet);
        return;
    }

    if (m_bStartGrabbing == TRUE)
    {
        if (TRUE == m_bSoftWareTriggerCheck)
        {
            GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON )->EnableWindow(TRUE);
        }
    }

    return;
}

// ch:���¿�ʼ�ɼ���ť | en:Click Start button
void CBasicDemoDlg::OnBnClickedStartGrabbingButton()
{
    if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }
    nFrmNum = 0;

    memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
    m_bThreadState = TRUE;
    unsigned int nThreadID = 0;
    WriteLog(L"start thread");
    m_hGrabThread = (void*)_beginthreadex( NULL , 0 , GrabThread , this, 0 , &nThreadID );
    if (NULL == m_hGrabThread)
    {
        m_bThreadState = FALSE;
        ShowErrorMsg(TEXT("Create thread fail"), 0);
        return;
    }

    int nRet = m_pcMyCamera->StartGrabbing();
    if (MV_OK != nRet)
    {
        m_bThreadState = FALSE;
        ShowErrorMsg(TEXT("Start grabbing fail"), nRet);
        return;
    }
    m_bStartGrabbing = TRUE;
    EnableControls(TRUE);
   
    return;
}

// ch:���½����ɼ���ť | en:Click Stop button
void CBasicDemoDlg::OnBnClickedStopGrabbingButton()
{
    showdiff = false;
    if (FALSE == m_bOpenDevice || FALSE == m_bStartGrabbing || NULL == m_pcMyCamera)
    {
        return;
    }

    m_bThreadState = FALSE;
    if (m_hGrabThread)
    {
      //  WaitForSingleObject(m_hGrabThread, INFINITE);   //����ط������ڵȴ�������Ῠ��������ע�͵�
        CloseHandle(m_hGrabThread);
        m_hGrabThread = NULL;
    }

    int nRet = m_pcMyCamera->StopGrabbing();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Stop grabbing fail"), nRet);
        return;
    }
    m_bStartGrabbing = FALSE;
    EnableControls(TRUE);

    return;
}

// ch:���»�ȡ������ť | en:Click Get Parameter button
void CBasicDemoDlg::OnBnClickedGetParameterButton()
{
    int nRet = GetTriggerMode();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Trigger Mode Fail"), nRet);
    }

    nRet = GetExposureTime();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Exposure Time Fail"), nRet);
    }

    nRet = GetGain();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Gain Fail"), nRet);
    }

    nRet = GetFrameRate();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Frame Rate Fail"), nRet);
    }

    nRet = GetTriggerSource();
    if (nRet != MV_OK)
    {
        ShowErrorMsg(TEXT("Get Trigger Source Fail"), nRet);
    }

    UpdateData(FALSE);
    return;
}

// ch:�������ò�����ť | en:Click Set Parameter button
void CBasicDemoDlg::OnBnClickedSetParameterButton()
{
    UpdateData(TRUE);

    bool bIsSetSucceed = true;
    int nRet = SetExposureTime();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg(TEXT("Set Exposure Time Fail"), nRet);
    }
    nRet = SetGain();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg(TEXT("Set Gain Fail"), nRet);
    }
    nRet = SetFrameRate();
    if (nRet != MV_OK)
    {
        bIsSetSucceed = false;
        ShowErrorMsg(TEXT("Set Frame Rate Fail"), nRet);
    }
    
    if (true == bIsSetSucceed)
    {
        ShowErrorMsg(TEXT("Set Parameter Succeed"), nRet);
    }

    return;
}

// ch:����������ť | en:Click Software button
void CBasicDemoDlg::OnBnClickedSoftwareTriggerCheck()
{
    UpdateData(TRUE);

    int nRet = SetTriggerSource();
    if (nRet != MV_OK)
    {
        return;
    }

    return;
}

// ch:��������һ�ΰ�ť | en:Click Execute button
void CBasicDemoDlg::OnBnClickedSoftwareOnceButton()
{
    if (TRUE != m_bStartGrabbing)
    {
        return;
    }

    int nRet = m_pcMyCamera->CommandExecute("TriggerSoftware");

    return;
}

// ch:���±���bmpͼƬ��ť | en:Click Save BMP button
void CBasicDemoDlg::OnBnClickedSaveBmpButton()
{
    if(RemoveCustomPixelFormats(m_stImageInfo.enPixelType))
    {
        ShowErrorMsg(TEXT("Not Support!"),0);
        return;
    }
    m_nSaveImageType = MV_Image_Bmp;
    int nRet = SaveImage();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Save bmp fail"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save bmp succeed"), nRet);

    return;
}

// ch:���±���jpgͼƬ��ť | en:Click Save JPG button
void CBasicDemoDlg::OnBnClickedSaveJpgButton()
{
    if(RemoveCustomPixelFormats(m_stImageInfo.enPixelType))
    {
        ShowErrorMsg(TEXT("Not Support!"),0);
        return;
    }
    m_nSaveImageType = MV_Image_Jpeg;
    int nRet = SaveImage();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Save jpg fail"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save jpg succeed"), nRet);

    return;
}

// ch:���Ͻ��˳� | en:Exit from upper right corner
void CBasicDemoDlg::OnClose()
{
    OnBnClickedStopGrabbingButton();
  //  CloseDevice();
    Sleep(1000);
    PostQuitMessage(0);
    CloseDevice();

    DeleteCriticalSection(&m_hSaveImageMux);
    exit(0);
    CDialog::OnClose();
}

BOOL CBasicDemoDlg::PreTranslateMessage(MSG* pMsg)
{


    if (m_hAccel)
    {
        if (::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
        {
            return(TRUE);
        }
        else
        {
            if(pMsg->message == WM_KEYDOWN)
            {
                int nRet;
                    if(int(pMsg->wParam) == 109)
                    {
			           int result = _ttoi(m_editDelay) - 5000;

			            if (result < 0)
				        result = 0;
                        m_editDelay.Format(L"%d", result);
						 nRet = m_pcMyCamera->SetFloatValue("TriggerDelay", float(result));
                        UpdateData(false);
						if (MV_OK != nRet)
						{
					//		return 0;
						}

					//	UpdateData(false);
                     }
                    else
                        if (int(pMsg->wParam) == 107)
                        {
							int result = _ttoi(m_editDelay) + 5000;

							if (result > 9000000)
								result = 9000000;
                            m_editDelay.Format(L"%d", result);
							 nRet = m_pcMyCamera->SetFloatValue("TriggerDelay", float(result));
                            UpdateData(false);
							if (MV_OK != nRet)
							{
//								return 0;
							}

							
                        }
            }
        }
     
    }
	
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
    return CDialog::PreTranslateMessage(pMsg);
}

void CBasicDemoDlg::OnBnClickedSaveTiffButton()
{
    if(RemoveCustomPixelFormats(m_stImageInfo.enPixelType))
    {
        ShowErrorMsg(TEXT("Not Support!"),0);
        return;
    }
    m_nSaveImageType = MV_Image_Tif;
    int nRet = SaveImage();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Save tiff fail"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save tiff succeed"), nRet);

    return;
}

void CBasicDemoDlg::OnBnClickedSavePngButton()
{
    if(RemoveCustomPixelFormats(m_stImageInfo.enPixelType))
    {
        ShowErrorMsg(TEXT("Not Support!"),0);
        return;
    }
    m_nSaveImageType = MV_Image_Png;
    int nRet = SaveImage();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Save png fail"), nRet);
        return;
    }
    ShowErrorMsg(TEXT("Save png succeed"), nRet);

    return;
}

bool CBasicDemoDlg::RemoveCustomPixelFormats(enum MvGvspPixelType enPixelFormat)
{
     int nResult = enPixelFormat & MV_GVSP_PIX_CUSTOM;
    if(MV_GVSP_PIX_CUSTOM == nResult)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void CBasicDemoDlg::OnBnClickedButtonDiff()
{
    showdiff = !showdiff;
    try {
        src = imread("1.bmp");
        cvtColor(src, bmpImgGray, CV_BGR2GRAY);
    }
    catch (Exception ex)
    {
        return;
  }

}

void CBasicDemoDlg::resize()
{
    try
    {
        
        float fsp[2];
        POINT Newp; //��ȡ���ڶԻ���Ĵ�С
        CRect recta;
        GetClientRect(&recta);     //ȡ�ͻ�����С  
        Newp.x = recta.right - recta.left;
        Newp.y = recta.bottom - recta.top;
        fsp[0] = (float)Newp.x / Old.x;
        fsp[1] = (float)Newp.y / Old.y;
        CRect Rect;
        int woc;
        CPoint OldTLPoint, TLPoint; //���Ͻ�
        CPoint OldBRPoint, BRPoint; //���½�
        HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //�г����пؼ�  
        while (hwndChild)
        {
            woc = ::GetDlgCtrlID(hwndChild);//ȡ��ID
            GetDlgItem(woc)->GetWindowRect(Rect);
            ScreenToClient(Rect);
            OldTLPoint = Rect.TopLeft();
            TLPoint.x = long(OldTLPoint.x * fsp[0]);
            TLPoint.y = long(OldTLPoint.y * fsp[1]);
            OldBRPoint = Rect.BottomRight();
            BRPoint.x = long(OldBRPoint.x * fsp[0]);
            BRPoint.y = long(OldBRPoint.y * fsp[1]);
            Rect.SetRect(TLPoint, BRPoint);
            GetDlgItem(woc)->MoveWindow(Rect, TRUE);
            hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
        }
        Old = Newp;
		CWnd* pControl = GetDlgItem(IDC_STATIC_video);
		pControl->GetWindowRect(&rect_view);    //����ӿؼ�����Ļ���ꣻ     
		this->ScreenToClient(&rect_view);   //�ӿؼ���Ļ����ӳ�䵽�ؼ��ͻ�����������һ��Ҫ��this
    }
    catch (Exception ex)
    {
        CString cstr(ex.msg.c_str());
        WriteLog(cstr);
    }

}
void CBasicDemoDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    if (nType == SIZE_MAXIMIZED) { //nType == SIZE_RESTORED || 
        GetDlgItem(IDC_STATIC_video)->ShowWindow(FALSE);
        GetDlgItem(IDC_STATIC_video)->ShowWindow(TRUE);
        resize();
    }
}

void CBasicDemoDlg::WriteLog( CString msg)
{
    CString filename;
   
    try
    {
        if (!m_WriteLog)
            return;
        CTime tt = CTime::GetCurrentTime();
        filename =L"./log/" + tt.Format("%Y-%m-%d") + L".log";
        setlocale(LC_CTYPE, ("chs"));
        //�����ļ��Ĵ򿪲���

        CStdioFile outFile(filename, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::typeText);
        CString msLine;
      

        //��ΪLog�ļ�������Ҫ��ÿ��Log��ʱ�����ʱ���ʽ�����ɶ��壬
        //����ĸ�ʽ�磺2010-8-10 Thursday, 15:58:12
        msLine = tt.Format("[%Y-%m-%d %A, %H:%M:%S] ") + msg;
        msLine += "\n\n";

        //���ļ�ĩβ�����¼�¼
        outFile.SeekToEnd();
        outFile.WriteString(msLine);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
        outFile.Close();
    }
    catch (CFileException* fx)
    {
        fx->Delete();
    }
}
//����picture control�ؼ���ͼƬ,   ����ID��picture control�ؼ�ID��
void CBasicDemoDlg::OnPictureSave(UINT ID)
{
    CWnd* bmpShow = GetDlgItem(ID);
    CDC* pdc = bmpShow->GetDC();
    ATL::CImage  imag , imDest;
    CRect rect;

    GetClientRect(&rect);        //��ȡ������С
    bmpShow->GetWindowRect(&rect);
    imag.Create(rect.Width(), rect.Height(), 32);
    ::BitBlt(imag.GetDC(), 0, 0, rect.Width(), rect.Height(), pdc->m_hDC, 0, 0, SRCCOPY);

    CString picturePath;

	HDC hDC = imag.GetDC();

	CDC* pDC = CDC::FromHandle(hDC);

	CBitmap bmp;
    
	bmp.CreateCompatibleBitmap(pDC, m_picWeight, m_picHeight);

	CDC memDC;

	memDC.CreateCompatibleDC(pDC);

	CBitmap* pOld = memDC.SelectObject(&bmp);

	::SetStretchBltMode(memDC.m_hDC, HALFTONE);

	::SetBrushOrgEx(memDC.m_hDC, 0, 0, NULL);

	imag.StretchBlt(memDC.m_hDC, CRect(0, 0, m_picWeight, m_picHeight)/*DestRect*/, CRect(0, 0, imag.GetWidth(), imag.GetHeight())/*SourceRect*/, SRCCOPY);

	//152,200��������Ҫ��ͼ��Ŀ�͸�(������Ϊ��λ)

	HBITMAP  hBitmap = (HBITMAP)memDC.SelectObject(pOld->m_hObject);

	imDest.Attach(hBitmap);// ����λͼ��Դ      
    HRESULT hResult = imDest.Save(L"1.png"); //����ͼƬ
    ReleaseDC(pdc);
    imag.ReleaseDC();
}

void CBasicDemoDlg::OnBnClickedButtonsave()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
  //  OnPictureSave(IDC_STATIC); //�������������picture control�����ͼƬ����̫�ͣ�Ӱ��̫��
    try
    {

      //  m_rotate = 3;
        Sleep(200);  //����ָ��ֳ���ʱ��
        cv::imwrite("1.bmp", orgPic);  //д��ͼƬsrcImage 
    }
    catch (Exception ex)
    {
        CString cstr(ex.msg.c_str());
        WriteLog(cstr);
    }
}


void CBasicDemoDlg::OnBnClickedButtonThreshold()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    CString str;
    GetDlgItemText(IDC_EDIT_Threshold, str);
    m_Threshold = _ttoi(str);
}


void CBasicDemoDlg::OnBnClickedButtonmaxvalue()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    CString str;
    GetDlgItemText(IDC_EDIT_maxvalue, str);
    m_maxvalue = _ttoi(str);
   
}


void CBasicDemoDlg::OnBnClickedButtontoleft()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    m_rotate -= 1;
    if (m_rotate < 0)
        m_rotate = 3;
	GetDlgItem(IDC_STATIC_video)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_video)->ShowWindow(TRUE); 

}


void CBasicDemoDlg::OnBnClickedButtontoright()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    if (m_rotate >= 3)
        m_rotate = -1;
    m_rotate += 1; 
	GetDlgItem(IDC_STATIC_video)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_video)->ShowWindow(TRUE);
}


void CBasicDemoDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    CDialog::OnLButtonDblClk(nFlags, point);
}


void CBasicDemoDlg::OnDblclkStaticVideo()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
  
   /* if (showdiff)
		showdiff = !showdiff;*/
   
    showFullScreen(IDC_STATIC_video);
	GetDlgItem(IDC_STATIC_video)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_video)->ShowWindow(TRUE);
}


void CBasicDemoDlg::OnDblclkDisplayStatic()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
   
    if (showdiff)
    {
        isDiffScFull = true;
        showFullScreen(IDC_DISPLAY_STATIC);
    }
   
	GetDlgItem(IDC_DISPLAY_STATIC)->ShowWindow(FALSE);
	GetDlgItem(IDC_DISPLAY_STATIC)->ShowWindow(TRUE);
}
void CBasicDemoDlg::showFullScreen(int u)
{
    if (!bFullScreen)
    {
      
        bFullScreen = true;
        HideControls();
		//���ؿؼ�
		CPaintDC dc(this);

		
        //��ȡϵͳ��Ļ���
        int g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);

        //��m_struOldWndpl�õ���ǰ���ڵ���ʾ״̬�ʹ���λ�ã��Թ��˳�ȫ����ʹ��
        GetWindowPlacement(&m_struOldWndpl);
        GetDlgItem(u)->GetWindowPlacement(&m_struOldWndpPic);

        //���������ȫ����ʾ�ͻ�����Ӧ�����õĴ��ڴ�С����ҪΪ�˽�����Ҫ��ʾ�Ĵ���߿�Ȳ����ų�����Ļ��
        CRect rectWholeDlg;
        CRect rectClient;
        GetWindowRect(&rectWholeDlg);//�õ���ǰ������ܵ��������Ļ������
        RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);//�õ��ͻ�����������
        ClientToScreen(&rectClient);//���ͻ�����Դ��������תΪ�����Ļ����
        //GetDlgItem(IDC_STATIC_PICSHOW)->GetWindowRect(rectClient);//�õ�PICTURE�ؼ�����

        rectFullScreen.left = rectWholeDlg.left - rectClient.left;
        rectFullScreen.top = rectWholeDlg.top - rectClient.top;
        rectFullScreen.right = rectWholeDlg.right + g_iCurScreenWidth - rectClient.right;
        rectFullScreen.bottom = rectWholeDlg.bottom + g_iCurScreenHeight - rectClient.bottom;

        //���ô��ڶ��������Ϊȫ������׼��������ȫ��״̬
        WINDOWPLACEMENT struWndpl;
        struWndpl.length = sizeof(WINDOWPLACEMENT);
        struWndpl.flags = 0;
        struWndpl.showCmd = SW_SHOW;
        struWndpl.rcNormalPosition = rectFullScreen;
        SetWindowPlacement(&struWndpl);//�ú�������ָ�����ڵ���ʾ״̬����ʾ��Сλ�õȣ������Ǹó�����Ϊ��Ҫ�ĺ���
		//���Ʊ���   
		/*CRect rect;
		GetClientRect(&rect);*/


        //��PICTURE�ؼ���������Ϊȫ����С
        GetDlgItem(u)->MoveWindow(CRect(0, 0, g_iCurScreenWidth, g_iCurScreenHeight));


  //      CPaintDC ddcc(this);
		//CBrush bruDB(RGB(50, 50, 50));//������ɫ  

		//ddcc.FillRect(CRect(0, 0, g_iCurScreenWidth, g_iCurScreenHeight), &bruDB);
    }
    else
    {
        GetDlgItem(u)->SetWindowPlacement(&m_struOldWndpPic);
        SetWindowPlacement(&m_struOldWndpl);
       
        bFullScreen = false;
        isDiffScFull = false;
        HideControls();
    }
	CWnd* pControl = GetDlgItem(IDC_STATIC_video);
	pControl->GetWindowRect(&rect_view);    //����ӿؼ�����Ļ���ꣻ     
	this->ScreenToClient(&rect_view);   //�ӿؼ���Ļ����ӳ�䵽�ؼ��ͻ�����������һ��Ҫ��this

	CRect prect;
	m_videoWnd.GetClientRect(&prect);   //��ȡ����
	FillRect(m_videoWnd.GetDC()->GetSafeHdc(), &prect, CBrush(RGB(50, 50, 50)));  //����ͼ������Ϊ��ɫ
	m_diff_display.GetClientRect(&prect);
	FillRect(m_diff_display.GetDC()->GetSafeHdc(), &prect, CBrush(RGB(50, 50, 50)));  //����ͼ������Ϊ��ɫ
}
void CBasicDemoDlg::HideControls()
{
    GetDlgItem(IDC_OPEN_BUTTON)->ShowWindow(bFullScreen ? SW_HIDE :SW_SHOW);
    GetDlgItem(IDC_CLOSE_BUTTON)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_START_GRABBING_BUTTON)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_STOP_GRABBING_BUTTON)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_SOFTWARE_TRIGGER_CHECK)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_SOFTWARE_ONCE_BUTTON)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    /*GetDlgItem(IDC_SAVE_BMP_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_TIFF_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_PNG_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);
    GetDlgItem(IDC_SAVE_JPG_BUTTON)->EnableWindow(m_bStartGrabbing ? TRUE : FALSE);*/
    GetDlgItem(IDC_EXPOSURE_EDIT)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_GAIN_EDIT)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_FRAME_RATE_EDIT)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_GET_PARAMETER_BUTTON)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_SET_PARAMETER_BUTTON)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_CONTINUS_MODE_RADIO)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_TRIGGER_MODE_RADIO)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);

    GetDlgItem(IDC_BUTTON_Threshold)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_BUTTON_maxvalue)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_BUTTON_save)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_BUTTON_DIFF)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_BUTTON_toleft)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_BUTTON_toright)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_EDIT_Threshold)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_EDIT_maxvalue)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);

    GetDlgItem(IDC_DEVICE_COMBO)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_INIT_STATIC)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_BUTTON_showerror)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_EXPOSURE_STATIC)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_GAIN_STATIC)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_FRAME_RATE_STATIC)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_ENUM_BUTTON)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_STATIC)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_STATIC_dirty)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_IMAGE_GRABBING_STATIC)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_PARAMETER_STATIC)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_DISPLAY_STATIC)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_STATIC_video)->ShowWindow(isDiffScFull ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_STATIC_time)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_EDIT1)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_STATIC_delay)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
    GetDlgItem(IDC_SPIN1)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_BUTTON_move_up)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_BUTTON_move_left)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_BUTTON_move_right)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_BUTTON_move_down)->ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
} 
void CBasicDemoDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    if (bFullScreen)
    {
        lpMMI->ptMaxSize.x = rectFullScreen.Width();
        lpMMI->ptMaxSize.y = rectFullScreen.Height();
        lpMMI->ptMaxPosition.x = rectFullScreen.left;
        lpMMI->ptMaxPosition.y = rectFullScreen.top;
        lpMMI->ptMaxTrackSize.x = rectFullScreen.Width();
        lpMMI->ptMaxTrackSize.y = rectFullScreen.Height();
       // MessageBox(L"OnGetMinMaxInfo");
    }

    CDialog::OnGetMinMaxInfo(lpMMI);
}




VOID CBasicDemoDlg::OnMyEvent(UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message) {

    case WM_DEVICECHANGE:
        switch (wParam)
        {
            PDEV_BROADCAST_HDR broadcastHdr;

        case DBT_DEVICEARRIVAL:

            broadcastHdr = (PDEV_BROADCAST_HDR)lParam;

            if (DBT_DEVTYP_DEVICEINTERFACE == broadcastHdr->dbch_devicetype)
            {
                //if(ytsoftkey.CheckKeyByFindort_2()==0) AfxMessageBox("�����������롣");
            }
            break;

        case DBT_DEVICEREMOVEPENDING:
        case DBT_DEVICEREMOVECOMPLETE://�յ�Ӳ��������Ϣ�󣬵��ü��������������Ƿ���������
        {
            //����ʹ�� CheckKeyByFindort_2���������Ƿ񱻲�����Ҳ����ʹ��������������������������Ƿ񱻲���
            if (ytsoftkey.CheckKeyByFindort_2() != 0)
            {
                MessageBox(L"�������������������˳���",TEXT("����"), MB_OK | MB_ICONERROR);
                OnClose();
            }
              

        }
        break;

        default:
            break;
        }
    default:
        break;
    }
}
LRESULT CBasicDemoDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    // TODO: �ڴ����ר�ô����/����û���
    OnMyEvent(message, wParam, lParam);
    return CDialog::DefWindowProc(message, wParam, lParam);
}


BOOL CBasicDemoDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CPoint mousePoint;                      //���嵱ǰ���������
	GetCursorPos(&mousePoint);              //��ȡ��ǰ����ڴ����е�����ֵ
	ScreenToClient(&mousePoint);
    if (mousePoint.x < rect_view.left || mousePoint.x > rect_view.right || mousePoint.y > rect_view.bottom || mousePoint.y < rect_view.top)
        return 0;  // MessageBox(L"out");
	//int dx = mousePoint.x - (rect_view.TopLeft()).x; //�����������㵽������ʼ��X���������
	//int dy = mousePoint.y - (rect_view.TopLeft()).y; //�����������㵽������ʼ��Y���������

	//���������ֹ��������ж����� �� ���������������ű��� 
	if (zDelta > 0)
	{
        //����������
		//startPoint.x = rc.TopLeft().x - (int)(dx * 0.1);//����Ŵ��ľ�����ʼ������
		//startPoint.y = rc.TopLeft().y - (int)(dy * 0.1);
		//scale *= 1.1;                  //����ÿ�ηŴ�ı���ֵ
        startPoint.x += 40 ;
        startPoint.y += 40 * h_w_float;
		endPoint.x -= 40 ;  //�������ź�ľ����յ�����
		endPoint.y -= 40 * h_w_float;
	}
	else
	{
        //����������
		//startPoint.x = rc.TopLeft().x + (int)(dx * 0.1);//������С��ľ�����ʼ������
		//startPoint.y = rc.TopLeft().y + (int)(dy * 0.1);
		//scale *= 0.9;                //����ÿ����С�ı���ֵ
		startPoint.x -= 40 ;
		startPoint.y -= 40 * h_w_float;
		endPoint.x += 40 ;  //�������ź�ľ����յ�����
		endPoint.y += 40 * h_w_float;
	}
    if (startPoint.x < 0 || startPoint.y < 0)
    {
        startPoint.x = 0;
        startPoint.y = 0;
    }
	//endPoint.x = startPoint.x + (int)(m_picHeight * scale);  //�������ź�ľ����յ�����
	//endPoint.y = startPoint.y + (int)(m_picWeight * scale);
    if (endPoint.x > m_picWeight || endPoint.y > m_picHeight)
    {
        endPoint.x = m_picWeight;
        endPoint.y = m_picHeight;
    }
    if (startPoint.x > endPoint.x - 40 || startPoint.y > endPoint.y - 40 * h_w_float)
    {
        startPoint.x -= 40;
        startPoint.y -= 40 * h_w_float;
		endPoint.x += 40;
		endPoint.y += 40 * h_w_float;
    }
	//Invalidate();   //���»���ͼ��
    /**/
    return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

//
//void CBasicDemoDlg::OnPicNcLButtonDblClk(UINT nHitTest, CPoint point)
//{
//    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//	if (nHitTest == HTCAPTION ) // Ϊ��������˫��  
//		return;
//
//    CDialog::OnNcLButtonDblClk(nHitTest, point);
//}
//#ifdef SETROI
//
//
//
//void CBasicDemoDlg::OnPicLButtonDown(UINT nFlags, CPoint point)
//{
//    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//	m_StartPoint = point;
//
//	m_EndPoint = point;
//
//	m_bLBDown = TRUE;
//
//	SetCapture();//������겶��
//
//    CDialog::OnLButtonDown(nFlags, point);
//}
//void CBasicDemoDlg::OnPicLButtonUp(UINT nFlags, CPoint point)
//{
//	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//	if (this == GetCapture())
//	{
//		ReleaseCapture();
//	}
//
//	m_bLBDown = FALSE;
//
//	Invalidate(FALSE);//���½���   
//
//	CDialog::OnLButtonUp(nFlags, point);
//}
//
//#endif



//void CBasicDemoDlg::OnPicMouseMove(UINT nFlags, CPoint point)
//{
//    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//	if (m_bLBDown)
//	{
//		m_EndPoint = point;
//
//		Invalidate(FALSE);//���½���  
//	}
//    CDialog::OnMouseMove(nFlags, point);
//}


//BOOL CBasicDemoDlg::OnPicEraseBkgnd(CDC* pDC)
//{
//    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//	  //return CDialogEx::OnEraseBkgnd(pDC);����Ҫ�ػ汳��  
//	return TRUE;
//  //  return CDialog::OnEraseBkgnd(pDC);
//}


void CBasicDemoDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    // TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(true);
	CString ss;
    int result = 0;
    Sleep(200);
  //  ((CEdit*)GetDlgItem(IDC_EDIT1))->SetSel(m_editDelay.GetLength(), m_editDelay.GetLength());
	if (pNMUpDown->iDelta == -1) // �����ֵΪ-1 , ˵�������Spin�����µļ�ͷ  
	{
        result = _ttoi(m_editDelay) - 5000;

        if (result < 0)
            result = 0;
        ss.Format(L"%d", result);
	}
	else if (pNMUpDown->iDelta == 1) // �����ֵΪ1, ˵�������Spin�����ϵļ�ͷ  
	{
		result = _ttoi(m_editDelay) + 5000;

		if (result > 9000000)
			result = 9000000;
		ss.Format(L"%d", result);
		
	}
    m_editDelay = ss;
    int nRet = m_pcMyCamera->SetFloatValue("TriggerDelay", result);
	if (MV_OK != nRet)
	{
		return ;
	}
  
	UpdateData(false);
    ((CEdit*)GetDlgItem(IDC_EDIT1))->SetSel(m_editDelay.GetLength(), m_editDelay.GetLength());
	*pResult = 0;
}


void CBasicDemoDlg::OnChangeEdit1()
{
    // TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
    // ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
    // ���������� CRichEditCtrl().SetEventMask()��
    // ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
    UpdateData(TRUE);
	if (m_editDelay.Find('/') >= 0 || m_editDelay.Find('\\') >= 0 || m_editDelay.Find('|') >= 0 ||
        m_editDelay.Find('-') >= 0 || m_editDelay.Find('+') >= 0 || m_editDelay.Find('\"') >= 0 ||
         m_editDelay.Find('*') >= 0 || m_editDelay.Find('<') >= 0 || m_editDelay.Find('<') >= 0)
	{
		int  tem_nEditSize = m_editDelay.GetLength();
        m_editDelay = m_editDelay.Right(tem_nEditSize - 1);
        if (tem_nEditSize == 2)
            m_editDelay = "0";
		UpdateData(FALSE);
		if (m_editDelay.GetLength() > 0)
		{
			//����Edit�����ĩβ,���������������
			((CEdit*)GetDlgItem(IDC_EDIT1))->SetSel(m_editDelay.GetLength(), m_editDelay.GetLength());
		}
	}
	int nRet = m_pcMyCamera->SetFloatValue("TriggerDelay", _ttoi(m_editDelay));
	if (MV_OK != nRet)
	{
		return;
	}
    ((CEdit*)GetDlgItem(IDC_EDIT1))->SetSel(m_editDelay.GetLength(), m_editDelay.GetLength());
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
}

void CBasicDemoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CPoint mousePoint;                      //���嵱ǰ���������
	GetCursorPos(&mousePoint);              //��ȡ��ǰ����ڴ����е�����ֵ
	ScreenToClient(&mousePoint);
	if (mousePoint.x < rect_view.left || mousePoint.x > rect_view.right || mousePoint.y > rect_view.bottom || mousePoint.y < rect_view.top)
		return;  // MessageBox(L"out");
	if (m_bLBDown)
	{
		startPoint.x += point.x - m_StartPoint.x;
		startPoint.y += point.y - m_StartPoint.y;
		endPoint.x += point.x - m_StartPoint.x;  //�������ź�ľ����յ�����
		endPoint.y += point.y - m_StartPoint.y;
	}
    CDialog::OnMouseMove(nFlags, point);
}


void CBasicDemoDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    m_bLBDown = FALSE;
    CDialog::OnLButtonUp(nFlags, point);
}


void CBasicDemoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CPoint mousePoint;                      //���嵱ǰ���������
	GetCursorPos(&mousePoint);              //��ȡ��ǰ����ڴ����е�����ֵ
	ScreenToClient(&mousePoint);
	if (mousePoint.x < rect_view.left || mousePoint.x > rect_view.right || mousePoint.y > rect_view.bottom || mousePoint.y < rect_view.top)
		return;  // MessageBox(L"out");
	m_StartPoint = point;

	//m_EndPoint = point;

	m_bLBDown = TRUE;

	SetCapture();//������겶��
    CDialog::OnLButtonDown(nFlags, point);
}


void CBasicDemoDlg::OnBnClickedButtonmoveup()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_rotate == 0)
	{
		if (endPoint.x + 40 > m_picWeight)
			return;
		startPoint.x += 40;
		endPoint.x += 40;
	}
	else
		if (m_rotate == 1)
		{
			if (startPoint.y - 40 * h_w_float < 0)
				return;
			startPoint.y -= 40 * h_w_float;
			endPoint.y -= 40 * h_w_float;
		}
		else
			if (m_rotate == 2)
			{
				if (startPoint.x - 40 < 0)
					return;
				startPoint.x -= 40;
				endPoint.x -= 40;
			}
			else
			{
				if (endPoint.y + 40 * h_w_float > m_picHeight)
					return;
				startPoint.y += 40 * h_w_float;
				endPoint.y += 40 * h_w_float;
			}

}


void CBasicDemoDlg::OnBnClickedButtonmoveleft()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������if (m_rotate == 0)
	if (m_rotate == 0)
	{
		if (startPoint.y - 40 * h_w_float < 0)
			return;
		startPoint.y -= 40 * h_w_float;
		endPoint.y -= 40 * h_w_float;
	}
	else
		if (m_rotate == 1)
		{
			if (startPoint.x - 40 < 0)
				return;
			startPoint.x -= 40;
			//startPoint.y -= 40 * h_w_float;
			endPoint.x -= 40;  //�������ź�ľ����յ�����
		}
		else
			if (m_rotate == 2)
			{
				if (endPoint.y + 40 * h_w_float > m_picHeight)
					return;
				startPoint.y += 40 * h_w_float;
				endPoint.y += 40 * h_w_float;
			}
			else
			{
				if (endPoint.x + 40 > m_picWeight)
					return;
				startPoint.x += 40;
				endPoint.x += 40;
			}
}


void CBasicDemoDlg::OnBnClickedButtonmoveright()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_rotate == 0)
	{
		if (endPoint.y + 40 * h_w_float > m_picHeight)
			return;
		startPoint.y += 40 * h_w_float;
		endPoint.y += 40 * h_w_float;
	}
	else
		if (m_rotate == 1)
		{
			if (endPoint.x + 40 > m_picWeight)
				return;
			startPoint.x += 40;
			endPoint.x += 40;
		}
		else
			if (m_rotate == 2)
			{
				if (startPoint.y - 40 * h_w_float < 0)
					return;
				startPoint.y -= 40 * h_w_float;
				endPoint.y -= 40 * h_w_float;
			}
			else
			{
				if (startPoint.x - 40 < 0)
					return;
				startPoint.x -= 40;
				endPoint.x -= 40;
			}

}


void CBasicDemoDlg::OnBnClickedButtonmovedown()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_rotate == 0)
	{
		if (startPoint.x - 40 < 0)
			return;
		startPoint.x -= 40;
		endPoint.x -= 40;
	}
	else
		if (m_rotate == 1)
		{
			if (endPoint.y + 40 * h_w_float > m_picHeight)
				return;
			startPoint.y += 40 * h_w_float;
			endPoint.y += 40 * h_w_float;
		}
		else
			if (m_rotate == 2)
			{
				if (endPoint.x + 40 > m_picWeight)
					return;
				startPoint.x += 40;
				endPoint.x += 40;
			}
			else
			{
				if (startPoint.y - 40 * h_w_float < 0)
					return;
				startPoint.y -= 40 * h_w_float;
				endPoint.y -= 40 * h_w_float;
			}
}
