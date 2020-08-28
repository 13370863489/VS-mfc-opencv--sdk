
// BasicDemoDlg.h : header file
#pragma once
#include "afxwin.h" 
#include "MvCamera.h"
#include "CvvImage.h"
#include "SoftKey.h"
#define STATUS_ERROR            -1
#define IMAGE_NAME_LEN          256

// CBasicDemoDlg dialog
class CBasicDemoDlg : public CDialog
{
// Construction
public:
	CBasicDemoDlg(CWnd* pParent = NULL);	// Standard constructor

    // Dialog Data
	enum { IDD = IDD_BasicDemo_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

/*ch:�ؼ���Ӧ���� | en:Control corresponding variable*/
private:
    /*ch:��ʼ�� | en:Initialization*/
    CButton m_ctrlOpenButton;                   // ch:���豸 | en:Open device
    CButton m_ctrlCloseButton;                  // ch:�ر��豸 | en:Close device

    /*ch:ͼ��ɼ� | en:Image Acquisition*/
    CButton m_ctrlContinusModeRadio;            // ch:����ģʽ | en:Continus Mode
    CButton m_ctrlTriggerModeRadio;             // ch:����ģʽ | en:Trigger mode
    CButton m_ctrlStartGrabbingButton;          // ch:��ʼץͼ | en:Start grabbing
    CButton m_ctrlStopGrabbingButton;           // ch:����ץͼ | en:Stop grabbing
    CButton m_ctrlSoftwareTriggerCheck;         // ch:���� | en:Software trigger
    BOOL    m_bSoftWareTriggerCheck;
    CButton m_ctrlSoftwareOnceButton;           // ch:����һ�� | en:Software Trigger Once

    /*ch:ͼ�񱣴� | en:Save Image*/
    CButton m_ctrlSaveBmpButton;                // ch:����Ϊbmp | en:Save as bmp
    CButton m_ctrlSaveJpgButton;                // ch:����Ϊjpg | en:Save as jpg

    /*ch:�������û�ȡ | en:Get Parameter Setting*/
    CEdit   m_ctrlExposureEdit;                 // ch:�ع�ʱ�� | en:Exposure Time
    double  m_dExposureEdit;
    CEdit   m_ctrlGainEdit;                     // ch:���� | en:Gain
    double  m_dGainEdit;
    CEdit   m_ctrlFrameRateEdit;                // ch:֡�� | en:Frame Rate
    double  m_dFrameRateEdit;   
    CButton m_ctrlGetParameterButton;           // ch:��ȡ���� | en:Get parameter
    CButton m_ctrlSetParameterButton;           // ch:���ò��� | en:Set parameter
    int m_Height;
    int m_Width;
    /*ch:�豸��ʾ������ | en:Device display drop-down box*/
    CComboBox m_ctrlDeviceCombo;                // ch:ö�ٵ����豸 | en:Enumerated device
    int      m_nDeviceCombo;
    bool showdiff = false;
    cv::Mat srcImage;
    int nFrmNum = 0;
    cv::Mat img_file;
    cv::Mat orgPic;
    IplImage* pFrImg;     //pFrImgΪ��ǰ֡�ĻҶ�ͼ
    IplImage* pFrImgSec;
    IplImage* pBkImg;     //pBkImgΪ��ǰ�����Ҷ�ͼ 
    cv::Mat imresult;
    IplImage* pBkImgTran;    //
    IplImage* pFrImgTran;
    IplImage* pDiff;
    POINT Old;
    IplImage* mhi; // �˶���ʷͼ��//MHI  
    IplImage** buf =NULL;
    IplImage* orient = NULL; // orientation����   m
    IplImage* mask = NULL; // valid orientation mask��Ч�����ͷ   
    IplImage* segmask = NULL; // motion segmentation map   
 //   IplImage* pBkImg = NULL;
    IplImage* motion = NULL;
    IplImage* src;
    void resize();
    const double MHI_DURATION = 0.5;
    const double MAX_TIME_DELTA = 0.5;
    const double MIN_TIME_DELTA = 0.05;
    CvMemStorage* storage; // temporary storage 
    bool isDiffScFull = false;  //ȱ�ݴ������״̬

    //set ROI
    BOOL m_draw_rect;
    CRect rect_view;
    CPoint down_point;
    CRect make_rect;
    CString m_edit_show;
    CRect rect_rect;
    CStatic m_showRectangle;
    CRect m_rect;
    bool m_WriteLog;
/*ch:�ڲ����� | en:Built-in function*/
    //����
    char DevicePath[MAX_PATH];
private:
    /*ch:�ʼʱ�Ĵ��ڳ�ʼ�� | en:Window initialization*/
    int DisplayWindowInitial(void);

    int EnableControls(BOOL bIsCameraReady);
    void ShowErrorMsg(CString csMessage, int nErrorNum);

    /*ch:��ʼ��������� | en:Initialization*/
    int OpenDevice(void);                    // ch:����� | en:Open Device

    /*ch:���á���ȡ�������� | en:Set and get parameters operation*/
    int SetTriggerMode(void);                // ch:���ô���ģʽ | en:Set Trigger Mode
    int GetTriggerMode(void);
    int GetExposureTime(void);               // ch:�����ع�ʱ�� | en:Set Exposure Time
    int SetExposureTime(void); 
    int GetGain(void);                       // ch:�������� | en:Set Gain
    int SetGain(void);
    int GetFrameRate(void);                  // ch:����֡�� | en:Set Frame Rate
    int SetFrameRate(void);
    int GetTriggerSource(void);              // ch:���ô���Դ | en:Set Trigger Source
    int SetTriggerSource(void);
    /*ch:ͼƬ���� | en:Save Image*/
    int SaveImage(void);                     // ch:����ͼƬ | en:Save Image

