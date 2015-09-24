#ifndef __CORE_XCURVE_H__
#define __CORE_XCURVE_H__
#include "private.h"
#include "defines.h"

namespace e
{
	class XCurveMaker
	{
	public:
		static void Plot(double* points
			, int pointCount
			, double* samples
			, int sampleCount
			, int p1
			, int p2
			, int p3
			, int p4)
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

		static int Calculate(double* samples, int sampleCount, double* points, int pointCount)
		{
			int p1, p2, p3, p4;
			int count = 0;
			double x, y;

			int* p = (int*)malloc(pointCount * sizeof(int));
			assert(p);
			if (!p) return -1;

			for (int i = 0; i < pointCount; i++)
			{
				x = points[2 * i + 0];
				y = points[2 * i + 1];

				if (x >= 0.0)
				{
					p[count++] = i;
				}
			}

			if (count != 0)
			{
				x = points[2 * p[0] + 0];
				y = points[2 * p[0] + 1];
				int boundary = ROUND(x * (double)(sampleCount - 1));

				for (int i = 0; i < boundary; i++)
				{
					samples[i] = y;
				}

				x = points[2 * p[count - 1] + 0];
				y = points[2 * p[count - 1] + 1];

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

					Plot(points, pointCount, samples, sampleCount, p1, p2, p3, p4);
				}

				for (int i = 0; i < count; i++)
				{
					x = points[2 * p[i] + 0];
					y = points[2 * p[i] + 1];
					samples[ROUND(x * (double)(sampleCount - 1))] = y;
				}
			}

			if (p) free(p);
			return sampleCount;
		}
	};

	class XCurve
	{
	public:
		XCurve(void)
		{
			points = 0;
			samples = 0;
			pointCount = 0;
			sampleCount = 0;
			identity = false;
			SetConfig(17, 256);
			Calculate();
		}

		virtual ~XCurve(void)
		{
			if (points) free(points);
			if (samples) free(samples);
		}

		bool SetConfig(int _pointCount, int _sampleCount)
		{
			if (pointCount != _pointCount)
			{
				pointCount = _pointCount;
				points = (double*)realloc(points, pointCount * 2 * sizeof(double));
				assert(points);
				if (points == 0) return false;
			}

			if (sampleCount != _sampleCount)
			{
				sampleCount = _sampleCount;
				samples = (double*)realloc(samples, sampleCount * sizeof(double));
				assert(samples);
				if (samples == 0) return false;
			}

			Reset();
			return true;
		}

		void Calculate(void)
		{
			assert(CheckPoints());
			XCurveMaker::Calculate(samples, sampleCount, points, pointCount);
			identity = true;
		}

		int GetSamples(double* results, int count=-1)
		{
			if (count == -1)
				count = sampleCount;
			else
				limit(count, 0, sampleCount);
			memcpy(results, samples, count * sizeof(double));
			return count;
		}

		int GetSamples(uint8* results, int scale=255, int count = -1)
		{
			if (count == -1)
				count = sampleCount;
			else
				limit(count, 0, sampleCount);
			for (int i = 0; i < count; i++)
			{
				results[i] = (uint8)ROUND(samples[i]* scale);
			}
		}

		void Reset(void)
		{
			for (int i = 0; i < sampleCount; i++)
			{
				samples[i] = (double)i / (double)(sampleCount - 1);
			}

			points[0] = points[1] = 0.0;
			for (int i = 1; i < pointCount - 1; i++)
			{
				points[2 * i + 0] = -1.0;
				points[2 * i + 1] = -1.0;
			}

			points[2 * (pointCount - 1) + 0] = -1.0;
			points[2 * (pointCount - 1) + 1] = -1.0;

			identity = false;
		}
	protected:
		bool CheckPoints(void) const
		{
			for (int i = 0; i < pointCount; i++)
			{
				double x = points[2 * i + 0];
				double y = points[2 * i + 1];

				if (!(x == -1.0 || x >= 0.0 && x <= 1.0))
					return false;
				if (!(y == -1.0 || y >= 0.0 && y <= 1.0))
					return false;
			}

			return true;
		}

	protected:
		double* points;
		double* samples;
		int pointCount;
		int sampleCount;
		bool identity;
	};
}

#endif