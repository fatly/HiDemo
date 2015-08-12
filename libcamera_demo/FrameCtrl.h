#pragma once

namespace e
{
	class CFrameCtrl
	{
	public:
		CFrameCtrl(void);
		~CFrameCtrl(void);
	public:

		enum State{
			STATE_IDLE,		// ����, ������, �������߼�����
			STATE_RENDER,   // ���Ʋ������߼�����
			STATE_SKIP		// ֻ�����߼�����
		};

		void Reset(void);

		State Step(void);

		void SetFramePerSecond(int nFrameCount)
		{
			m_nFramePerSecond = nFrameCount;
		}

		double GetRenderFPS(void) const
		{
			return m_lfRenderFPS;
		}

		double GetSkipedFPS(void) const
		{
			return m_lfSkipedFPS;
		}

		int GetThisCycleFrame(void) const
		{
			return m_nThisCycleRenderedFrames;
		}

		double GetLoad(void) const
		{
			return m_lfLoad;
		}

	private:
		int    m_nFramePerSecond;
		int    m_nContinuousSkipCount;
		double m_lfRenderFrameCount;
		double m_lfBusyTimeSpan;
		double m_lfSkipedFrameCount;
		double m_lfRenderFPS;
		double m_lfSkipedFPS;
		double m_lfRenderFrameCycleT0;
		State  m_LastState;
		int    m_nThisCycleRenderedFrames;
		double m_lfLoad;	//����
		double m_lfPrevT0;
	};
}