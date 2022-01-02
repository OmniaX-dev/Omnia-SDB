#include "reader.h"

#include <iostream>

int sdb::Reader::readHeader(byte* data, String& header, int& major_version, int& minor_version, int pointer)
{
	header = readString(data, pointer);
	major_version = readByte(data, pointer);
	minor_version = readByte(data, pointer);
	return pointer;
}

byte sdb::Reader::readByte(byte* data, int& pointer)
{
	return data[pointer++];
}

short sdb::Reader::readShort(byte* data, int& pointer)
{
	short s = (short) ((data[pointer] << 8) | (data[pointer + 1]));
	pointer += Utils::_get_data_type_size(SHORT);
	return s;
}

int sdb::Reader::readInt(byte* data, int& pointer)
{
	int i = (int) ((data[pointer] << 24) | (data[pointer + 1] << 16) | (data[pointer + 2] << 8) | (data[pointer + 3]));
	pointer += Utils::_get_data_type_size(INT);
	return i;
}

long long sdb::Reader::readLong(byte* data, int& pointer)
{
	long long l = (long long) ((data[pointer] << 56) | (data[pointer + 1] << 48) | (data[pointer + 2] << 40) | (data[pointer + 3] << 32) | (data[pointer + 4] << 24) | (data[pointer + 5] << 16) | (data[pointer + 6] << 8) | (data[pointer + 7]));
	pointer += Utils::_get_data_type_size(LONG);
	return l;
}

float sdb::Reader::readFloat(byte* data, int& pointer)
{
	_int_float _if;
	_if.i = readInt(data, pointer);
	return _if.f;
}

double sdb::Reader::readDouble(byte* data, int& pointer)
{
	_long_double _ld;
	_ld.l = readLong(data, pointer);
	return _ld.d;
}

bool sdb::Reader::readBool(byte* data, int& pointer)
{
	return readByte(data, pointer) != 0;
}

String sdb::Reader::readString(byte* data, int& pointer)
{
	short len = readShort(data, pointer);
	String str = "";
	for (short i = 0; i < len; i++)
		str += readByte(data, pointer);
	return str;
}

String sdb::Reader::readString(byte* data, int& pointer, short size)
{
	String str = "";
	for (short i = 0; i < size; i++)
		str += readByte(data, pointer);
	return str;
}
