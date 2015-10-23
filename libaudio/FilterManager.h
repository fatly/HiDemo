#pragma once

namespace e
{
	class CFilterManager
	{
	public:
		CFilterManager(void);
		virtual ~CFilterManager(void);
	protected:
		IGraphBuilder* m_pGraphBuilder;
		IBaseFilter* m_pAudioReader;
	};
}