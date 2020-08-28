
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
#ifdef SETROI
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
#endif
END_MESSAGE_MAP()

// ch:取流线程 | en:Grabbing thread
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	DisplayWindowInitial();     // ch:显示框初始化 | en:Display Window Initialization
   
    InitializeCriticalSection(&m_hSaveImageMux);
    CRect rect;
    GetClientRect(&rect);//取客户区大小
    Old.x = rect.right - rect.left;
    Old.y = rect.bottom - rect.top;
    m_Threshold = 30;
    m_maxvalue = 200;
    static CFont Bfont;
    Bfont.CreatePointFont(220, _T("宋体"));
    GetDlgItem(IDC_BUTTON_showerror)->SetFont(&Bfont);
	return TRUE;  // return TRUE  unless you set the focus to a control
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

// ch:按钮使能 | en:Enable control
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

    return MV_OK;
}

// ch:最开始时的窗口初始化 | en:Initial window initialization
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

// ch:显示错误信息 | en:Show error message
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

// ch:打开设备 | en:Open Device
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

    // ch:由设备信息创建设备实例 | en:Device instance created by device information
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

    // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
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

// ch:关闭设备 | en:Close Device
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

// ch:获取触发模式 | en:Get Trigger Mode
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

// ch:设置触发模式 | en:Set Trigger Mode
int CBasicDemoDlg::SetTriggerMode(void)
{
    int nRet = m_pcMyCamera->SetEnumValue("TriggerMode", m_nTriggerMode);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    return MV_OK;
}

// ch:获取曝光时间 | en:Get Exposure Time
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

