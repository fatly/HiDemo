/***************************************************************************//**
* ��Ȩ���� (C) 
* 
* �ļ����ƣ� 	FFScale.h
* �ļ���ʶ�� 	
* ����ժҪ�� 	ʹ��ffmpeg�е�sws_scale����ͼƬ��ʽת����resize�ķ�װ�ࡣ
* ����˵���� 	��ģ������Գ�����ͼ���ʽ������ת���������ͼ���ʽ����ֱ��ʹ��
*				ffmepg�е�sws_scale��
*
*				����װ��ʹ�÷������£�
*				1������һ��CFFScale����
*				2�����øö����SetAttribut�����������������ͼ��ĸ�ʽ��Scale�㷨��
*				3�����øö����Scale����������Scale������
*
*				ע�⣺��ģ��ٶ�YUVͼ���ʽ���õ��������ڴ�ռ���е�ͼ��洢����
*				ʵ�����������ˣ���ģ�鲻��Ӧ���ֳ��ϡ�
* ��ǰ�汾�� 	V1.0
* ��    �ߣ� 	Cloud
* ������ڣ� 	2011-10-28
*******************************************************************************/
#pragma once

extern "C"
{
	#include "swscale.h"
}

//Scale�㷨
enum SwsAlogrithm
{
	SWS_SA_FAST_BILINEAR	= 0x1,
	SWS_SA_BILINEAR			= 0x2,
	SWS_SA_BICUBIC			= 0x4,
	SWS_SA_X				= 0x8,
	SWS_SA_POINT			= 0x10,
	SWS_SA_AREA				= 0x20,
	SWS_SA_BICUBLIN			= 0x40,
	SWS_SA_GAUSS			= 0x80,
	SWS_SA_SINC				= 0x100,
	SWS_SA_LANCZOS			= 0x200,
	SWS_SA_SPLINE			= 0x400,
};

//��Ƶͼ���ʽ
enum PicFormat
{
	SWS_PF_NONE			= PIX_FMT_NONE,
	SWS_PF_YUV420P		= PIX_FMT_YUV420P,
	SWS_PF_RGB24		= PIX_FMT_RGB24,
	SWS_PF_BGR24		= PIX_FMT_BGR24,
	SWS_PF_ARGB			= PIX_FMT_ARGB,
	SWS_PF_RGBA			= PIX_FMT_RGBA,
	SWS_PF_ABGR			= PIX_FMT_ABGR,
	SWS_PF_BGRA			= PIX_FMT_BGRA,
	SWS_PF_YUV422       = PIX_FMT_YUYV422,
};


class CFFScale
{
public:
	CFFScale(void);
	~CFFScale(void);

	//�����������ͼƬ�����Լ�Scale�㷨
	void SetAttribute(PicFormat srcFormat, PicFormat dstFormat, SwsAlogrithm enAlogrithm = SWS_SA_FAST_BILINEAR);

	//Scale
	BOOL Scale(
		byte *pSrc, int nSrcW, int nSrcH, int nSrcPicth,
		byte *pDst, int nDstW, int nDstH, int nDstPicth
		);

private:

	//��ʼ��
	BOOL Init();

	//����ʼ��
	void DeInit();

	SwsContext*	m_pSwsContext;		//SWS����
	PicFormat m_srcFormat;			//Դ���ظ�ʽ
	PicFormat m_dstFormat;			//Ŀ�����ظ�ʽ
	SwsAlogrithm m_enAlogrithm;		//Resize�㷨

	int m_nSrcW, m_nSrcH;			//Դͼ����
	int m_nSrcPicth;				//Դͼ���һ�����ݵĳ���
	int m_nSrcSlice[4];				//Դͼ�������������ʼ��ַƫ��
	int m_nSrcStride[4];			//Դͼ�������һ�����ݵĳ���

	int m_nDstW, m_nDstH;			//Ŀ��ͼ����
	int m_nDstPicth;				//Ŀ��ͼ���һ�����ݵĳ���
	int m_nDstSlice[4];				//Ŀ��ͼ�������������ʼ��ַƫ��
	int m_nDstStride[4];			//Ŀ��ͼ�������һ�����ݵĳ���

};
