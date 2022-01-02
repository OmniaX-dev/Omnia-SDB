#include "writer.h"

const String sdb::Writer::HEADER = "TBSL";
const short sdb::Writer::VERSION = 0x0100;

void sdb::Writer::writeHeader(byte* data, int& pointer)
{
	write(data, pointer, HEADER);
	write(data, pointer, VERSION);
}

int sdb::Writer::getHeaderSize(void)
{
	return Utils::_get_data_type_size(SHORT) + HEADER.length() + Utils::_get_data_type_size(SHORT);
}

void sdb::Writer::write(byte* data, int& pointer, byte value)
{
	data[pointer++] = value;
}

void sdb::Writer::write(byte* data, int& pointer, short value)
{
	data[pointer++] = (value & 0xff00) >> 8;
	data[pointer++] = (value & 0x00ff);
}

void sdb::Writer::write(byte* data, int& pointer, int value)
{
	data[pointer++] = (value & 0xff000000) >> 24;
	data[pointer++] = (value & 0x00ff0000) >> 16;
	data[pointer++] = (value & 0x0000ff00) >> 8;
	data[pointer++] = (value & 0x000000ff);
}

void sdb::Writer::write(byte* data, int& pointer, long long value)
{
	data[pointer++] = (value & 0xff00000000000000) >> 56;
	data[pointer++] = (value & 0x00ff000000000000) >> 48;
	data[pointer++] = (value & 0x0000ff0000000000) >> 40;
	data[pointer++] = (value & 0x000000ff00000000) >> 32;
	data[pointer++] = (value & 0x00000000ff000000) >> 24;
	data[pointer++] = (value & 0x0000000000ff0000) >> 16;
	data[pointer++] = (value & 0x000000000000ff00) >> 8;
	data[pointer++] = (value & 0x00000000000000ff);
}

void sdb::Writer::write(byte* data, int& pointer, float value)
{
	_int_float _if;
	_if.f = value;
	write(data, pointer, _if.i);
}

void sdb::Writer::write(byte* data, int& pointer, double value)
{
	_long_double _ld;
	_ld.d = value;
	write(data, pointer, _ld.l);
}

void sdb::Writer::write(byte* data, int& pointer, bool value)
{
	data[pointer++] = value ? 1 : 0;
}

void sdb::Writer::write(byte* data, int& pointer, byte* value, int len)
{
	for (int i = 0; i < len; i++)
		data[pointer++] = value[i];
}

void sdb::Writer::write(byte* data, int& pointer, String value)
{
	short len = value.length();
	if (len < 1)
		return;
	write(data, pointer, len);
	byte* b_arr = Utils::_c_str_to_b_arr(value);
	write(data, pointer, b_arr, len);
	delete[] b_arr;
}

void sdb::Writer::write(byte* data, int& pointer, short * value, int len)
{
	for (int i = 0; i < len; i++)
		write(data, pointer, value[i]);
}

void sdb::Writer::write(byte* data, int& pointer, int* value, int len)
{
	for (int i = 0; i < len; i++)
		write(data, pointer, value[i]);
}

void sdb::Writer::write(byte* data, int& pointer, long long* value, int len)
{
	for (int i = 0; i < len; i++)
		write(data, pointer, value[i]);
}

void sdb::Writer::write(byte* data, int& pointer, float* value, int len)
{
	for (int i = 0; i < len; i++)
		write(data, pointer, value[i]);
}

void sdb::Writer::write(byte* data, int& pointer, double* value, int len)
{
	for (int i = 0; i < len; i++)
		write(data, pointer, value[i]);
}

void sdb::Writer::write(byte* data, int& pointer, bool* value, int len)
{
	for (int i = 0; i < len; i++)
		write(data, pointer, value[i]);
}
