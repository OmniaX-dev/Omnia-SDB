#include "types.h"

#include <iostream>
#include <sstream>
#include "database.h"
#include "reader.h"
#include "writer.h"

byte* sdb::Utils::_c_str_to_b_arr(String str)
{
	const char* c_arr = str.c_str();
	byte* b_arr = new byte[str.length()];
	for (unsigned int i = 0; i < str.length(); i++)
		b_arr[i] = c_arr[i];
	return b_arr;
}

int sdb::Utils::_get_data_type_size(byte type)
{
	switch (type)
	{
		case UNKNOWN:
			return 0;
		case BYTE:
			return 1;
		case SHORT:
			return 2;
		case INT:
			return 4;
		case LONG:
			return 8;
		case FLOAT:
			return 4;
		case DOUBLE:
			return 8;
		case BOOLEAN:
			return 1;
		default:
			return 0;
	}
}

void sdb::Utils::_dump_obj_tree(ObjectDescriptor* obj, int depth)
{
	for (DataDescriptor* dd : obj->getDescriptorList())
	{
		_padding(depth);
		std::cout << dd->getName();
		if (dd->getContainerType() == OBJECT)
			std::cout << ":" << ((ObjectDescriptor*) dd)->getUniqueID();
		std::cout << "\n";
		if (dd->getContainerType() == OBJECT)
			_dump_obj_tree((ObjectDescriptor*) dd, depth + 1);
	}
}

void sdb::Utils::_dump_database_tree(Database* db)
{
	for (ObjectDescriptor* o : db->getAllObjects())
	{
		std::cout << o->getName() << ":" << o->getUniqueID() << "\n";
		_dump_obj_tree(o, 1);
	}
}

void sdb::Utils::_padding(int depth, int ns)
{
	for (int i = 0; i < depth * ns; i++)
		std::cout << " ";
}

void sdb::Utils::_hex_dump(byte* data, int length)
{
	
	for (int i = 0; i < length; i++)
	{
		_p_hex(data[i]);
		std::cout << "  ";
		if ((i + 1) % 8 == 0)
		{
			std::cout << "   | ";
			for (int j = i - 7; j <= i; j++)
				std::cout << (data[j] != 10 && data[j] != 13 ? (char) data[j] : ' ') << " | ";
			std::cout << "\n";
		}
	}
}

void sdb::Utils::_dump_toc_data(const sdb::Utils::TableOfContents* toc)
{
	for (auto it = toc->table.begin(); it != toc->table.end(); it++)
		std::cout << it->first << "  -->  " << it->second << "\n";
}

unsigned short sdb::Utils::next_id = 100;

unsigned short sdb::Utils::_next_id(void)
{
	return next_id++;
}

void sdb::Utils::TableOfContents::add(String name, int id, int addr)
{
	std::stringstream sstr;
	sstr << name << ":" << id;
	table[sstr.str()] = addr;
}

int sdb::Utils::TableOfContents::getSize(void)
{
	int size = 0;
	for (auto iterator = table.begin(); iterator != table.end(); iterator++)
		size += _get_data_type_size(SHORT) + iterator->first.length() + _get_data_type_size(INT);
	size += _get_data_type_size(INT);
	size += _get_data_type_size(SHORT);
	return size;
}

void sdb::Utils::TableOfContents::writeTo(byte* data, int& pointer)
{
	Writer::write(data, pointer, getSize());
	Writer::write(data, pointer, (short) (table.size()));
	for (auto iterator = table.begin(); iterator != table.end(); iterator++)
	{
		Writer::write(data, pointer, iterator->first);
		Writer::write(data, pointer, iterator->second);
	}
}

int sdb::FieldDescriptor::getTotalSize(void)
{
	int size = 1;
	size += Utils::_get_data_type_size(SHORT) + name.length();
	size += 1;
	size += dataSize;
	return size;
}

