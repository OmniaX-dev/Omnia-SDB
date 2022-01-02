#include "database.h"

#include <fstream>
#include <iostream>
#include "reader.h"
#include "writer.h"

sdb::Database::Database(String name)
{
	this->name = name;
	size = Utils::_get_data_type_size(SHORT) + name.length();
	size += Utils::_get_data_type_size(INT);
	size += Utils::_get_data_type_size(INT);
	size += Writer::getHeaderSize();
	size += Utils::_get_data_type_size(INT);
	size += Utils::_get_data_type_size(INT);
	major_version = minor_version = -1;
	editable = true;
	obj_count = -1;
}

void sdb::Database::writeToFile(String path)
{
	byte* data = toByteArray();
	std::ofstream(path, std::ios::binary).write((char*) data, size);
	free(data);
}

int sdb::Database::getSize(void)
{
	return size;
}

void sdb::Database::calcSize(void)
{
	if (!editable)
		return;
	size = Utils::_get_data_type_size(SHORT) + name.length();
	size += Utils::_get_data_type_size(INT);
	size += Utils::_get_data_type_size(INT);
	size += Writer::getHeaderSize();
	size += Utils::_get_data_type_size(INT);
	size += Utils::_get_data_type_size(INT);
	for (ObjectDescriptor* o : objects)
		size += o->getTotalSize();
}

byte* sdb::Database::toByteArray(bool has_toc)
{
	byte* data = (byte*) malloc(sizeof(byte) * this->size);
	int pointer = 0;
	Writer::write(data, pointer, (int) 0x0); //Header size, set at 0 for now
	Writer::writeHeader(data, pointer);
	int toc_pointer = pointer;
	Writer::write(data, pointer, (int) 0x0); //Table of contents' pointer, set at 0 for now
	Writer::write(data, pointer, name);
	int size_pointer = pointer;
	Writer::write(data, pointer, size);
	Writer::write(data, pointer, (signed) objects.size());
	int p = 0;
	Writer::write(data, p, pointer);
	for (ObjectDescriptor* o : objects)
		o->writeTo(data, pointer, &toc);
	Writer::write(data, toc_pointer, (has_toc ? pointer : (int) 0));
	if (has_toc)
	{
		size += toc.getSize();
		Writer::write(data, size_pointer, size);
		data = (byte*) realloc(data, sizeof(byte) * size);
		toc.writeTo(data, pointer);
	}
	return data;
}

void sdb::Database::addObject(ObjectDescriptor* obj)
{
	if (!editable)
		return;
	objects.push_back(obj);
	size += obj->getTotalSize();
}

sdb::Database sdb::Database::deserialize(byte* data, int length, io::io_type _io_type)
{
	int min_v = 0, maj_v = 0;
	String header = "", name = "";
	int pointer = 0;
	Reader::readInt(data, pointer); //This is the complete header size
	pointer = Reader::readHeader(data, header, maj_v, min_v);
	if (header != Writer::HEADER)
	{
		std::cerr << "Wrong file type: File type does not match description.\n";
		exit(WRONG_FILE_TYPE);
	}
	int toc_addr = Reader::readInt(data, pointer);
	name = Reader::readString(data, pointer);
	int size = Reader::readInt(data, pointer);
	if (size != length)
	{
		std::cerr << "Corrupted file: Wrong file size.\n";
		exit(WRONG_FILE_SIZE);
	}
	int obj_count = Reader::readInt(data, pointer);
	Database db(name);
	for (int i = 0; i < obj_count; i++)
		db.addObject(ObjectDescriptor::deserialize(data, pointer));
	db.obj_count = obj_count;
	db.size = size;
	if (_io_type == io::read_only)
		db.editable = false;
	else if (_io_type == io::read_write)
		db.editable = true;
	if (toc_addr == 0)
		return db;
	else if (toc_addr != pointer)
	{
		std::cerr << "Corrupted file: TOC pointer does not match EOD: " << toc_addr << " (" << pointer << ")\n";
		exit(CORRUPTED_TOC_POINTER);
	}
	Reader::readInt(data, pointer); //This is the size of the table of contents
	short toc_obj_count = Reader::readShort(data, pointer);
	db.toc.fromDataArray(data, toc_obj_count, pointer);
	return db;
}

const sdb::ObjectDescriptor& sdb::Database::operator [](String name)
{
	for (ObjectDescriptor* o : objects)
	{
		if (o->getName() == name)
			return *o;
	}
	return ObjectDescriptor::null_obj();
}

