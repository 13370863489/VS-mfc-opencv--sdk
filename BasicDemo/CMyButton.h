#pragma once
#include <afxwin.h>
#include <atlimage.h>
//#include "CvvImage.h"
class CMyButton :
    public CButton
{
public:
//	DECLARE_DYNAMIC(CMyButton)
	//��ť����ͼ��
	ATL::CImage m_imgButton;
	//��ťpng·�����������㣬����������3��״̬
	CString m_strImgPath;
	//�����ڱ���ͼƬ����·����͸��png��Ҫʹ��
	CString m_strImgParentPath;

	//���ð�ť����ͼƬ·���������ڱ���ͼƬ·��
	void SetImagePath(CString strImgPath, CString strParentImgPath);
	//��ʼ����ť����Ҫ�ǵ�����ť��λ�ã�����͸��ɫ
	bool InitMyButton( bool bIsPng/*�Ƿ���PNGͼƬ*/);//int nX/*���Ͻ�X����*/, int nY/*���Ͻ�Y����*/, int nW/*ͼ���*/, int nH/*ͼ���*/,
	//�Ի��ƺ���
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
//	DECLARE_MESSAGE_MAP()

};