sdb::FieldDescriptor::FieldDescriptor(void)
{
	name = "";
	data = NULL;
	dataType = 0;
	dataSize = 0;
	containerType = FIELD;
}

void sdb::FieldDescriptor::writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc)
{
	Writer::write(dest, pointer, containerType);
	Writer::write(dest, pointer, name);
	Writer::write(dest, pointer, dataType);
	Writer::write(dest, pointer, data, dataSize);
}

byte sdb::FieldDescriptor::byteValue(void) const
{
	if (dataType != BYTE)
		return -1;
	int ptr = 0;
	return Reader::readByte(data, ptr);
}

short sdb::FieldDescriptor::shortValue(void) const
{
	if (dataType != SHORT)
		return -1;
	int ptr = 0;
	return Reader::readShort(data, ptr);
}

int sdb::FieldDescriptor::intValue(void) const
{
	if (dataType != INT)
		return -1;
	int ptr = 0;
	return Reader::readInt(data, ptr);
}

long long sdb::FieldDescriptor::longValue(void) const
{
	if (dataType != LONG)
		return -1;
	int ptr = 0;
	return Reader::readLong(data, ptr);
}

float sdb::FieldDescriptor::floatValue(void) const
{
	if (dataType != FLOAT)
		return -1;
	int ptr = 0;
	return Reader::readFloat(data, ptr);
}

double sdb::FieldDescriptor::doubleValue(void) const
{
	if (dataType != DOUBLE)
		return -1;
	int ptr = 0;
	return Reader::readDouble(data, ptr);
}

bool sdb::FieldDescriptor::boolValue(void) const
{
	if (dataType != BOOLEAN)
		return false;
	int ptr = 0;
	return Reader::readBool(data, ptr);
}

sdb::FieldDescriptor* sdb::FieldDescriptor::get(String name, byte dt, int& pointer, byte* data)
{
	FieldDescriptor fd;
	switch (dt)
	{
		case BYTE:
			fd = newField(name, Reader::readByte(data, pointer));
		break;
		case SHORT:
			fd = newField(name, Reader::readShort(data, pointer));
		break;
		case INT:
			fd = newField(name, Reader::readInt(data, pointer));
		break;
		case LONG:
			fd = newField(name, Reader::readLong(data, pointer));
		break;
		case FLOAT:
			fd = newField(name, Reader::readFloat(data, pointer));
		break;
		case DOUBLE:
			fd = newField(name, Reader::readDouble(data, pointer));
		break;
		case BOOLEAN:
			fd = newField(name, Reader::readBool(data, pointer));
		break;
		case UNKNOWN:
		default:
			std::cerr << "Unknown data type: addr " << pointer << "\n";
		break;
	}
	return new FieldDescriptor(fd);
}

sdb::FieldDescriptor* sdb::FieldDescriptor::deserialize(byte* data, int& pointer)
{
	String name = Reader::readString(data, pointer);
	byte dt = Reader::readByte(data, pointer);
	FieldDescriptor* fd = get(name, dt, pointer, data);
	return fd;
}

sdb::FieldDescriptor sdb::FieldDescriptor::newField(String name, byte value)
{
	FieldDescriptor f;
	f.name = name;
	f.dataType = BYTE;
	f.dataSize = Utils::_get_data_type_size(BYTE);
	f.data = new byte[f.dataSize];
	int counter = 0;
	Writer::write(f.data, counter, value);
	return f;
}

sdb::FieldDescriptor sdb::FieldDescriptor::newField(String name, short value)
{
	FieldDescriptor f;
	f.name = name;
	f.dataType = SHORT;
	f.dataSize = Utils::_get_data_type_size(SHORT);
	f.data = new byte[f.dataSize];
	int counter = 0;
	Writer::write(f.data, counter, value);
	return f;
}

sdb::FieldDescriptor sdb::FieldDescriptor::newField(String name, int value)
{
	FieldDescriptor f;
	f.name = name;
	f.dataType = INT;
	f.dataSize = Utils::_get_data_type_size(INT);
	f.data = new byte[f.dataSize];
	int counter = 0;
	Writer::write(f.data, counter, value);
	return f;
}

