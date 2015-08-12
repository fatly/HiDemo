#ifndef __CORE_CURVE_H__
#define __CORE_CURVE_H__

namespace e
{
	class Curve
	{
	public:
		Curve(void);
		Curve(int pointCount, int sampleCount);
		void SetPointCount(int pointCount);
		void SetSampleCount(int sampleCount);
		void SetPoint(int index, double x, double y);
		void SetSample(int index, double value);
		void GetPoint(const int index, int& x, int& y);
		void GetPoint(const int index, double& x, double& y);
		int GetPointCount(void) const;
		int GetSampleCount(void) const;
		double GetSample(double value);
		uint8 GetSample(int index);
		void Reset(void);
		void Calculate(void);
		const Curve & operator=(const Curve & r);
		virtual ~Curve(void);
	private:
		void Plot(int p1, int p2, int p3, int p4);
	protected:
		int			pointCount;
		double*		points;
		int			sampleCount;
		double*		samples;
		bool		identity;
		uint8*		tables;
	};

	typedef enum
	{
		CURVE_CHANNEL_C = 0,		/*< desc="Value" >*/
		CURVE_CHANNEL_B = 1,		/*< desc="Blue"  >*/
		CURVE_CHANNEL_G = 2,		/*< desc="Green" >*/
		CURVE_CHANNEL_R = 3,		/*< desc="Red"   >*/
		CURVE_CHANNEL_A = 4,		/*< desc="Alpha" >*/
	} CurveChannel;

	class CurvesConfig
	{
	public:
		CurvesConfig(int pointCount, int sampleCount);
		void CreateSpline(CurveChannel channel, int count, double* points);
		void Calculate(void);
		void SetSelectCurve(const int channel);
		Curve* GetCurve(const int channel) const;
		Curve* GetSelectCurve(void) const;
		int GetSelectChannel(void) const;
		void Reset(void);
		const CurvesConfig & operator=(const CurvesConfig & r);
		virtual ~CurvesConfig(void);
	public:
		Curve* curves[5];
		int channel;
	};
}

#endif