// ch:设置曝光时间 | en:Set Exposure Time
int CBasicDemoDlg::SetExposureTime(void)
{
    // ch:调节这两个曝光模式，才能让曝光时间生效
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

// ch:获取增益 | en:Get Gain
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

// ch:设置增益 | en:Set Gain
int CBasicDemoDlg::SetGain(void)
{
    // ch:设置增益前先把自动增益关闭，失败无需返回
    //en:Set Gain after Auto Gain is turned off, this failure does not need to return
    int nRet = m_pcMyCamera->SetEnumValue("GainAuto", 0);

    return m_pcMyCamera->SetFloatValue("Gain", (float)m_dGainEdit);
}

// ch:获取帧率 | en:Get Frame Rate
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

// ch:设置帧率 | en:Set Frame Rate
int CBasicDemoDlg::SetFrameRate(void)
{
    int nRet = m_pcMyCamera->SetBoolValue("AcquisitionFrameRateEnable", true);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    return m_pcMyCamera->SetFloatValue("AcquisitionFrameRate", (float)m_dFrameRateEdit);
}

// ch:获取触发源 | en:Get Trigger Source
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

// ch:设置触发源 | en:Set Trigger Source
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

// ch:保存图片 | en:Save Image
int CBasicDemoDlg::SaveImage()
{
    if (FALSE == m_bStartGrabbing)
    {
        return STATUS_ERROR;
    }

    // ch:预留BMP头 | en:Reserved BMP header size
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
    pstSaveFileParam.enImageType = m_nSaveImageType; // ch:需要保存的图像类型 | en:Image format to save
    pstSaveFileParam.enPixelType = m_stImageInfo.enPixelType;  // ch:相机对应的像素格式 | en:Camera pixel type
    pstSaveFileParam.nWidth      = m_stImageInfo.nWidth;         // ch:相机对应的宽 | en:Width
    pstSaveFileParam.nHeight     = m_stImageInfo.nHeight;          // ch:相机对应的高 | en:Height
    pstSaveFileParam.nDataLen    = m_stImageInfo.nFrameLen;
    pstSaveFileParam.pData       = m_pGrabBuf;

    // ch:jpg图像质量范围为(50-99], png图像质量范围为[0-9] | en:jpg image nQuality range is (50-99], png image nQuality range is [0-9]
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
//    cvCvtColor(img, buf[last], CV_BGR2GRAY); // 讲帧数据转化为灰度图，buf[0]指向第一帧图，buf[1]第二帧   
//
//    idx2 = (last + 1) % 3; // 每N帧计一小循环，即idx2按1，2，3……N增长，到N后又变成1；index of (last - (N-1))th frame   
//    last = idx2;//更新last=0，1，2，3；0，1，2，3；……   
//
//    silh = buf[idx2];
//    cvAbsDiff(buf[idx1], buf[idx2], silh); // 前后帧求差，silh = abs(buf[idx1] - buf[idx2])；//get difference between frames   
//
//    cvThreshold(silh, silh, diff_threshold, 1, CV_THRESH_BINARY); //将求差后图像利用阈值二值化 //and threshold it   
//    cvUpdateMotionHistory(silh, mhi, timestamp, MHI_DURATION); //去掉"运动时产生的重影，像光流一样的东西"以更新运动历史图像// update MHI   
//                                                                //MHI（motion history image） 中在运动发生的象素点被设置为当前时间戳，而运动发生较久的象素点被清除。   
//                                                                //mhi为二值历史图   
//    // 将二值历史图mhi转化成灰度图，再转化成rgb图//convert MHI to blue 8u image   
//    cvCvtScale(mhi, mask, 255. / MHI_DURATION,
//        (MHI_DURATION - timestamp) * 255. / MHI_DURATION);// 将二值历史图mhi转化成灰度图，
//                                                        //timestamp=1时刚好将历史轮廓消掉
//    cvZero(dst);
//    cvSplit(mask, 0, 0, 0, dst);//从几个单通道数组组合成多通道数组,即将灰度图转化为蓝色图，按BGR顺序正好是蓝色
//
//    // 计算运动梯度方向，和方向标，calculate motion gradient orientation and valid orientation mask   
//    cvCalcMotionGradient(mhi, mask, orient, MAX_TIME_DELTA, MIN_TIME_DELTA, 3);
//
//    if (!storage)
//        storage = cvCreateMemStorage(0);
//    else
//        cvClearMemStorage(storage);
//
//    // 获得各个运动部件（将整个运动分成几个独立的运动）存储在storage里，segment motion: get sequence of motion components   
//    //segmask用于存放被标记的运动图像映射图？// segmask is marked motion components map. It is not used further   
//    seq = cvSegmentMotion(mhi, segmask, storage, timestamp, MAX_TIME_DELTA);
//    /*mhi
//    运动历史图像
//    seg_mask
//    发现应当存储的 mask 的图像, 单通道, 32bits， 浮点数.
//    storage
//    包含运动连通域序列的内存存储仓
//    timestamp
//    当前时间，毫秒单位
//    seg_thresh
//    分割阈值，推荐等于或大于运动历史“每步”之间的间隔。
//    */
//
//    //cvNamedWindow( "segmask", 1 );   
//    //cvShowImage( "segmask", segmask );   
//    // 遍历所有运动部件iterate through the motion components,   
//    // One more iteration (i == -1) corresponds to the whole image (global motion)  
//    int personCount = 0;
//    for (i = 0; i < seq->total; i++)
//    {
//
//        if (i < 0) { // case of the whole image   
//            comp_rect = cvRect(0, 0, size.width, size.height);
//            color = CV_RGB(255, 255, 255);//白色表示整体运动图   
//            magnitude = 100;//指明圆半径   
//        }
//        else { // i-th motion component   
//            comp_rect = ((CvConnectedComp*)cvGetSeqElem(seq, i))->rect;
//            // if( comp_rect.width + comp_rect.height < 70 ) // 忽略很小的连通区域reject very small components   
//            //     continue;   
//            if (comp_rect.height < 30 || comp_rect.width < 10)
//                continue;
//            color = CV_RGB(255, 0, 0);//红色表示部件运动图   
//            magnitude = 20;//指明圆半径   
//            personCount++;
//        }
//
//        //选择感兴趣的运动部件，是通过和矩形comp_rect匹配得到的， select component ROI（感兴趣区域）   
//        cvSetImageROI(silh, comp_rect);//基于给定的矩形comp_rect设置图像的 ROI（感兴趣区域）   
//        cvSetImageROI(mhi, comp_rect);
//        cvSetImageROI(orient, comp_rect);
//        cvSetImageROI(mask, comp_rect);
//
//        // 计算方向（角度）calculate orientation   
//        angle = cvCalcGlobalOrientation(orient, mask, mhi, timestamp, MHI_DURATION);//计算一个选择部件的运动方向   
//        angle = 360.0 - angle;  // adjust for images with top-left origin   
//
//        count = cvNorm(silh, 0, CV_L1, 0); // 计算包围轮廓的最小矩形所需点的个数calculate number of points within silhouette（轮廓） ROI   
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
//        cvCircle(img, center, cvRound(magnitude * 1.2), color, 3, CV_AA, 0);    //cvRound: 对一个double型的数进行四舍五入
//        cvLine(img, center, cvPoint(cvRound(center.x + magnitude * cos(angle * CV_PI / 180)),
//            cvRound(center.y - magnitude * sin(angle * CV_PI / 180))), color, 3, CV_AA, 0);
//    }
// //   std::cout << "视频中有： " << personCount << " 个人" << std::endl;
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
    cimg.CopyOf(img); // 复制图片
    cimg.DrawToHDC(hDC, &rect); // 将图片绘制到显示控件的指定区域内
    ReleaseDC(pDC);
}
/*

*/
bool CBasicDemoDlg::Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData)
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
        return false;
    }

    if (NULL == srcImage.data)
    {
        return false;
    }


    //save converted image in a local file
    try {

       
        if (showdiff)
        {
            src = &IplImage(img_file);
           
            //如果是第一帧，需要申请内存，并初始化
            if (nFrmNum == 0)
            {

                pBkImg = cvCreateImage(cvSize(pstImageInfo->nWidth, pstImageInfo->nHeight), IPL_DEPTH_8U, 1);
                pFrImg = cvCreateImage(cvSize(pstImageInfo->nWidth, pstImageInfo->nHeight), IPL_DEPTH_8U, 1);
                pFrImgSec = cvCreateImage(cvSize(pstImageInfo->nWidth, pstImageInfo->nHeight), IPL_DEPTH_8U, 1);
               // pBkImgTran = cvCreateImage(cvSize(pstImageInfo->nWidth, pstImageInfo->nHeight), IPL_DEPTH_8U, 1);
                nFrmNum = 1;
            }
            else //是否进行播放，而不进行检测
            {
                
                //pFrImg为当前帧的灰度图
                cvCvtColor(&IplImage(srcImage) , pFrImg, CV_BGR2GRAY);  //1 原图像 2  输出图像  灰度处理
                cvCvtColor(src, pFrImgSec, CV_BGR2GRAY);
                WriteLog(L"图像做差");
                cvAbsDiff(pFrImg, pFrImgSec, pBkImg);    //图像做差
                cvThreshold(pBkImg, pBkImg, m_Threshold,m_maxvalue, CV_THRESH_BINARY);  //进行阈值处理 二值化

               // cvThreshold(pBkImg, pBkImg, 30, 1, CV_THRESH_BINARY);//将求差后图像利用阈值二值化 //and threshold it   
                CvSize size = cvSize(pBkImg->width, pBkImg->height); // get current frame size   
               // IplImage*  mhi = cvCreateImage(size, IPL_DEPTH_32F, 1);
                imresult = cv::cvarrToMat(pBkImg);
                // 4.腐蚀  
                Mat kernel_erode = getStructuringElement(MORPH_RECT, Size(3, 3));
                Mat kernel_dilate = getStructuringElement(MORPH_RECT, Size(18, 18));
                erode(imresult, imresult, kernel_erode);
               
                // 5.膨胀  
               // dilate(imresult, imresult, kernel_dilate);

                vector<vector<Point>> contours;
                findContours(imresult, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
                // 在result上绘制轮廓
                drawContours(srcImage, contours, -1, Scalar(0, 200, 255), 2);  //原图上
                // 7.查找正外接矩形  
                vector<Rect> boundRect(contours.size());
                for (int i = 0; i < contours.size(); i++)
                {
                    boundRect[i] = boundingRect(contours[i]);
                    // 在result上绘制正外接矩形
                    rectangle(srcImage, boundRect[i], Scalar(0, 255, 0), 2);  //在原图上画矩形
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

                rotate(imresult, imresult, m_rotate);
                WriteLog(L"图像显示");
                DrawPicToHDC(&(IplImage(imresult)), IDC_DISPLAY_STATIC);  // 显示处理图
                
            }
        }
        //图像旋转 
        if (!isDiffScFull)  //如果右侧全屏，则停止显示左侧图像
        {
            rotate(srcImage, srcImage, m_rotate);
            DrawPicToHDC(&(IplImage(srcImage)), IDC_STATIC_video);  //左侧窗口
        }
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        CString str;
        str.Format(_T("用时：%f秒"), duration);


        CStatic * pStatic = (CStatic*)GetDlgItem(IDC_STATIC_time);
        pStatic->SetWindowText(str);
      
        //srcImage.release();
        
    }
    catch (cv::Exception& ex) {

          //  CBasicDemoDlg lg = new CBasicDemoDlg();
            CString cstr(ex.msg.c_str());
           WriteLog(L"ERROR:  " + __LINE__ + cstr);
        
    }

   // srcImage.release();

    return true;
}

int CBasicDemoDlg::GrabThreadProcess()
{
    // ch:从相机中获取一帧图像大小 | en:Get size of one frame from camera
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
        WriteLog(L"准备开始进入wile循环");
        MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
        MV_DISPLAY_FRAME_INFO stDisplayInfo = { 0 };
        bool bConvertRet = false;
        while (m_bThreadState)
        {
            WriteLog(L"1");
            EnterCriticalSection(&m_hSaveImageMux);
            WriteLog(L"2");
            nRet = m_pcMyCamera->GetOneFrameTimeout(m_pGrabBuf, m_nGrabBufSize, &stImageInfo, 1000);
            WriteLog(L"3");
            if (nRet == MV_OK)
            {
                WriteLog(L"4");
                memcpy(&m_stImageInfo, &stImageInfo, sizeof(MV_FRAME_OUT_INFO_EX));
            }
            WriteLog(L"5");
            LeaveCriticalSection(&m_hSaveImageMux);

            if (nRet == MV_OK)
            {
                if (RemoveCustomPixelFormats(stImageInfo.enPixelType))
                {
                    continue;
                }
				/*  stDisplayInfo.hWnd = m_hwndDisplay;
				  stDisplayInfo.pData = m_pGrabBuf;
				  stDisplayInfo.nDataLen = stImageInfo.nFrameLen;
				  stDisplayInfo.nWidth = stImageInfo.nWidth;
				  stDisplayInfo.nHeight = stImageInfo.nHeight;
				  stDisplayInfo.enPixelType = stImageInfo.enPixelType;*/

                //转换为MAT 格式并显示
                WriteLog(L"准备进入convert2Mat");
                bConvertRet = Convert2Mat(&stImageInfo, m_pGrabBuf);
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
        CBasicDemoDlg lg = new CBasicDemoDlg();
        CString cstr(ex.msg.c_str());
        lg.WriteLog(L"ERROR:  " + __LINE__ +  cstr);
    }
    return MV_OK;
}

// convert data stream in Mat format

// ch:按下查找设备按钮:枚举 | en:Click Find Device button:Enumeration 
void CBasicDemoDlg::OnBnClickedEnumButton()
{
    CString strMsg;
    CString strIndex;
    m_ctrlDeviceCombo.ResetContent();
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet)
    {
        return;
    }

    // ch:将值加入到信息列表框中并显示出来 | en:Add value to the information list box and display
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
            strIndex.Format(_T("[%d]号相机"), i);
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
            strIndex.Format(_T("[%d]号相机"), i);
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

    return;
}

// ch:按下打开设备按钮：打开设备 | en:Click Open button: Open Device
void CBasicDemoDlg::OnBnClickedOpenButton()
{
    int nRet = OpenDevice();
    if (MV_OK != nRet)
    {
        ShowErrorMsg(TEXT("Open Fail"), nRet);
        return;
    }

    OnBnClickedGetParameterButton(); // ch:获取参数 | en:Get Parameter
    EnableControls(TRUE);
    MVCC_INTVALUE_EX height = { 0 }, width = { 0 };
	m_pcMyCamera->GetIntValue("Height", &height);
    m_pcMyCamera->GetIntValue("Width", &width);
    m_Height = height.nCurValue;
    m_Width = width.nCurValue;
    return;
}

// ch:按下关闭设备按钮：关闭设备 | en:Click Close button: Close Device
void CBasicDemoDlg::OnBnClickedCloseButton()
{
    OnBnClickedStopGrabbingButton();
    CloseDevice();
    EnableControls(TRUE);

    return;
}

// ch:按下连续模式按钮 | en:Click Continues button
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

// ch:按下触发模式按钮 | en:Click Trigger Mode button
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

// ch:按下开始采集按钮 | en:Click Start button
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

// ch:按下结束采集按钮 | en:Click Stop button
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
      //  WaitForSingleObject(m_hGrabThread, INFINITE);   //这个地方总是在等待，程序会卡死，所以注释掉
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

// ch:按下获取参数按钮 | en:Click Get Parameter button
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

// ch:按下设置参数按钮 | en:Click Set Parameter button
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

// ch:按下软触发按钮 | en:Click Software button
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

// ch:按下软触发一次按钮 | en:Click Execute button
void CBasicDemoDlg::OnBnClickedSoftwareOnceButton()
{
    if (TRUE != m_bStartGrabbing)
    {
        return;
    }

    int nRet = m_pcMyCamera->CommandExecute("TriggerSoftware");

    return;
}

// ch:按下保存bmp图片按钮 | en:Click Save BMP button
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

// ch:按下保存jpg图片按钮 | en:Click Save JPG button
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

// ch:右上角退出 | en:Exit from upper right corner
void CBasicDemoDlg::OnClose()
{
    OnBnClickedStopGrabbingButton();
  //  CloseDevice();
    Sleep(1000);
    PostQuitMessage(0);
    CloseDevice();

    DeleteCriticalSection(&m_hSaveImageMux);
    CDialog::OnClose();
}

BOOL CBasicDemoDlg::PreTranslateMessage(MSG* pMsg)
{
  /*  if (pMsg->message == WM_KEYDOWN&&pMsg->wParam == VK_ESCAPE)
    {
        return TRUE;
    }

    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        return TRUE;
    }*/
   
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
    img_file = imread("1.bmp");
 //   buf = NULL;
    // temporary images    
  ///  mhi = NULL; // 运动历史图像//MHI   
   
   /* if (srcImage.size() != 0)
        srcImage.release();*/
}

void CBasicDemoDlg::resize()
{
    float fsp[2];
    POINT Newp; //获取现在对话框的大小
    CRect recta;
    GetClientRect(&recta);     //取客户区大小  
    Newp.x = recta.right - recta.left;
    Newp.y = recta.bottom - recta.top;
    fsp[0] = (float)Newp.x / Old.x;
    fsp[1] = (float)Newp.y / Old.y;
    CRect Rect;
    int woc;
    CPoint OldTLPoint, TLPoint; //左上角
    CPoint OldBRPoint, BRPoint; //右下角
    HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件  
    while (hwndChild)
    {
        woc = ::GetDlgCtrlID(hwndChild);//取得ID
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

}
void CBasicDemoDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED) {
        resize();
    }
}