sdb::FieldDescriptor sdb::FieldDescriptor::newField(String name, long long value)
{
	FieldDescriptor f;
	f.name = name;
	f.dataType = LONG;
	f.dataSize = Utils::_get_data_type_size(LONG);
	f.data = new byte[f.dataSize];
	int counter = 0;
	Writer::write(f.data, counter, value);
	return f;
}

sdb::FieldDescriptor sdb::FieldDescriptor::newField(String name, float value)
{
	FieldDescriptor f;
	f.name = name;
	f.dataType = FLOAT;
	f.dataSize = Utils::_get_data_type_size(FLOAT);
	f.data = new byte[f.dataSize];
	int counter = 0;
	Writer::write(f.data, counter, value);
	return f;
}

sdb::FieldDescriptor sdb::FieldDescriptor::newField(String name, double value)
{
	FieldDescriptor f;
	f.name = name;
	f.dataType = DOUBLE;
	f.dataSize = Utils::_get_data_type_size(DOUBLE);
	f.data = new byte[f.dataSize];
	int counter = 0;
	Writer::write(f.data, counter, value);
	return f;
}

sdb::FieldDescriptor sdb::FieldDescriptor::newField(String name, bool value)
{
	FieldDescriptor f;
	f.name = name;
	f.dataType = BOOLEAN;
	f.dataSize = Utils::_get_data_type_size(BOOLEAN);
	f.data = new byte[f.dataSize];
	int counter = 0;
	Writer::write(f.data, counter, value);
	return f;
}

void sdb::FieldDescriptor::editData(String name, byte value)
{
	if (isNUll())
		return;
	if (data != NULL)
		delete[] data;
	this->name = name;
	this->dataType = BYTE;
	this->dataSize = Utils::_get_data_type_size(BYTE);
	this->data = new byte[this->dataSize];
	int counter = 0;
	Writer::write(this->data, counter, value);
}

void sdb::FieldDescriptor::editData(String name, short value)
{
	if (isNUll())
		return;
	if (data != NULL)
		delete[] data;
	this->name = name;
	this->dataType = SHORT;
	this->dataSize = Utils::_get_data_type_size(SHORT);
	this->data = new byte[this->dataSize];
	int counter = 0;
	Writer::write(this->data, counter, value);
}

void sdb::FieldDescriptor::editData(String name, int value)
{
	if (isNUll())
		return;
	if (data != NULL)
		delete[] data;
	this->name = name;
	this->dataType = INT;
	this->dataSize = Utils::_get_data_type_size(INT);
	this->data = new byte[this->dataSize];
	int counter = 0;
	Writer::write(this->data, counter, value);
}

void sdb::FieldDescriptor::editData(String name, long long value)
{
	if (isNUll())
		return;
	if (data != NULL)
		delete[] data;
	this->name = name;
	this->dataType = LONG;
	this->dataSize = Utils::_get_data_type_size(LONG);
	this->data = new byte[this->dataSize];
	int counter = 0;
	Writer::write(this->data, counter, value);
}

void sdb::FieldDescriptor::editData(String name, float value)
{
	if (isNUll())
		return;
	if (data != NULL)
		delete[] data;
	this->name = name;
	this->dataType = FLOAT;
	this->dataSize = Utils::_get_data_type_size(FLOAT);
	this->data = new byte[this->dataSize];
	int counter = 0;
	Writer::write(this->data, counter, value);
}

void sdb::FieldDescriptor::editData(String name, double value)
{
	if (isNUll())
		return;
	if (data != NULL)
		delete[] data;
	this->name = name;
	this->dataType = DOUBLE;
	this->dataSize = Utils::_get_data_type_size(DOUBLE);
	this->data = new byte[this->dataSize];
	int counter = 0;
	Writer::write(this->data, counter, value);
}

