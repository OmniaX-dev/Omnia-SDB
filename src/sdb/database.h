#ifndef __DATABASE__H__
#define __DATABASE__H__

#include <fstream>
#include "types.h"

namespace sdb
{
	namespace io
	{
		enum io_type
			: byte
			{	
				read_only = 0, read_write
		};
	}
	
	class DatabaseInfo
	{
		public:
			inline String getName(void) const
			{
				return name;
			}
			inline const Utils::TableOfContents& getToc(void) const
			{
				return toc;
			}
			
		protected:
			String name;
			int size;
			byte major_version;
			byte minor_version;
			int obj_count;
			Utils::TableOfContents toc;

		protected:
			inline DatabaseInfo(void) :
					name(""), size(0), major_version(-1), minor_version(-1), obj_count(0)
			{
			}
	};
	
	class Database : public DatabaseInfo
	{
		public:
			Database(String name);
			void writeToFile(String path);
			int getSize(void);
			byte* toByteArray(bool has_toc = true);
			void addObject(ObjectDescriptor* obj);
			void calcSize(void);
			const ObjectDescriptor& operator[](String name);
			ObjectDescriptor& getEditable(String name);
			
			inline bool isEditable(void)
			{
				return editable;
			}
			
			inline const ObjectDescriptor& getObject(int index)
			{
				return *objects[index];
			}
			inline const std::vector<ObjectDescriptor*>& getAllObjects(void)
			{
				return objects;
			}
			
			static Database deserialize(byte* data, int length, io::io_type _io_type = io::read_only);
			static Database deserialize(String filePath, io::io_type _io_type = io::read_only);

		private:
			std::vector<ObjectDescriptor*> objects;
			bool editable;
			
	};
	
	class DynamicDatabase : public DatabaseInfo
	{
		private:
			std::ifstream file;

		public:
			inline DynamicDatabase(void)
			{
				create("");
			}
			inline DynamicDatabase(String filePath)
			{
				create(filePath);
			}
			void create(String filePath);
			std::vector<ObjectDescriptor> getObjects(String name, int count = -1);
			ObjectDescriptor operator[](String name);

			inline size_t fileSize(void)
			{
				return size;
			}
			inline void close(void)
			{
				file.close();
			}
	};
}

#endif