void CBasicDemoDlg::WriteLog( CString msg)
{
    try
    {
        setlocale(LC_CTYPE, ("chs"));
        //设置文件的打开参数
        CStdioFile outFile(L"log.log", CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::typeText);
        CString msLine;
        CTime tt = CTime::GetCurrentTime();

        //作为Log文件，经常要给每条Log打时间戳，时间格式可自由定义，
        //这里的格式如：2010-June-10 Thursday, 15:58:12
        msLine = tt.Format("[%Y-%B-%d %A, %H:%M:%S] ") + msg;
        msLine += "\n\n";

        //在文件末尾插入新纪录
        outFile.SeekToEnd();
        outFile.WriteString(msLine);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
        outFile.Close();
    }
    catch (CFileException* fx)
    {
        fx->Delete();
    }
}
//保存picture control控件的图片,   参数ID是picture control控件ID号
void CBasicDemoDlg::OnPictureSave(UINT ID)
{
    CWnd* bmpShow = GetDlgItem(ID);
    CDC* pdc = bmpShow->GetDC();
    ATL::CImage  imag , imDest;
    CRect rect;

    GetClientRect(&rect);        //获取画布大小
    bmpShow->GetWindowRect(&rect);
    imag.Create(rect.Width(), rect.Height(), 32);
    ::BitBlt(imag.GetDC(), 0, 0, rect.Width(), rect.Height(), pdc->m_hDC, 0, 0, SRCCOPY);

    CString picturePath;

	HDC hDC = imag.GetDC();

	CDC* pDC = CDC::FromHandle(hDC);

	CBitmap bmp;
    
	bmp.CreateCompatibleBitmap(pDC,m_Width, m_Height);

	CDC memDC;

	memDC.CreateCompatibleDC(pDC);

	CBitmap* pOld = memDC.SelectObject(&bmp);

	::SetStretchBltMode(memDC.m_hDC, HALFTONE);

	::SetBrushOrgEx(memDC.m_hDC, 0, 0, NULL);

	imag.StretchBlt(memDC.m_hDC, CRect(0, 0, m_Width, m_Height)/*DestRect*/, CRect(0, 0, imag.GetWidth(), imag.GetHeight())/*SourceRect*/, SRCCOPY);

	//152,200就是你想要的图像的宽和高(以像素为单位)

	HBITMAP  hBitmap = (HBITMAP)memDC.SelectObject(pOld->m_hObject);

	imDest.Attach(hBitmap);// 载入位图资源      
    HRESULT hResult = imDest.Save(L"1.png"); //保存图片
    ReleaseDC(pdc);
    imag.ReleaseDC();
}