void sdb::FieldDescriptor::editData(String name, bool value)
{
	if (isNUll())
		return;
	if (data != NULL)
		delete[] data;
	this->name = name;
	this->dataType = BOOLEAN;
	this->dataSize = Utils::_get_data_type_size(BOOLEAN);
	this->data = new byte[this->dataSize];
	int counter = 0;
	Writer::write(this->data, counter, value);
}

void sdb::ArrayDescriptor::writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc)
{
	Writer::write(dest, pointer, containerType);
	Writer::write(dest, pointer, name);
	Writer::write(dest, pointer, dataType);
	Writer::write(dest, pointer, elementCount);
	Writer::write(dest, pointer, (is_string && dataType == BYTE));
	switch (dataType)
	{
		case BYTE:
			Writer::write(dest, pointer, data, elementCount);
		break;
		case SHORT:
			Writer::write(dest, pointer, _shortArray, elementCount);
		break;
		case INT:
			Writer::write(dest, pointer, _intArray, elementCount);
		break;
		case LONG:
			Writer::write(dest, pointer, _longArray, elementCount);
		break;
		case FLOAT:
			Writer::write(dest, pointer, _floatArray, elementCount);
		break;
		case DOUBLE:
			Writer::write(dest, pointer, _doubleArray, elementCount);
		break;
		case BOOLEAN:
			Writer::write(dest, pointer, _boolArray, elementCount);
		break;
		case UNKNOWN:
		default:
		break;
	}
}

int sdb::ArrayDescriptor::getTotalSize(void)
{
	int size = Utils::_get_data_type_size(BYTE);
	size += Utils::_get_data_type_size(SHORT) + name.length();
	size += Utils::_get_data_type_size(BYTE);
	size += Utils::_get_data_type_size(INT);
	size += elementCount * Utils::_get_data_type_size(dataType);
	size += Utils::_get_data_type_size(BOOLEAN);
	return size;
}

byte* sdb::ArrayDescriptor::byteArray(void) const
{
	if (dataType != BYTE)
		return NULL;
	return data;
}

short *sdb::ArrayDescriptor::shortArray(void) const
{
	if (dataType != SHORT)
		return NULL;
	return _shortArray;
}

int* sdb::ArrayDescriptor::intArray(void) const
{
	if (dataType != INT)
		return NULL;
	return _intArray;
}

long long *sdb::ArrayDescriptor::longArray(void) const
{
	if (dataType != LONG)
		return NULL;
	return _longArray;
}

float* sdb::ArrayDescriptor::floatArray(void) const
{
	if (dataType != FLOAT)
		return NULL;
	return _floatArray;
}

double* sdb::ArrayDescriptor::doubleArray(void) const
{
	if (dataType != DOUBLE)
		return NULL;
	return _doubleArray;
}

bool* sdb::ArrayDescriptor::boolArray(void) const
{
	if (dataType != BOOLEAN)
		return NULL;
	return _boolArray;
}

