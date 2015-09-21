#include "Curve.h"
#include <float.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

namespace e
{
	Curve::Curve(void)
	{
		pointCount = 0;
		points = 0;
		sampleCount = 0;
		samples = 0;
		identity = false;
		tables = 0;
	}

	Curve::Curve(int pointCount, int sampleCount)
	{
		this->pointCount = 0;
		this->points = 0;
		this->sampleCount = 0;
		this->samples = 0;
		this->identity = false;
		this->tables = 0;

		SetPointCount(pointCount);
		SetSampleCount(sampleCount);
	}


	Curve::~Curve()
	{
		if (points != 0)
		{
			free(points);
			points = 0;
		}

		if (samples != 0)
		{
			free(samples);
			samples = 0;
		}

		if (tables != 0)
		{
			free(tables);
			tables = 0;
		}
	}

	void Curve::SetPointCount(int pointCount)
	{
		assert(pointCount >= 2 && pointCount <= 1024);

		if (this->pointCount != pointCount)
		{
			this->pointCount = pointCount;

			points = (double*)realloc(points, sizeof(double) * 2 * pointCount);
			assert(points);

			points[0 * 2 + 0] = 0.0;
			points[0 * 2 + 1] = 0.0;

			for (int i = 1; i < pointCount - 1; i++)
			{
				points[i * 2 + 0] = -1.0;
				points[i * 2 + 1] = -1.0;
			}

			points[(pointCount - 1) * 2 + 0] = 1.0;
			points[(pointCount - 1) * 2 + 1] = 1.0;

			identity = true;
		}
	}

	void Curve::SetSampleCount(int sampleCount)
	{
		assert(sampleCount >= 256 && sampleCount < 4096);

		if (this->sampleCount != sampleCount)
		{
			this->sampleCount = sampleCount;

			samples = (double*)realloc(samples, sampleCount * sizeof(double));
			assert(samples);

			for (int i = 0; i < sampleCount; i++)
			{
				samples[i] = (double)i / (double)(sampleCount - 1);
			}

			tables = (byte*)realloc(tables, sampleCount * sizeof(byte));
			assert(tables);

			for (int i = 0; i < sampleCount; i++)
			{
				tables[i] = (byte)ROUND(samples[i] * 255);
			}

			identity = true;
		}
	}

	void Curve::SetPoint(int index, double x, double y)
	{
		assert(index >= 0 && index < pointCount);
		assert(x == -1.0 || x >= 0.0 && x <= 1.0);
		assert(y == -1.0 || y >= 0.0 && y <= 1.0);

		points[index * 2 + 0] = x;
		points[index * 2 + 1] = y;
	}

	void Curve::SetSample(int index, double value)
	{
		assert(index >= 0 && index < sampleCount);
		assert(value >= 0.0 && value <= 1.0);
		samples[index] = value;
	}

	void Curve::Reset(void)
	{
		for (int i = 0; i < sampleCount; i++)
		{
			samples[i] = (double)i / (double)(sampleCount - 1);
			tables[i] = (byte)ROUND(samples[i] * 255);
		}

		SetPoint(0, 0.0, 0.0);
		for (int i = 1; i < pointCount - 1; i++)
		{
			SetPoint(i, -1.0, -1.0);
		}
		SetPoint(pointCount - 1, 1.0, 1.0);

		identity = true;
	}

	void Curve::Calculate(void)
	{
		int p1, p2, p3, p4;
		int count;
		double x, y;

		int* p = (int*)malloc(pointCount * sizeof(int));
		assert(p);

		count = 0;
		for (int i = 0; i < pointCount; i++)
		{
			GetPoint(i, x, y);

			if (x >= 0.0)
			{
				p[count++] = i;
			}
		}

		if (count != 0)
		{
			GetPoint(p[0], x, y);
			int boundary = ROUND(x * (double)(sampleCount - 1));

			for (int i = 0; i < boundary; i++)
			{
				samples[i] = y;
			}

			GetPoint(p[count - 1], x, y);
			boundary = ROUND(x * (double)(sampleCount - 1));

			for (int i = boundary; i < sampleCount; i++)
			{
				samples[i] = y;
			}

			for (int i = 0; i < count - 1; i++)
			{
				p1 = p[MAX(i - 1, 0)];
				p2 = p[i + 0];
				p3 = p[i + 1];
				p4 = p[MIN(i + 2, count - 1)];

				Plot(p1, p2, p3, p4);
			}

			for (int i = 0; i < count; i++)
			{
				GetPoint(p[i], x, y);

				samples[ROUND(x * (double)(sampleCount - 1))] = y;
			}
		}

		//map to tables
		for (int i = 0; i < sampleCount; i++)
		{
			tables[i] = (byte)(ROUND(samples[i] * 255));
		}

		identity = false;
		free(p);
	}

