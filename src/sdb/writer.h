#ifndef __SERIALIZER__H__
#define __SERIALIZER__H__

#include "types.h"

namespace sdb
{	
	class Writer
	{
		public:
			static const String HEADER;
			static const short VERSION;
			
		public:
			static void writeHeader(byte* data, int& pointer);
			static int getHeaderSize(void);
			
			static void write(byte* data, int& pointer, byte value);
			static void write(byte* data, int& pointer, short value);
			static void write(byte* data, int& pointer, int value);
			static void write(byte* data, int& pointer, long long value);
			static void write(byte* data, int& pointer, float value);
			static void write(byte* data, int& pointer, double value);
			static void write(byte* data, int& pointer, bool value);
			static void write(byte* data, int& pointer, String value);

			static void write(byte* data, int& pointer, byte* value, int len = 0);
			static void write(byte* data, int& pointer, short* value, int len = 0);
			static void write(byte* data, int& pointer, int* value, int len = 0);
			static void write(byte* data, int& pointer, long long* value, int len = 0);
			static void write(byte* data, int& pointer, float* value, int len = 0);
			static void write(byte* data, int& pointer, double* value, int len = 0);
			static void write(byte* data, int& pointer, bool* value, int len = 0);
			
	};

}

#endif