sdb::ArrayDescriptor* sdb::ArrayDescriptor::get(String name, byte dt, int size, int& pointer, byte* data)
{
	ArrayDescriptor ad;
	if (dt == BYTE)
	{
		byte* arr = new byte[size];
		for (int i = 0; i < size; i++)
			arr[i] = Reader::readByte(data, pointer);
		ad = newArray(name, arr, size);
		return new ArrayDescriptor(ad);
	}
	if (dt == SHORT)
	{
		short* arr = new short[size];
		for (int i = 0; i < size; i++)
			arr[i] = Reader::readShort(data, pointer);
		ad = newArray(name, arr, size);
		return new ArrayDescriptor(ad);
	}
	if (dt == INT)
	{
		int* arr = new int[size];
		for (int i = 0; i < size; i++)
			arr[i] = Reader::readInt(data, pointer);
		ad = newArray(name, arr, size);
		return new ArrayDescriptor(ad);
	}
	if (dt == LONG)
	{
		long long* arr = new long long[size];
		for (int i = 0; i < size; i++)
			arr[i] = Reader::readLong(data, pointer);
		ad = newArray(name, arr, size);
		return new ArrayDescriptor(ad);
	}
	if (dt == FLOAT)
	{
		float* arr = new float[size];
		for (int i = 0; i < size; i++)
			arr[i] = Reader::readFloat(data, pointer);
		ad = newArray(name, arr, size);
		return new ArrayDescriptor(ad);
	}
	if (dt == DOUBLE)
	{
		double* arr = new double[size];
		for (int i = 0; i < size; i++)
			arr[i] = Reader::readDouble(data, pointer);
		ad = newArray(name, arr, size);
		return new ArrayDescriptor(ad);
	}
	if (dt == BOOLEAN)
	{
		bool* arr = new bool[size];
		for (int i = 0; i < size; i++)
			arr[i] = Reader::readBool(data, pointer);
		ad = newArray(name, arr, size);
		return new ArrayDescriptor(ad);
	}
	std::cerr << "Unknown data type: addr " << pointer << "\n";
	return NULL;
}

sdb::ArrayDescriptor* sdb::ArrayDescriptor::deserialize(byte* data, int& pointer)
{
	String name = Reader::readString(data, pointer);
	byte dt = Reader::readByte(data, pointer);
	int ec = Reader::readInt(data, pointer);
	bool str_flag = Reader::readBool(data, pointer);
	ArrayDescriptor* ad = NULL;
	if (str_flag)
	{
		String content = Reader::readString(data, pointer, ec);
		ad = new StringDescriptor(name, content);
		return ad;
	}
	ad = get(name, dt, ec, pointer, data);
	return ad;
}

sdb::ArrayDescriptor::ArrayDescriptor(void)
{
	name = "";
	data = NULL;
	dataType = 0;
	elementCount = 0;
	
	_shortArray = NULL;
	_intArray = NULL;
	_longArray = NULL;
	_floatArray = NULL;
	_doubleArray = NULL;
	_boolArray = NULL;
	
	is_string = false;
	containerType = ARRAY;
}

sdb::ArrayDescriptor sdb::ArrayDescriptor::newArray(String name, byte* data, int size)
{
	ArrayDescriptor array;
	array.name = name;
	array.dataType = BYTE;
	array.elementCount = size;
	array.data = data;
	return array;
}

sdb::ArrayDescriptor sdb::ArrayDescriptor::newArray(String name, short * data, int size)
{
	ArrayDescriptor array;
	array.name = name;
	array.dataType = SHORT;
	array.elementCount = size;
	array._shortArray = data;
	return array;
}

sdb::ArrayDescriptor sdb::ArrayDescriptor::newArray(String name, int* data, int size)
{
	ArrayDescriptor array;
	array.name = name;
	array.dataType = INT;
	array.elementCount = size;
	array._intArray = data;
	return array;
}

sdb::ArrayDescriptor sdb::ArrayDescriptor::newArray(String name, long long * data, int size)
{
	ArrayDescriptor array;
	array.name = name;
	array.dataType = LONG;
	array.elementCount = size;
	array._longArray = data;
	return array;
}

sdb::ArrayDescriptor sdb::ArrayDescriptor::newArray(String name, float* data, int size)
{
	ArrayDescriptor array;
	array.name = name;
	array.dataType = FLOAT;
	array.elementCount = size;
	array._floatArray = data;
	return array;
}

sdb::ArrayDescriptor sdb::ArrayDescriptor::newArray(String name, double* data, int size)
{
	ArrayDescriptor array;
	array.name = name;
	array.dataType = DOUBLE;
	array.elementCount = size;
	array._doubleArray = data;
	return array;
}

