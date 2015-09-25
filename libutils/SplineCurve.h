#ifndef __CORE_SPLINECURVE_H__
#define __CORE_SPLINECURVE_H__
#include "private.h"
#include "defines.h"

namespace e
{
	class SplineCurve
	{
		friend class CurveMaker;
	public:
		SplineCurve(void);
		SplineCurve(int pointCount, int sampleCount);
		virtual ~SplineCurve(void);
	public:
		bool SetConfig(int pointCount, int sampleCount);
		void SetPoint(int index, double x, double y);
		void SetSample(int index, double value);
		void GetPoint(int index, double& x, double &y);
		void GetPoint(int index, int& x, int& y, double scale = 255);
		bool SetPointCount(int count, bool reset=true);
		bool SetSampleCount(int count, bool reset=true);
		void UpdatePoint(int index, double x, double y);
		int GetPointCount(void) const;
		int GetSampleCount(void) const;
		int GetPoints(double* results, int count = -1);
		int GetSamples(double* results, int count = -1);
		int GetSamples(uint8* results, int count = -1);
		int GetSample(int index, double scale = 255.0) const;
		double GetSample(double value) const;//value>=0.0 && value<=1.0
		void Calculate(void);
		void Calculate(double* samples, int sampleCount);
		void Reset(void);
		bool IsIdentity(void) const;
		const SplineCurve& operator=(const SplineCurve& other);
	protected:
		bool CheckPoints(void) const;
	protected:
		double* points;
		double* samples;
		int pointCount;
		int sampleCount;
		bool identity;
	};

	class CurveMaker
	{
	public:
		static int Calculate(double*samples
			, const int sampleCount
			, const double* points
			, const int pointCount);

		static int Calculate(double*sampels
			, const int sampleCount
			, const SplineCurve* curve);
	protected:
		static void Plot(double* samples
			, const int sampleCount
			, const double* points
			, const int pointCount
			, int p1
			, int p2
			, int p3
			, int p4);
	};

}

#endif