	void Curve::Plot(int p1, int p2, int p3, int p4)
	{
		double y1, y2, slope;
		double x0 = points[p2 * 2 + 0];
		double y0 = points[p2 * 2 + 1];
		double x3 = points[p3 * 2 + 0];
		double y3 = points[p3 * 2 + 1];

		/*
		* the x values of the inner control points are fixed at
		* x1 = 2/3*x0 + 1/3*x3   and  x2 = 1/3*x0 + 2/3*x3
		* this ensures that the x values increase linearily with the
		* parameter t and enables us to skip the calculation of the x
		* values altogehter - just calculate y(t) evenly spaced.
		*/

		double dx = x3 - x0;
		double dy = y3 - y0;

		if (dx <= 0) return;

		if (p1 == p2 && p3 == p4)
		{
			/* No information about the neighbors,
			* calculate y1 and y2 to get a straight line
			*/
			y1 = y0 + dy / 3.0;
			y2 = y0 + dy * 2.0 / 3.0;
		}
		else if (p1 == p2 && p3 != p4)
		{
			/* only the right neighbor is available. Make the tangent at the
			* right endpoint parallel to the line between the left endpoint
			* and the right neighbor. Then point the tangent at the left towards
			* the control handle of the right tangent, to ensure that the curve
			* does not have an inflection point.
			*/
			slope = (points[p4 * 2 + 1] - y0) / (points[p4 * 2 + 0] - x0);

			y2 = y3 - slope * dx / 3.0;
			y1 = y0 + (y2 - y0) / 2.0;
		}
		else if (p1 != p2 && p3 == p4)
		{
			/* see previous case */
			slope = (y3 - points[p1 * 2 + 1]) / (x3 - points[p1 * 2 + 0]);

			y1 = y0 + slope * dx / 3.0;
			y2 = y3 + (y1 - y3) / 2.0;
		}
		else /* (p1 != p2 && p3 != p4) */
		{
			/* Both neighbors are available. Make the tangents at the endpoints
			* parallel to the line between the opposite endpoint and the adjacent
			* neighbor.
			*/
			slope = (y3 - points[p1 * 2 + 1]) / (x3 - points[p1 * 2 + 0]);

			y1 = y0 + slope * dx / 3.0;

			slope = (points[p4 * 2 + 1] - y0) / (points[p4 * 2 + 0] - x0);

			y2 = y3 - slope * dx / 3.0;
		}

		/*
		* finally calculate the y(t) values for the given bezier values. We can
		* use homogenously distributed values for t, since x(t) increases linearily.
		*/
		for (int i = 0; i <= ROUND(dx * (double)(sampleCount - 1)); i++)
		{
			double t = i / dx / (double)(sampleCount - 1);
			double y = y0 * (1 - t) * (1 - t) * (1 - t) +
				3 * y1 * (1 - t) * (1 - t) * t +
				3 * y2 * (1 - t) * t * t +
				y3 * t * t * t;

			int index = i + ROUND(x0 * (double)(sampleCount - 1));

			if (index < sampleCount)
			{
				samples[index] = clamp(y, 0.0, 1.0);
			}
		}
	}

	double Curve::GetSample(double value)
	{
		if (this->identity)
		{
			if (_finite(value))
			{
				return clamp(value, 0.0, 1.0);
			}

			return 0.0;
		}

		if (value > 0.0 && value < 1.0)
		{
			value = value * (sampleCount - 1);
			int index = (int)value;
			double f = value - index;

			return (1.0 - f) * samples[index] + f * samples[index + 1];
		}
		else if (value >= 1.0)
		{
			return samples[sampleCount - 1];
		}
		else
		{
			return samples[0];
		}
	}