    /*ch:�ر����ٲ��� | en:Close and deatroy operation*/
    int CloseDevice(void);                   // ch:�ر��豸 | en:Close Device

    // ch:ȥ���Զ�������ظ�ʽ | en:Remove custom pixel formats
    bool RemoveCustomPixelFormats(enum MvGvspPixelType enPixelFormat);
    void ChickKey();
private:
    /*ch:״̬ | en:Status*/
    BOOL  m_bOpenDevice;                        // ch:�Ƿ���豸 | en:Whether to open device
    BOOL  m_bStartGrabbing;                     // ch:�Ƿ�ʼץͼ | en:Whether to start grabbing
    int   m_nTriggerMode;                       // ch:����ģʽ | en:Trigger Mode
    int   m_nTriggerSource;                     // ch:����Դ | en:Trigger Source
    MV_SAVE_IAMGE_TYPE   m_nSaveImageType;      // ch:����ͼ���ʽ | en:Save Image Type

    /*ch:�豸��� | en:Device Related*/
    CMvCamera*      m_pcMyCamera;               // ch:CMyCamera��װ�˳��ýӿ� | en:CMyCamera packed commonly used interface
    HWND  m_hwndDisplay;                        // ch:��ʾ��� | en:Display Handle
    MV_CC_DEVICE_INFO_LIST m_stDevList;         // ch:�豸��Ϣ�б�ṹ������������洢�豸�б�
                                                //en:Device information list structure variable used to store device list

    unsigned char*  m_pSaveImageBuf;         // ch:���ڱ���ͼ��Ļ��� | en:Buffer to save image
    unsigned int    m_nSaveImageBufSize;
    CRITICAL_SECTION m_hSaveImageMux;

    unsigned char*  m_pGrabBuf;            // ch:���ڴ�������ȡͼ��Ļ��� | en:Buffer to get image from driver
    unsigned int    m_nGrabBufSize;
    MV_FRAME_OUT_INFO_EX m_stImageInfo;

    void*           m_hGrabThread;              // ch:ȡ���߳̾�� | en:Grab thread handle
    BOOL            m_bThreadState;
    SoftKey ytsoftkey;
    private:
        BOOL bFullScreen;
        CRect rectFullScreen;
        WINDOWPLACEMENT m_struOldWndpl;//�ṹ�а������йش�������Ļ��λ�õ���Ϣ
        WINDOWPLACEMENT m_struOldWndpPic;//PICTURE�ؼ�����Ļ��λ�õ���Ϣ
public:
    /*ch:��ʼ�� | en:Initialization*/
    afx_msg void OnBnClickedEnumButton();               // ch:�����豸 | en:Find Devices
    afx_msg void OnBnClickedOpenButton();               // ch:���豸 | en:Open Devices
    afx_msg void OnBnClickedCloseButton();              // ch:�ر��豸 | en:Close Devices
   
    /*ch:ͼ��ɼ� | en:Image Acquisition*/
    afx_msg void OnBnClickedContinusModeRadio();        // ch:����ģʽ | en:Continus Mode
    afx_msg void OnBnClickedTriggerModeRadio();         // ch:����ģʽ | en:Trigger Mode
    afx_msg void OnBnClickedStartGrabbingButton();      // ch:��ʼ�ɼ� | en:Start Grabbing
    afx_msg void OnBnClickedStopGrabbingButton();       // ch:�����ɼ� | en:Stop Grabbing
    afx_msg void OnBnClickedSoftwareTriggerCheck();     // ch:���� | en:Software Trigger
    afx_msg void OnBnClickedSoftwareOnceButton();       // ch:����һ�� | en:Software Trigger Execute Once
  
    /*ch:ͼ�񱣴� | en:Image Save*/
    afx_msg void OnBnClickedSaveBmpButton();            // ch:����bmp | en:Save bmp
    afx_msg void OnBnClickedSaveJpgButton();            // ch:����jpg | en:Save jpg
  
    /*ch:�������û�ȡ | en:Parameters Get and Set*/
    afx_msg void OnBnClickedGetParameterButton();       // ch:��ȡ���� | en:Get Parameter
    afx_msg void OnBnClickedSetParameterButton();       // ch:���ò��� | en:Exit from upper right corner
  
    afx_msg void OnClose();
public:
    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    void OnMyEvent(UINT message, WPARAM wParam, LPARAM lParam);
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    int GrabThreadProcess();
    afx_msg void OnBnClickedSaveTiffButton();
    afx_msg void OnBnClickedSavePngButton();
public:
    void DrawPicToHDC(IplImage* img, UINT ID);
    int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
    bool Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData);
    afx_msg void OnBnClickedButtonDiff();
    // ������ʱ
    CString use_time;
    afx_msg void OnSize(UINT nType, int cx, int cy);

    void WriteLog( CString msg);
    afx_msg void OnBnClickedButtonsave();
    void OnPictureSave(UINT ID);
    void  update_mhi(IplImage* img, IplImage* dst, int diff_threshold);
    int m_Threshold = 30;
    afx_msg void OnBnClickedButtonThreshold();
    double m_maxvalue = 200;
    afx_msg void OnBnClickedButtonmaxvalue();
    afx_msg void OnBnClickedButtontoleft();
    int m_rotate = 3;
    afx_msg void OnBnClickedButtontoright();
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    CStatic m_videoWnd;
    afx_msg void OnDblclkStaticVideo();
    afx_msg void OnDblclkDisplayStatic();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
    void showFullScreen(int u);
    void ReadIni();
    //set roi

///#define SETROI 
#ifdef SETROI
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
#endif
    BOOL DoRegisterDeviceInterface();


};
