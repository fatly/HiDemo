#include "Windows.h"
#include "FFScale.h"

#pragma comment(lib, "libavutil.lib")
#pragma comment(lib, "libswscale.lib")
#pragma comment(lib, "libgcc.lib")
#pragma comment(lib, "libmingwex.lib")

//����
CFFScale::CFFScale(void)
{
	m_pSwsContext = NULL;
	m_srcFormat = SWS_PF_NONE;	
	m_dstFormat = SWS_PF_NONE;	
	m_enAlogrithm = SWS_SA_FAST_BILINEAR;

	m_nSrcW = m_nSrcH = 0;			
	m_nSrcPicth = 0;				
	m_nDstW = m_nDstH = 0;
	m_nDstPicth = 0;
	for (int i=0; i<4; i++)
	{
		m_nSrcSlice[i] = -1;
		m_nSrcStride[i] = 0;
		m_nDstSlice[i] = -1;
		m_nDstStride[i] = 0; 
	}
}

//����
CFFScale::~CFFScale(void)
{
	DeInit();
}

/***************************************************************************//**
* �������ƣ�	SetAttribute
* ����������	�����������ͼƬ�����Լ�Scale�㷨��
* ��    ����	srcFormat	>> Դͼ���ʽ��
* ��    ����	dstFormat	>> Ŀ��ͼ���ʽ��
* ��    ����	enAlogrithm	>> Scale�㷨��
* �� �� ֵ��	
* ����˵����	
* �޸�����		�޸���			�޸�����
* ------------------------------------------------------------------------------
* 2011-10-28	Cloud	      	����
*******************************************************************************/
void CFFScale::SetAttribute(PicFormat srcFormat, PicFormat dstFormat, SwsAlogrithm enAlogrithm)
{
	m_srcFormat = srcFormat;
	m_dstFormat = dstFormat;
	m_enAlogrithm = enAlogrithm;
	DeInit();
}

/***************************************************************************//**
* �������ƣ�	Init
* ����������	��ʼ����
* �� �� ֵ��	ִ�гɹ�����TRUE�����򷵻�FALSE��
* ����˵����	
* �޸�����		�޸���			�޸�����
* ------------------------------------------------------------------------------
* 2011-10-28	Cloud	      	����
*******************************************************************************/
BOOL CFFScale::Init()
{
	//����Ԥ�����ù����������ʽ
	if (SWS_PF_NONE == m_srcFormat || SWS_PF_NONE == m_dstFormat)
	{
		return FALSE;
	}

	//����ʼ��
	DeInit();

	//����sws����
	m_pSwsContext = sws_getContext(
		m_nSrcW,
		m_nSrcH,
		(PixelFormat)m_srcFormat,
		m_nDstW,
		m_nDstH,
		(PixelFormat)m_dstFormat,
		(int)m_enAlogrithm,
		NULL, 
		NULL, 
		NULL);
	if (NULL == m_pSwsContext)
	{
		return FALSE;
	}

	//��ʼ��ԴSlice��Stride
	if (m_srcFormat == SWS_PF_YUV420P)
	{
		m_nSrcSlice[0] = 0;
		m_nSrcSlice[1] = m_nSrcW * m_nSrcH;
		m_nSrcSlice[2] = m_nSrcW * m_nSrcH * 5 / 4;
		m_nSrcSlice[3] = -1;

		m_nSrcStride[0] = m_nSrcW;
		m_nSrcStride[1] = m_nSrcW / 2;
		m_nSrcStride[2] = m_nSrcW / 2;
		m_nSrcStride[3] = 0;

	}
	else
	{
		m_nSrcSlice[0] = 0;
		m_nSrcSlice[1] = -1;
		m_nSrcSlice[2] = -1;
		m_nSrcSlice[3] = -1;

		m_nSrcStride[0] = m_nSrcPicth;
		m_nSrcStride[1] = 0;
		m_nSrcStride[2] = 0;
		m_nSrcStride[3] = 0;
	}

	//��ʼ��Ŀ��Slice��Stride
	if (m_dstFormat == SWS_PF_YUV420P)
	{
		m_nDstSlice[0] = 0;
		m_nDstSlice[1] = m_nDstW * m_nDstH;
		m_nDstSlice[2] = m_nDstW * m_nDstH * 5 / 4;
		m_nDstSlice[3] = -1;

		m_nDstStride[0] = m_nDstW;
		m_nDstStride[1] = m_nDstW / 2;
		m_nDstStride[2] = m_nDstW / 2;
		m_nDstStride[3] = 0;

	}
	else
	{
		m_nDstSlice[0] = 0;
		m_nDstSlice[1] = -1;
		m_nDstSlice[2] = -1;
		m_nDstSlice[3] = -1;

		m_nDstStride[0] = m_nDstPicth;
		m_nDstStride[1] = 0;
		m_nDstStride[2] = 0;
		m_nDstStride[3] = 0;
	}
	return TRUE;
}

