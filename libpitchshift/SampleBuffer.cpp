#include "SampleBuffer.h"
#include <assert.h>

namespace e
{
	SampleBuffer::SampleBuffer(int channels /* = 2 */)
	{
		assert(channels > 0);
		this->buffer[0] = 0;
		this->buffer[1] = 0;
		this->sizeInBytes = 0;
		this->sampleInBuffer = 0;
		this->channels = channels;
		Capacity(32);
	}

	SampleBuffer::~SampleBuffer()
	{
		if (buffer[1])
		{
			free(buffer[1]);
		}

		buffer[0] = buffer[1] = 0;
	}

	void SampleBuffer::SetChannels(int channels)
	{
		assert(channels > 0);
		this->channels = channels;
		this->sampleInBuffer = (channels * sampleInBuffer) / channels;
	}

	void SampleBuffer::PutSamples(uint samples)
	{
		Capacity(sampleInBuffer + samples);
		sampleInBuffer += samples;
	}

	void SampleBuffer::PutSamples(sample_t* samples, int count)
	{
		memcpy(End(count), samples, sizeof(sample_t) * channels * count);
		sampleInBuffer += count;
	}

	void SampleBuffer::Capacity(int capacity)
	{
		assert(capacity > 0);

		if (capacity > GetCapacity())
		{
			sizeInBytes = (capacity * channels * sizeof(sample_t) + 4095) & ((uint)-4096);
			assert(sizeInBytes % 2 == 0);
			buffer[1] = (sample_t*)realloc(buffer[1], (sizeInBytes / sizeof(sample_t) + 16 / sizeof(sample_t)));
			assert(buffer[1] != 0);
			if (buffer[1] == 0) E_THROW("SampleBuffer realloc failed!!!");

			buffer[0] = (sample_t*)ALIGN_POINTER_16(buffer[1]);
			offset = 0;
		}
		else
		{
			Rewind();
		}
	}

	uint SampleBuffer::GetCapacity(void) const
	{
		return sizeInBytes / (channels * sizeof(sample_t));
	}

	uint SampleBuffer::GetChannels(void) const
	{
		return channels;
	}

	uint SampleBuffer::GetSampleCount(void) const
	{
		return sampleInBuffer;
	}

	uint SampleBuffer::GetSamples(uint n)
	{
		if (n >= sampleInBuffer)
		{
			uint temp = sampleInBuffer;
			sampleInBuffer = 0;
			return temp;
		}

		sampleInBuffer -= n;
		offset += n;
		return n;
	}

	uint SampleBuffer::GetSamples(sample_t* samples, uint count)
	{
		uint n = min(count, sampleInBuffer);
		memcpy(samples, Begin(), sizeof(sample_t)*channels*n);
		GetSamples(n);
	}

	uint SampleBuffer::AdjustSampleCount(uint count)
	{
		if (count < sampleInBuffer)
		{
			sampleInBuffer = count;
		}
		return sampleInBuffer;
	}

	void SampleBuffer::Rewind(void)
	{
		if (buffer[0] && offset)
		{
			memmove(buffer[0], Begin(), sizeof(sample_t) * channels * sampleInBuffer);
			offset = 0;
		}
	}

	sample_t* SampleBuffer::Begin(void)
	{
		assert(buffer);
		return buffer[0] + offset * channels;
	}

	sample_t* SampleBuffer::End(uint slackCapacity)
	{
		Capacity(sampleInBuffer + slackCapacity);
		return buffer[0] + sampleInBuffer * channels;
	}

	bool SampleBuffer::IsEmpty(void) const
	{
		return sampleInBuffer == 0;
	}

	void SampleBuffer::Clear(void)
	{
		sampleInBuffer = 0;
		offset = 0;
	}
}

