#pragma once

#include "stdafx.h"
#include "cv.h"
#include "CMyButton.h"
// SetRoi 对话框
class SetRoi : public CDialogEx
{
	DECLARE_DYNAMIC(SetRoi)

public:
	SetRoi(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~SetRoi();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_setroi };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	CRect roi_rect;
	CPoint m_StartPoint;//开始点

	CPoint m_EndPoint;//结束点

	BOOL m_bLBDown = FALSE;//鼠标是否按下
	CPoint startPoint;  //矩形起始点坐标（左上角点）     
	CPoint endPoint;    //矩形终点坐标 （右下角点）
	BOOL move_flag;
	BOOL src_flag;
	BOOL up_flag;
	CPoint sel_pos;
	
	CRect pic_rect;  //得出图片缩放后的大小
	double img_rect_width;
	double img_rect_height;
	double rect_width;
	double rect_height;
	double img_width;
	double img_height;
	double xscale;
	double yscale;
	cv::Mat  srcimg;
	CPoint m_ptStart;
	CPoint m_ptPrev;
	afx_msg void OnBnClickedButtonloadimg();
	void MatToCImage(cv::Mat mat, ATL::CImage& cimage);
	void ShowImage(cv::Mat img, UINT ID);// ID 是Picture Control控件的ID号   

	bool m_bClickEmpty; //判断是否点击了空白的地方，以实现拖动框选择 

	CPoint OldEmptyBegin;//点击空白开始的座标

	CPoint NowEmptyEnd;//移动时保存座标

	static CPoint RoiPoint_start, RoiPoint_end;
	CRect pic_dlg_rect; // 图片窗口的大小
	CRect nono_rect; // 图片窗口的大小
	afx_msg void OnBnClickedOk();
	CButton button_roi_ok;
};
