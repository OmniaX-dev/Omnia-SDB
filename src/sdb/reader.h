#ifndef __READER__H__
#define __READER__H__

#include "types.h"

namespace sdb
{
	class Reader
	{
		public:
			static int readHeader(byte* data, String& header, int& major_version, int& minor_version, int pointer = 4);
			
			static byte readByte(byte* data, int& pointer);
			static short readShort(byte* data, int& pointer);
			static int readInt(byte* data, int& pointer);
			static long long readLong(byte* data, int& pointer);
			static float readFloat(byte* data, int& pointer);
			static double readDouble(byte* data, int& pointer); //TODO: Fix this
			static bool readBool(byte* data, int& pointer);
			static String readString(byte* data, int& pointer);
			static String readString(byte* data, int& pointer, short size);
	};
}

#endif
