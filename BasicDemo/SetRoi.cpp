// SetRoi.cpp: 实现文件
//

//#include "pch.h"
#include "SetRoi.h"
#include "afxdialogex.h"
#include "resource.h"
#include <highgui.hpp>
#include "BasicDemoDlg.h"

// SetRoi 对话框
IMPLEMENT_DYNAMIC(SetRoi, CDialogEx)



SetRoi::SetRoi(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_setroi, pParent)
{
	//button_roi_ok.SetImagePath(_T("C:\\Users\\Administrator\\Desktop\\button1.png"), _T("C:\\Users\\Administrator\\Desktop\\button2.png"));
	//button_roi_ok.InitMyButton(true);
}

SetRoi::~SetRoi()
{
}

void SetRoi::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, button_roi_ok);
}


BEGIN_MESSAGE_MAP(SetRoi, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_loadimg, &SetRoi::OnBnClickedButtonloadimg)
	ON_BN_CLICKED(IDOK, &SetRoi::OnBnClickedOk)
END_MESSAGE_MAP()


// SetRoi 消息处理程序


void SetRoi::OnLButtonDown(UINT nFlags, CPoint point)
{


	GetCursorPos(&sel_pos);
	GetDlgItem(IDC_STATIC_image)->GetWindowRect(&pic_rect);

	
	if (pic_rect.PtInRect(sel_pos))    //如果鼠标在源图像显示范围内
	{
		img_rect_width = pic_rect.right - pic_rect.left;  //图像 width
		img_rect_height = pic_rect.bottom - pic_rect.top;  // 图像 height

		ClipCursor(&pic_rect);
		m_bClickEmpty = true;

		OldEmptyBegin = point;

		NowEmptyEnd = point;
	}
	
	CDialogEx::OnLButtonDown(nFlags, point);
}

 CPoint SetRoi::RoiPoint_start, SetRoi::RoiPoint_end;
void SetRoi::OnLButtonUp(UINT nFlags, CPoint point)
{
	
	
	if (m_bClickEmpty) {
		GetDlgItem(IDC_STATIC_image)->GetWindowRect(&nono_rect);
		ScreenToClient(nono_rect);
		m_bClickEmpty = false;
		ClipCursor(NULL);
		NowEmptyEnd = point;
		//换算一下roi的真实大小
		img_rect_width = pic_rect.right - pic_rect.left;  //图像 width
		img_rect_height = pic_rect.bottom - pic_rect.top;  // 图像 height

		rect_width = pic_dlg_rect.right - pic_dlg_rect.left;  //控件 width
		rect_height = pic_dlg_rect.bottom - pic_dlg_rect.top;  // 控件 height

		xscale = img_width / img_rect_width;  // 图片宽比例
		yscale = img_height / img_rect_height; //图片高比例

		SetRoi::RoiPoint_start.x = long((OldEmptyBegin.x - nono_rect.left ) * xscale);
		SetRoi::RoiPoint_start.y = long((OldEmptyBegin.y - nono_rect.top) * yscale);
		SetRoi::RoiPoint_end.x = long((NowEmptyEnd.x - nono_rect.left ) * xscale);
		SetRoi::RoiPoint_end.y = long((NowEmptyEnd.y - nono_rect.top ) * yscale);



	}
	CDialogEx::OnLButtonUp(nFlags, point);
}


void SetRoi::OnMouseMove(UINT nFlags, CPoint point)
{

	if (m_bClickEmpty && pic_rect.PtInRect(sel_pos)) {
		CClientDC dc(this);

		CBrush* pBrush = CBrush::FromHandle((HBRUSH)GetStockObject(DEVICE_DEFAULT_FONT));//NULL_BRUSH

		CBrush* POldBrush = dc.SelectObject(pBrush);

		int nOldMode = dc.SetROP2(R2_NOTXORPEN);

		dc.Rectangle(&CRect(OldEmptyBegin, NowEmptyEnd));

		dc.Rectangle(&CRect(OldEmptyBegin, point));

		NowEmptyEnd = point;

		dc.SelectObject(POldBrush);

		dc.SetROP2(nOldMode);

	}
	CDialogEx::OnMouseMove(nFlags, point);
}


BOOL SetRoi::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_bClickEmpty = false;
	// TODO:  在此添加额外的初始化
	//InitMySkin();
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void SetRoi::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CDialogEx::OnPaint()
	cv::Mat srcimg = cv::imread("1.bmp");
	ShowImage(srcimg, IDC_STATIC_image);

}


void SetRoi::OnBnClickedButtonloadimg()
{

}
void SetRoi::MatToCImage(cv::Mat mat, ATL::CImage& cimage)
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
	img_width = mat.cols;
	img_height = mat.rows;


	//重建cimage
	cimage.Destroy();
	cimage.Create(img_width, img_height, 8 * nChannels);


	//拷贝数据


	uchar* pucRow;                                    //指向数据区的行指针
	uchar* pucImage = (uchar*)cimage.GetBits();        //指向数据区的指针
	int nStep = cimage.GetPitch();                    //每行的字节数,注意这个返回值有正有负


	if (1 == nChannels)                                //对于单通道的图像需要初始化调色板
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


	for (int nRow = 0; nRow < img_height; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < img_width; nCol++)
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
}   //图像转换 mattocimage
/***
showImage 为了实现按照比例显示图片
*/
void SetRoi::ShowImage(cv::Mat img, UINT ID)// ID 是Picture Control控件的ID号     
{
	// CRect   rect;
	ATL::CImage  q;
	MatToCImage(img, q);
	//CWnd* pWnd = NULL;
	//pWnd = GetDlgItem(ID);//获取控件句柄
	GetDlgItem(ID)->GetClientRect(&pic_rect);//获取Picture Control控件的客户区
	float cx = img.cols;
	float cy = img.rows;//获取图片的宽 高
	float k = cy / cx;//获得图片的宽高比

	float dx = pic_rect.Width();
	float dy = pic_rect.Height();//获得控件的宽高比
	float t = dy / dx;//获得控件的宽高比 
	if (k >= t)
	{
		// 高 > 宽
		pic_rect.right = floor(pic_rect.bottom / k);
		pic_rect.left = (dx - pic_rect.right) / 2;
		pic_rect.right = floor(pic_rect.bottom / k) + (dx - pic_rect.right) / 2;
		 // picRect.right += 100;
	}
	else
	{ 
		// 高 < 宽
		pic_rect.bottom = floor(k * pic_rect.right);
		pic_rect.top = (dy - pic_rect.bottom) / 2;
		pic_rect.bottom = floor(k * pic_rect.right) + (dy - pic_rect.bottom) / 2;
	   // picRect.bottom += 100;
	}
	//pWnd->MoveWindow(pic_rect);
	//相关的计算为了让图片在绘图区居中按比例显示，原理很好懂，如果图片很宽但是不高，就上下留有空白区；如果图片很高而不宽就左右留有空白区，并且保持两边空白区一样大
	CDC* pDc = NULL;
	pDc = GetDlgItem(ID)->GetDC();//获取picture control的DC，这是什么玩意我也不知道，百度就行
	int ModeOld = SetStretchBltMode(pDc->m_hDC, STRETCH_HALFTONE);//设置指定设备环境中的位图拉伸模式

	q.StretchBlt(pDc->m_hDC, pic_rect, SRCCOPY);//显示函数
	SetStretchBltMode(pDc->m_hDC, ModeOld);

	ReleaseDC(pDc);//释放指针空间
	//pWnd = NULL;
}

void SetRoi::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}
