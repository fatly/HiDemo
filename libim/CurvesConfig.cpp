#include "CurvesConfig.h"
#include <assert.h>
#include <SplineCurve.h>

namespace e
{
	//////////////////////////////////////////////////////////////////////////
	//----------------implements of CurvesConfig ----------------------
	//////////////////////////////////////////////////////////////////////////
	CurvesConfig::CurvesConfig(int pointCount, int sampleCount)
	{
		for (int i = 0; i < CURVE_SIZE; i++)
		{
			curves[i] = new SplineCurve(pointCount, sampleCount);
			assert(curves[i] != 0);
		}

		channel = CURVE_CHANNEL_C;
	}

	CurvesConfig::~CurvesConfig(void)
	{
		for (int i = 0; i < CURVE_SIZE; i++)
		{
			if (curves[i] != 0)
			{
				delete curves[i];
				curves[i] = 0;
			}
		}
	}

	void CurvesConfig::Calculate(void)
	{
		for (int i = 0; i < CURVE_SIZE; i++)
		{
			curves[i]->Calculate();
		}
	}

	void CurvesConfig::CreateSpline(CurveChannel channel, int count, double* points)
	{
		assert(channel >= CURVE_CHANNEL_C && channel <= CURVE_CHANNEL_A);
		assert(count >= 2 && count <= 1024);

		SplineCurve* curve = GetSplineCurve(channel);
		curve->SetPointCount(count);

		for (int i = 0; i < count; i++)
		{
			curve->SetPoint(i, points[2 * i + 0], points[2 * i + 1]);
		}
	}

	void CurvesConfig::Reset(void)
	{
		for (int i = 0; i < CURVE_SIZE; i++)
		{
			if (curves[i] != 0)
			{
				curves[i]->Reset();
			}
		}
	}

	void CurvesConfig::SetSelectCurve(const int index)
	{
		assert(index >= 0 && index <= CURVE_SIZE);
		channel = index;
	}

	SplineCurve* CurvesConfig::GetSplineCurve(const int index) const
	{
		assert(index >= 0 && index <= CURVE_SIZE);
		return curves[index];
	}

	SplineCurve* CurvesConfig::GetSelectSplineCurve(void) const
	{
		return curves[channel];
	}

	int CurvesConfig::GetSelectIndex(void) const
	{
		return channel;
	}

	const CurvesConfig & CurvesConfig::operator=(const CurvesConfig & r)
	{
		if (this != &r)
		{
			for (int i = 0; i < CURVE_SIZE; i++)
			{
				*curves[i] = *(r.curves[i]);
			}
		}

		return *this;
	}
}
