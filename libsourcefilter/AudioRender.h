#pragma once

namespace e
{
	class CAudioRender : public CBaseRenderer
	{
	public:
		CAudioRender(__in_opt LPCTSTR pName,         // Debug ONLY description
			__inout_opt LPUNKNOWN pUnk,       // Aggregated owner object
			__inout HRESULT *phr);
		virtual ~CAudioRender(void);
		// Derived classes MUST override these
		virtual HRESULT DoRenderSample(IMediaSample *pMediaSample);
		virtual HRESULT CheckMediaType(const CMediaType *pMediaType);
	protected:
		CMediaType m_mt;
	};
}