/***************************************************************************//**
* �������ƣ�	DeInit
* ����������	����ʼ����
* �� �� ֵ��	
* ����˵����	
* �޸�����		�޸���			�޸�����
* ------------------------------------------------------------------------------
* 2011-10-28	Cloud	      	����
*******************************************************************************/
void CFFScale::DeInit()
{
	if (NULL != m_pSwsContext)
	{
		sws_freeContext(m_pSwsContext);
	}
	m_pSwsContext = NULL;
}

/***************************************************************************//**
* �������ƣ�	Scale
* ����������	Scale
* ��    ����	pSrc			>> Դͼ���ڴ���ʼ��ַ��
* ��    ����	nSrcW			>> Դͼ���ȣ�
* ��    ����	nSrcH			>> Դͼ��߶ȣ�
* ��    ����	nSrcPicth		>> Դͼ��ÿ�����ݵĳ��ȣ�YUV��ʽ�ĸ�ֵ�������ɣ���
* ��    ����	pDst			<< Ŀ��ͼ���ڴ���ʼ��ַ��
* ��    ����	nDstW			>> Ŀ��ͼ���ȣ�
* ��    ����	nDstH			>> Ŀ��ͼ��߶ȣ�
* ��    ����	nDstPicth		>> Ŀ��ͼ��ÿ�����ݵĳ��ȣ�YUV��ʽ�ĸ�ֵ�������ɣ���
* �� �� ֵ��	ִ�гɹ�����TRUE�����򷵻�FALSE��
* ����˵����	
* �޸�����		�޸���			�޸�����
* ------------------------------------------------------------------------------
* 2011-10-28	Cloud	      	����
*******************************************************************************/
BOOL CFFScale::Scale(byte *pSrc, int nSrcW, int nSrcH, int nSrcPicth, byte *pDst, int nDstW, int nDstH, int nDstPicth)
{
	//����κβ��������仯������Ҫ���³�ʼ��
	if (nSrcW != m_nSrcW || nSrcH != m_nSrcH || m_nSrcPicth != m_nSrcPicth
		|| nDstW != m_nDstW || nDstH != m_nDstH || m_nDstPicth != m_nDstPicth)
	{
		m_nSrcW = nSrcW;
		m_nSrcH = nSrcH;
		m_nSrcPicth = nSrcPicth;
		m_nDstW = nDstW;
		m_nDstH = nDstH;
		m_nDstPicth = nDstPicth;
		DeInit();
	}

	//���δ�ܳɹ���ʼ��������ʧ��
	if (NULL == m_pSwsContext && !Init())
	{
		return FALSE;
	}

	//������Scale����
	byte *srcSlice[4], *dstSlice[4];
	for (int i=0; i<4; i++)
	{
		srcSlice[i] = m_nSrcSlice[i] < 0 ? NULL : (pSrc + m_nSrcSlice[i]);
		dstSlice[i] = m_nDstSlice[i] < 0 ? NULL : (pDst + m_nDstSlice[i]);
	}
	
	int r = sws_scale
		(
		m_pSwsContext,
		srcSlice,
		m_nSrcStride,
		0,
		m_nSrcH,
		dstSlice,
		m_nDstStride
		);

	return r == m_nDstH;
}