sdb::ObjectDescriptor& sdb::Database::getEditable(String name)
{
	if (!editable)
		return (ObjectDescriptor&)ObjectDescriptor::null_obj();
	for (ObjectDescriptor* o : objects)
	{
		if (o->getName() == name)
			return *o;
	}
	return (ObjectDescriptor&)ObjectDescriptor::null_obj();
}

sdb::Database sdb::Database::deserialize(String filePath, io::io_type _io_type)
{
	std::ifstream file(filePath);
	file.seekg(0, std::ios::end);
	size_t len = file.tellg();
	char* content = new char[len];
	file.seekg(0, std::ios::beg);
	file.read(content, len);
	file.close();
	return deserialize((byte*) content, len, _io_type);
}

void sdb::DynamicDatabase::create(String filePath)
{
	if (filePath == "")
		return;
	
	file.open(filePath.c_str());
	file.seekg(0, std::ios::end);
	size = file.tellg();
	file.seekg(0, std::ios::beg);
	char* header_size = new char[Utils::_get_data_type_size(INT)];
	file.read(header_size, Utils::_get_data_type_size(INT));
	int p = 0;
	int hs = Reader::readInt((byte*) header_size, p);
	delete[] header_size;
	hs -= Utils::_get_data_type_size(INT);
	file.seekg(Utils::_get_data_type_size(INT), std::ios::beg);
	char* header = new char[hs];
	file.read(header, hs);
	
	int min_v = 0, maj_v = 0;
	String header_str = "";
	name = "";
	p = Reader::readHeader((byte*) header, header_str, maj_v, min_v, 0);
	if (header_str != Writer::HEADER)
	{
		std::cerr << "Wrong file type: File type does not match description.\n";
		exit(WRONG_FILE_TYPE);
	}
	int toc_addr = Reader::readInt((byte*) header, p);
	name = Reader::readString((byte*) header, p);
	int size = Reader::readInt((byte*) header, p);
	if (size != this->size)
	{
		std::cerr << "Corrupted file: Wrong file size.\n";
		exit(WRONG_FILE_SIZE);
	}
	obj_count = Reader::readInt((byte*) header, p);
	
	file.seekg(toc_addr, std::ios::beg);
	char* tmp = new char[Utils::_get_data_type_size(INT)];
	file.read(tmp, 4);
	int p2 = 0;
	int toc_size = Reader::readInt((byte*) tmp, p2);
	delete[] tmp;
	
	toc_addr += Utils::_get_data_type_size(INT);
	file.seekg(toc_addr, std::ios::beg);
	tmp = new char[Utils::_get_data_type_size(SHORT)];
	file.read(tmp, Utils::_get_data_type_size(SHORT));
	p2 = 0;
	short toc_obj_count = Reader::readShort((byte*) tmp, p2);
	delete[] tmp;
	
	toc_addr += Utils::_get_data_type_size(SHORT);
	file.seekg(toc_addr, std::ios::beg);
	
	toc_size -= Utils::_get_data_type_size(INT);
	toc_size -= Utils::_get_data_type_size(SHORT);
	
	tmp = new char[toc_size];
	file.read(tmp, toc_size);
	
	p = 0;
	toc.fromDataArray((byte*) tmp, toc_obj_count, p);
}

std::vector<sdb::ObjectDescriptor> sdb::DynamicDatabase::getObjects(String name, int count)
{
	name += ":";
	std::vector<sdb::ObjectDescriptor> obj_list;
	if (!file.is_open())
		return obj_list;
	file.clear();
	int ptr = 0;
	char* tmp = NULL;
	int obj_size = 0;
	int p = 0;
	for (auto iterator = toc.table.begin(); iterator != toc.table.end(); iterator++)
	{
		if (name.length() > iterator->first.length())
			continue;
		if (iterator->first.substr(0, name.length()) == name)
		{
			ptr = iterator->second;
			ptr++;
			file.seekg(ptr, std::ios::beg);
			tmp = new char[4];
			file.read(tmp, 4);
			obj_size = Reader::readInt((byte*) tmp, p);
			p = 0;
			delete[] tmp;
			ptr--;
			file.seekg(ptr, std::ios::beg);
			tmp = new char[obj_size];
			file.read(tmp, obj_size);
			ObjectDescriptor* obj = ObjectDescriptor::deserialize((byte*) tmp, p);
			obj_list.push_back(*obj);
			if ((signed) obj_list.size() == count)
				return obj_list;
			p = 0;
		}
	}
	return obj_list;
}

sdb::ObjectDescriptor sdb::DynamicDatabase::operator [](String name)
{
	std::vector<ObjectDescriptor> obj = getObjects(name, 1);
	if (obj.size() < 1)
		return ObjectDescriptor::null_obj();
	return obj[0];
}
