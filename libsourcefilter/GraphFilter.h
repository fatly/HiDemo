#pragma once
namespace e
{
	class CGraphFilter
	{
	public:
		CGraphFilter(void);
		virtual ~CGraphFilter(void);
	public:
		HRESULT Create(void);
		HRESULT Start(void);
		HRESULT Pause(void);
		HRESULT Stop(void);
		HRESULT Destroy(void);
	protected:
		IGraphBuilder* m_pGraphBuilder;
		ICaptureGraphBuilder2* m_pCaptureGraphBuilder2;
		IBaseFilter* m_pAudioSource;
		IBaseFilter* m_pAudioRender;
		IMediaControl* m_pMediaControl;
	};
}