sdb::ArrayDescriptor sdb::ArrayDescriptor::newArray(String name, bool* data, int size)
{
	ArrayDescriptor array;
	array.name = name;
	array.dataType = BOOLEAN;
	array.elementCount = size;
	array._boolArray = data;
	return array;
}

void sdb::ArrayDescriptor::editData(String name, byte* data, int size)
{
	if (isNUll())
		return;
	this->name = name;
	this->dataType = BYTE;
	this->elementCount = size;
	this->data = data;
}

void sdb::ArrayDescriptor::editData(String name, short * data, int size)
{
	if (isNUll())
		return;
	this->name = name;
	this->dataType = SHORT;
	this->elementCount = size;
	this->_shortArray = data;
}

void sdb::ArrayDescriptor::editData(String name, int* data, int size)
{
	if (isNUll())
		return;
	this->name = name;
	this->dataType = INT;
	this->elementCount = size;
	this->_intArray = data;
}

void sdb::ArrayDescriptor::editData(String name, long long * data, int size)
{
	if (isNUll())
		return;
	this->name = name;
	this->dataType = LONG;
	this->elementCount = size;
	this->_longArray = data;
}

void sdb::ArrayDescriptor::editData(String name, float* data, int size)
{
	if (isNUll())
		return;
	this->name = name;
	this->dataType = FLOAT;
	this->elementCount = size;
	this->_floatArray = data;
}

void sdb::ArrayDescriptor::editData(String name, double* data, int size)
{
	if (isNUll())
		return;
	this->name = name;
	this->dataType = DOUBLE;
	this->elementCount = size;
	this->_doubleArray = data;
}

void sdb::ArrayDescriptor::editData(String name, bool* data, int size)
{
	if (isNUll())
		return;
	this->name = name;
	this->dataType = BOOLEAN;
	this->elementCount = size;
	this->_boolArray = data;
}


const sdb::ObjectDescriptor* sdb::ObjectDescriptor::_null_obj = new sdb::ObjectDescriptor;
const sdb::FieldDescriptor* sdb::FieldDescriptor::_null_fld = new sdb::FieldDescriptor(NULL_DESCRIPTOR);
const sdb::ArrayDescriptor* sdb::ArrayDescriptor::_null_arr = new sdb::ArrayDescriptor(NULL_DESCRIPTOR);
const sdb::DataDescriptor* sdb::DataDescriptor::_null_dsc = new sdb::DataDescriptor;

sdb::ObjectDescriptor::ObjectDescriptor(String name)
{
	containerType = OBJECT;
	this->name = name;
	desc_size = 0;
	desc_count = 0;
	unique_id = Utils::_next_id();
}

void sdb::ObjectDescriptor::writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc)
{
	if (toc != NULL)
		toc->add(name, unique_id, pointer);
	Writer::write(dest, pointer, containerType);
	Writer::write(dest, pointer, getTotalSize());
	Writer::write(dest, pointer, name);
	Writer::write(dest, pointer, unique_id);
	Writer::write(dest, pointer, desc_count);
	for (DataDescriptor* d : descriptors)
		d->writeTo(dest, pointer, toc);
}

sdb::ObjectDescriptor* sdb::ObjectDescriptor::deserialize(byte* data, int& pointer, byte ct, int deep, Utils::TableOfContents* toc)
{
	if (ct == RESERVED)
		ct = Reader::readByte(data, pointer);
	int size = Reader::readInt(data, pointer);
	String name = Reader::readString(data, pointer);
	short id = Reader::readShort(data, pointer);
	int desc_count = Reader::readInt(data, pointer);
	ObjectDescriptor* obj = new ObjectDescriptor(name);
	obj->containerType = ct;
	obj->unique_id = id;
	DataDescriptor* dd = NULL;
	for (int i = 0; i < desc_count; i++)
	{
		ct = Reader::readByte(data, pointer);
		switch (ct)
		{
			case OBJECT:
				dd = deserialize(data, pointer, ct, deep + 1);
				obj->addDescriptor(dd);
				dd = NULL;
			break;
			case ARRAY:
				dd = ArrayDescriptor::deserialize(data, pointer);
				obj->addDescriptor(dd);
				dd = NULL;
			break;
			case FIELD:
				dd = FieldDescriptor::deserialize(data, pointer);
				obj->addDescriptor(dd);
				dd = NULL;
			break;
			default:
				std::cerr << "Unknown field descriptor: addr = " << (pointer - 1) << "\n";
			break;
		}
	}
	if (obj->getTotalSize() != size)
		std::cerr << "Wrong object size. obj_name: " << obj->name << "; size: " << obj->getTotalSize() << " (" << size << ")\n";
	return obj;
}