	byte Curve::GetSample(int index)
	{
		assert(index >= 0 && index < sampleCount);
		return tables[index];
	}

	void Curve::GetPoint(const int index, int& x, int& y)
	{
		assert(index >= 0 && index < pointCount);
		x = ROUND(points[index * 2 + 0] * 255);
		y = ROUND(points[index * 2 + 1] * 255);
	}

	void Curve::GetPoint(const int index, double& x, double& y)
	{
		assert(index >= 0 && index < pointCount);
		x = points[index * 2 + 0];
		y = points[index * 2 + 1];
	}

	int Curve::GetPointCount(void) const
	{
		return pointCount;
	}

	int Curve::GetSampleCount(void) const
	{
		return sampleCount;
	}

	void Curve::GetSamples(byte* result)
	{
		assert(result);
		memcpy(result, tables, sizeof(byte) * 256);
	}

	const Curve & Curve::operator=(const Curve & r)
	{
		if (this != &r)
		{
			pointCount = r.pointCount;
			points = (double*)realloc(points, sizeof(double) * 2 * pointCount);
			assert(points);

			for (int i = 0; i < pointCount; i++)
			{
				points[i] = r.points[i];
			}

			sampleCount = r.sampleCount;
			samples = (double*)realloc(samples, sizeof(double)*sampleCount);
			assert(samples);

			for (int i = 0; i < sampleCount; i++)
			{
				samples[i] = r.samples[i];
			}

			tables = (byte*)realloc(tables, sizeof(byte) * sampleCount);
			assert(tables);

			for (int i = 0; i < sampleCount; i++)
			{
				tables[i] = r.tables[i];
			}

			identity = r.identity;
		}

		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	//class CurvesConfig 
	//////////////////////////////////////////////////////////////////////////
	CurvesConfig::CurvesConfig(int pointCount, int sampleCount)
	{
		for (int i = 0; i < 5; i++)
		{
			curves[i] = new Curve(pointCount, sampleCount);
			assert(curves[i] != 0);
		}

		channel = CURVE_CHANNEL_C;
	}

	CurvesConfig::~CurvesConfig(void)
	{
		for (int i = 0; i < 5; i++)
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
		for (int i = 0; i < 5; i++)
		{
			curves[i]->Calculate();
		}
	}

	void CurvesConfig::CreateSpline(CurveChannel channel, int count, double* points)
	{
		assert(channel >= CURVE_CHANNEL_C && channel <= CURVE_CHANNEL_A);
		assert(count >= 2 && count <= 1024);

		Curve* curve = GetCurve(channel);
		curve->SetPointCount(count);

		for (int i = 0; i < count; i++)
		{
			curve->SetPoint(i, points[2 * i + 0], points[2 * i + 1]);
		}
	}

	void CurvesConfig::Reset(void)
	{
		for (int i = 0; i < 5; i++)
		{
			if (curves[i] != 0)
			{
				curves[i]->Reset();
			}
		}
	}

	void CurvesConfig::SetSelectCurve(const int channel)
	{
		assert(channel >= CURVE_CHANNEL_C && channel <= CURVE_CHANNEL_A);
		this->channel = channel;
	}

	Curve* CurvesConfig::GetCurve(const int channel) const
	{
		assert(channel >= CURVE_CHANNEL_C && channel <= CURVE_CHANNEL_A);
		return curves[channel];
	}

	Curve* CurvesConfig::GetSelectCurve(void) const
	{
		assert(channel >= CURVE_CHANNEL_C && channel <= CURVE_CHANNEL_A);
		return curves[channel];
	}

	int CurvesConfig::GetSelectChannel(void) const
	{
		assert(channel >= CURVE_CHANNEL_C && channel <= CURVE_CHANNEL_A);
		return channel;
	}

	const CurvesConfig & CurvesConfig::operator=(const CurvesConfig & r)
	{
		if (this != &r)
		{
			for (int i = 0; i < 5; i++)
			{
				*curves[i] = *(r.curves[i]);
			}
		}

		return *this;
	}
}
