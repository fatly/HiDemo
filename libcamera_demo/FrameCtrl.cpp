#include "FrameCtrl.h"
#include <windows.h>

namespace e
{
	inline double GetTicks(void)
	{
		DWORD64 ret, freq;
		if (QueryPerformanceCounter((LARGE_INTEGER*)&ret) &&
			QueryPerformanceFrequency((LARGE_INTEGER*)&freq))
		{
			return double(ret) / freq;
		}
		else
		{
			return double(::GetTickCount()) / 1000.0;
		}
	}


	CFrameCtrl::CFrameCtrl()
	{
		Reset();
	}

	CFrameCtrl::~CFrameCtrl()
	{

	}

	void CFrameCtrl::Reset(void)
	{
		m_nContinuousSkipCount = 0;
		m_lfRenderFrameCount = 0;
		m_lfSkipedFrameCount = 0;
		m_lfRenderFPS = 0;
		m_lfSkipedFPS = 0;
		m_nThisCycleRenderedFrames = 0;
		m_lfRenderFrameCycleT0 = GetTicks();
		m_LastState = STATE_IDLE;
		m_lfLoad = 1.0;
		m_lfBusyTimeSpan = 0;
		m_lfPrevT0 = GetTicks();
		//15帧每秒
		m_nFramePerSecond = 15;
	}

	CFrameCtrl::State CFrameCtrl::Step(void)
	{
		double t = GetTicks();

		if (m_LastState == STATE_SKIP || m_LastState == STATE_RENDER)
		{
			m_lfBusyTimeSpan += t - m_lfPrevT0;
		}

		double lfRenderFrameSpan = t - m_lfRenderFrameCycleT0;

		// 每秒计算一次FPS和负载
		double lfTotalFrameCount = m_lfRenderFrameCount + m_lfSkipedFrameCount;

		if (lfTotalFrameCount >= m_nFramePerSecond)
		{
			m_lfRenderFrameCycleT0 = t;
			m_lfRenderFPS = m_lfRenderFrameCount / lfRenderFrameSpan;
			m_lfSkipedFPS = m_lfSkipedFrameCount / lfRenderFrameSpan;

			if (m_lfRenderFrameCount > 0)
			{
				double lfNewLoad = m_lfBusyTimeSpan * m_nFramePerSecond / lfRenderFrameSpan / m_lfRenderFrameCount;
				m_lfLoad = lfNewLoad;
			}

			m_lfRenderFrameCount = 0;
			m_lfSkipedFrameCount = 0;
			lfRenderFrameSpan = 0;
			m_lfBusyTimeSpan = 0;
		}
		else if (lfRenderFrameSpan < 0)
		{
			m_lfRenderFrameCycleT0 = t;
			lfRenderFrameSpan = 0;
		}

		m_lfPrevT0 = t;

		double lfFrameShouldBe = lfRenderFrameSpan * m_nFramePerSecond;

		// 如果太快就休息一下, 这里的0.5是经验估计值
		// TODO: 垂直同步时, Load极可能不对
		if (lfFrameShouldBe < m_lfRenderFrameCount + (1.1 - m_lfLoad) * 0.5)
		{
			m_LastState = STATE_IDLE;
			return STATE_IDLE;
		}

		m_nThisCycleRenderedFrames++;
		if (m_nThisCycleRenderedFrames >= m_nFramePerSecond)
		{
			m_nThisCycleRenderedFrames = 0;
		}

		State ret;
		// 容忍, 如果帧数下降小于这个值, 就没必要跳帧
		static const double threshold_a = 2;
		static const int max_continous_skip = 1;

		if (m_nContinuousSkipCount >= max_continous_skip // 限制连跳
			|| lfFrameShouldBe < lfTotalFrameCount + threshold_a) // 容限
		{
			m_lfRenderFrameCount++;
			m_nContinuousSkipCount = 0;
			ret = STATE_RENDER;
		}
		else
		{
			m_nContinuousSkipCount++;
			m_lfSkipedFrameCount++;
			ret = STATE_SKIP;
		}

		m_LastState = ret;
		return ret;
	}
}