int sdb::ObjectDescriptor::getTotalSize(void)
{
	int size = 1 + Utils::_get_data_type_size(INT);
	size += Utils::_get_data_type_size(SHORT);
	size += Utils::_get_data_type_size(SHORT) + name.length();
	size += Utils::_get_data_type_size(INT);
	size += desc_size;
	return size;
}

void sdb::ObjectDescriptor::addDescriptor(DataDescriptor* descriptor)
{
	descriptors.push_back(descriptor);
	desc_size += descriptor->getTotalSize();
	desc_count++;
}

const sdb::DataDescriptor& sdb::ObjectDescriptor::operator [](String name) const
{
	for (DataDescriptor* d : descriptors)
	{
		if (d->getName() == name)
			return *d;
	}
	return DataDescriptor::null_dsc();
}

sdb::ObjectDescriptor& sdb::ObjectDescriptor::object(String name) const
{
	for (DataDescriptor* d : descriptors)
	{
		if (d->getContainerType() == OBJECT && d->getName() == name)
			return *_obj_cast(d);
	}
	return (ObjectDescriptor&)ObjectDescriptor::null_obj();
}

sdb::ArrayDescriptor& sdb::ObjectDescriptor::array(String name) const
{
	for (DataDescriptor* d : descriptors)
	{
		if (d->getContainerType() == ARRAY && d->getName() == name)
			return *_arr_cast(d);
	}
	return (ArrayDescriptor&)ArrayDescriptor::null_arr();
}

sdb::StringDescriptor& sdb::ObjectDescriptor::string(String name) const
{
	for (DataDescriptor* d : descriptors)
	{
		if (d->getContainerType() == ARRAY && d->getName() == name)
			return *_str_cast(d);
	}
	return (StringDescriptor&)StringDescriptor::null_str();
}

sdb::FieldDescriptor& sdb::ObjectDescriptor::field(String name) const
{
	for (DataDescriptor* d : descriptors)
	{
		if (d->getContainerType() == FIELD && d->getName() == name)
			return *_fld_cast(d);
	}
	return (FieldDescriptor&)FieldDescriptor::null_fld();
}

sdb::StringDescriptor::StringDescriptor(String name, String data) :
		ArrayDescriptor()
{
	is_string = true;
	this->name = name;
	dataType = BYTE;
	elementCount = data.length();
	this->data = Utils::_c_str_to_b_arr(data);
}

void sdb::StringDescriptor::writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc)
{
	ArrayDescriptor::writeTo(dest, pointer);
}

int sdb::StringDescriptor::getTotalSize(void)
{
	return ArrayDescriptor::getTotalSize();
}

String sdb::StringDescriptor::stringValue(void) const
{
	if (dataType != BYTE)
		return "NULL_STR";
	int ptr = 0;
	return Reader::readString(data, ptr, elementCount);
}

void sdb::Utils::TableOfContents::fromDataArray(byte* data, short toc_obj_count, int& pointer)
{
	String obj_id = "";
	int obj_addr = 0;
	table.clear();
	for (short i = 0; i < toc_obj_count; i++)
	{
		obj_id = Reader::readString(data, pointer);
		obj_addr = Reader::readInt(data, pointer);
		table[obj_id] = obj_addr;
	}
}
