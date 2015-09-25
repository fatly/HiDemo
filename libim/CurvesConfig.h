#ifndef __CORE_CURVESCONFIG_H__
#define __CORE_CURVESCONFIG_H__
#include "Defines.h"
#include "SplineCurve.h"

#define CURVE_SIZE 6
namespace e
{
	typedef enum
	{
		CURVE_CHANNEL_C = 0,		/*< desc="Value" >*/
		CURVE_CHANNEL_B = 1,		/*< desc="Blue"   >*/
		CURVE_CHANNEL_G = 2,		/*< desc="Green" >*/
		CURVE_CHANNEL_R = 3,		/*< desc="Red"    >*/
		CURVE_CHANNEL_A = 4,		/*< desc="Alpha" >*/
	} CurveChannel;

	class SplineCurve;
	class CurvesConfig
	{
	public:
		CurvesConfig(int pointCount, int sampleCount);
		virtual ~CurvesConfig(void);
	public:
		void CreateSpline(CurveChannel channel, int count, double* points);
		void Calculate(void);
		void SetSelectCurve(const int index);
		int GetSelectIndex(void) const;
		SplineCurve* GetSplineCurve(const int index) const;
		SplineCurve* GetSelectSplineCurve(void) const;
		void Reset(void);
		const CurvesConfig & operator=(const CurvesConfig & r);
	public:
		//selected channel index
		int channel;
		SplineCurve* curves[CURVE_SIZE];
	};
}

#endif