void CBasicDemoDlg::OnBnClickedButtonsave()
{
    // TODO: 在此添加控件通知处理程序代码
  //  OnPictureSave(IDC_STATIC); //用这个方法保存picture control里面的图片像素太低，影响太大
    cv::imwrite("1.bmp", srcImage);  //写入图片srcImage 
}


void CBasicDemoDlg::OnBnClickedButtonThreshold()
{
    // TODO: 在此添加控件通知处理程序代码
    CString str;
    GetDlgItemText(IDC_EDIT_Threshold, str);
    m_Threshold = _ttoi(str);
}


void CBasicDemoDlg::OnBnClickedButtonmaxvalue()
{
    // TODO: 在此添加控件通知处理程序代码
    CString str;
    GetDlgItemText(IDC_EDIT_maxvalue, str);
    m_maxvalue = _ttoi(str);
   
}


void CBasicDemoDlg::OnBnClickedButtontoleft()
{
    // TODO: 在此添加控件通知处理程序代码
    m_rotate -= 1;
    if (m_rotate < 0)
        m_rotate = 3;
}


void CBasicDemoDlg::OnBnClickedButtontoright()
{
    // TODO: 在此添加控件通知处理程序代码
    if (m_rotate >= 3)
        m_rotate = -1;
    m_rotate += 1; 
  
}


void CBasicDemoDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    CDialog::OnLButtonDblClk(nFlags, point);
}


void CBasicDemoDlg::OnDblclkStaticVideo()
{
    // TODO: 在此添加控件通知处理程序代码
    if (showdiff)
        showdiff = !showdiff;
    showFullScreen(IDC_STATIC_video);
  
}


void CBasicDemoDlg::OnDblclkDisplayStatic()
{
    // TODO: 在此添加控件通知处理程序代码
   
    if (showdiff)
    {
        isDiffScFull = true;
        showFullScreen(IDC_DISPLAY_STATIC);
    }
   
   
}
void CBasicDemoDlg::showFullScreen(int u)
{
    if (!bFullScreen)
    {
      
        bFullScreen = true;

        //获取系统屏幕宽高
        int g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);
        int g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);

        //用m_struOldWndpl得到当前窗口的显示状态和窗体位置，以供退出全屏后使用
        GetWindowPlacement(&m_struOldWndpl);
        GetDlgItem(u)->GetWindowPlacement(&m_struOldWndpPic);

        //计算出窗口全屏显示客户端所应该设置的窗口大小，主要为了将不需要显示的窗体边框等部分排除在屏幕外
        CRect rectWholeDlg;
        CRect rectClient;
        GetWindowRect(&rectWholeDlg);//得到当前窗体的总的相对于屏幕的坐标
        RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);//得到客户区窗口坐标
        ClientToScreen(&rectClient);//将客户区相对窗体的坐标转为相对屏幕坐标
        //GetDlgItem(IDC_STATIC_PICSHOW)->GetWindowRect(rectClient);//得到PICTURE控件坐标

        rectFullScreen.left = rectWholeDlg.left - rectClient.left;
        rectFullScreen.top = rectWholeDlg.top - rectClient.top;
        rectFullScreen.right = rectWholeDlg.right + g_iCurScreenWidth - rectClient.right;
        rectFullScreen.bottom = rectWholeDlg.bottom + g_iCurScreenHeight - rectClient.bottom;

        //设置窗口对象参数，为全屏做好准备并进入全屏状态
        WINDOWPLACEMENT struWndpl;
        struWndpl.length = sizeof(WINDOWPLACEMENT);
        struWndpl.flags = 0;
        struWndpl.showCmd = SW_SHOW;
        struWndpl.rcNormalPosition = rectFullScreen;
        SetWindowPlacement(&struWndpl);//该函数设置指定窗口的显示状态和显示大小位置等，是我们该程序最为重要的函数

        //将PICTURE控件的坐标设为全屏大小
        GetDlgItem(u)->MoveWindow(CRect(0, 0, g_iCurScreenWidth, g_iCurScreenHeight));
    }
    else
    {

        SetWindowPlacement(&m_struOldWndpl);
        GetDlgItem(u)->SetWindowPlacement(&m_struOldWndpPic);
        bFullScreen = false;
        isDiffScFull = false;
    }
}

void CBasicDemoDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if (bFullScreen)
    {
        lpMMI->ptMaxSize.x = rectFullScreen.Width();
        lpMMI->ptMaxSize.y = rectFullScreen.Height();
        lpMMI->ptMaxPosition.x = rectFullScreen.left;
        lpMMI->ptMaxPosition.y = rectFullScreen.top;
        lpMMI->ptMaxTrackSize.x = rectFullScreen.Width();
        lpMMI->ptMaxTrackSize.y = rectFullScreen.Height();
    }

    CDialog::OnGetMinMaxInfo(lpMMI);
}
#ifdef SETROI
char arr[100];
void CBasicDemoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    if (m_draw_rect == false && point.x > rect_view.left && point.y < rect_view.bottom && point.y > rect_view.top && point.x < rect_view.right)
    {
        down_point = point;
        //  KillTimer(4);
        make_rect.left = point.x;
        make_rect.top = point.y;
        sprintf(arr, "x:%d && y:%d\n\r", point.x, point.y);
        //	MessageBox(arr);
        m_edit_show = arr;
        m_draw_rect = true;
        UpdateData(FALSE);

    }


    CDialog::OnLButtonDown(nFlags, point);
}


void CBasicDemoDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if (m_draw_rect == true && point.x > rect_view.left && point.y < rect_view.bottom && point.y > rect_view.top && point.x < rect_view.right)
    {
        //	CPoint mouse_up = point;
        m_draw_rect = false;
        rect_rect = make_rect;
        //SetTimer(4,10,NULL);
    }
    CDialog::OnLButtonUp(nFlags, point);
}


void CBasicDemoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    if (m_draw_rect)
    {
        if (abs(point.x - down_point.x) > 10 || abs(point.y - down_point.y) > 10) //矩形区域过小则不处理
        {
            if (point.x <= rect_view.left)
                make_rect.right = rect_view.left;
            else
                if (point.x >= rect_view.right)
                    make_rect.right = rect_view.right;
                else
                    make_rect.right = point.x;

            if (point.y <= rect_view.top)
                make_rect.bottom = rect_view.top;
            else
                if (point.y >= rect_view.bottom)
                    make_rect.bottom = rect_view.bottom;
                else
                    make_rect.bottom = point.y;
            sprintf(arr, "\n\r\txx:%d $$ yy:%d\n\r", make_rect.right, make_rect.bottom);
            m_edit_show += arr;
            UpdateData(FALSE);
            //	pFrame = cvQueryFrame( g_capture );
                //CvvImage m_CvvImage;  
                //m_CvvImage.CopyOf(pFrame,1); //复制该帧图像    
                //m_CvvImage.DrawToHDC(hDC, &rect); //显示到设备的矩形框内
                //pdc->Rectangle(m_rect);
            //	InvalidateRect(&rect,false); //刷新控件区域
            CDC* pdc = GetDlgItem(IDC_STATIC_video)->GetDC();// GetWindowDC();   
            pdc->SelectStockObject(NULL_BRUSH);
            SetRect(&m_rect, abs(make_rect.left - rect_view.left), abs(make_rect.top - rect_view.top), abs(make_rect.right - rect_view.left), abs(make_rect.bottom - rect_view.top));
            //InvalidateRect(&rect,TRUE); //刷新控件区域
            pdc->Rectangle(m_rect /*CRect( make_rect.left - rect_view.left , make_rect.top - rect_view.top , make_rect.right - rect_view.left , make_rect.bottom - rect_view.top )*/);
            ReleaseDC(pdc);
        }
        else
        {
            make_rect.left = down_point.x;
            make_rect.top = down_point.y;
            make_rect.right = down_point.x + 10;
            make_rect.bottom = down_point.y + 10;
        }
    }
        //  CPoint mouse_up = point;
    CDialog::OnMouseMove(nFlags, point);
}
#endif