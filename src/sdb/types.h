#ifndef __TYPES__H__
#define __TYPES__H__

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

#define _p_hex(val) printf("0x%02x", val)
#define _obj_cast(dd) ((ObjectDescriptor*)dd)
#define _fld_cast(dd) ((FieldDescriptor*)dd)
#define _arr_cast(dd) ((ArrayDescriptor*)dd)
#define _str_cast(dd) ((StringDescriptor*)dd)

typedef unsigned char byte;
typedef std::string String;

namespace sdb
{
	enum errcodes
	: byte
	{	
		WRONG_FILE_TYPE = 0, WRONG_FILE_SIZE, CORRUPTED_TOC_POINTER
	};
	
	enum DataType
		: byte
		{	
			UNKNOWN = 0, BYTE, SHORT, INT, LONG, FLOAT, DOUBLE, BOOLEAN
	};
	
	enum ContainerType
		: byte
		{	
			FIELD = 0, OBJECT, ARRAY, NULL_DESCRIPTOR = 254, RESERVED = 255
	};
	
	union _int_float
	{
			int i;
			float f;
	};
	
	union _long_double
	{
			long long l;
			double d;
	};
	
	class Database;
	class ObjectDescriptor;
	class Utils
	{
		public:
			struct TableOfContents;
		
		public:
			static byte* _c_str_to_b_arr(String str);
			static int _get_data_type_size(byte type);
			static void _hex_dump(byte* data, int length);
			static void _dump_database_tree(Database* db);
			static void _padding(int depth, int ns = 4);
			static void _dump_obj_tree(ObjectDescriptor* obj, int depth = 1);
			static void _dump_toc_data(const TableOfContents* toc);
			static unsigned short _next_id(void);

		public:
			struct TableOfContents
			{
				public:
					std::unordered_map<String, int> table;

				public:
					void add(String name, int id, int addr);
					int getSize(void);
					void writeTo(byte* data, int& pointer);
					void fromDataArray(byte* data, short toc_obj_count, int& pointer);
			};

			struct Timer
			{
				public:
					inline Timer(void) :
							beg_(clock_::now())
					{
					}
					inline void reset(void)
					{
						beg_ = clock_::now();
					}
					inline double elapsed(void) const
					{
						return std::chrono::duration_cast<second_>(clock_::now() - beg_).count();
					}
					
				private:
					typedef std::chrono::high_resolution_clock clock_;
					typedef std::chrono::duration<double, std::ratio<1>> second_;
					std::chrono::time_point<clock_> beg_;
			};

		private:
			static unsigned short next_id;
			
	};
	
	struct DataDescriptor
	{
		protected:
			byte containerType;
			String name;
			byte dataType;
			byte* data;
			
		private:
			static const DataDescriptor* _null_dsc;

		public:
			inline bool is(byte ct) const { return containerType == ct; }
			inline bool isObject(void) const { return is(OBJECT); }
			inline bool isArray(void) const { return is(ARRAY); }
			inline bool isField(void) const { return is(FIELD); }
			inline bool isNUll(void) const { return is(NULL_DESCRIPTOR); }
			
			inline String getName(void) const { return name; }
			inline byte getContainerType(void) const { return containerType; }
			
			inline static const DataDescriptor& null_dsc(void) { return *_null_dsc; }
			
			inline virtual void writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc = NULL) {}
			inline virtual int getTotalSize(void) { return -1; }
			inline virtual ~DataDescriptor(void)
			{
			}
			
		protected:
			inline DataDescriptor(void) : containerType(NULL_DESCRIPTOR), name("NULL_DSC"), dataType(UNKNOWN), data(NULL) {}
			
	};
	
	struct ArrayDescriptor : public DataDescriptor
	{
		protected:
			int elementCount;
			bool is_string;
			short* _shortArray;
			int* _intArray;
			long long* _longArray;
			float* _floatArray;
			double* _doubleArray;
			bool* _boolArray;
			
			static const ArrayDescriptor* _null_arr;

		public:
			void writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc = NULL);
			int getTotalSize(void);
			
			inline int getElementCount(void) const { return elementCount; }

			static ArrayDescriptor* deserialize(byte* data, int& pointer);
			
			static const ArrayDescriptor& null_arr(void) { return *_null_arr; }

			static ArrayDescriptor newArray(String name, byte* data, int size);
			static ArrayDescriptor newArray(String name, short* data, int size);
			static ArrayDescriptor newArray(String name, int* data, int size);
			static ArrayDescriptor newArray(String name, long long* data, int size);
			static ArrayDescriptor newArray(String name, float* data, int size);
			static ArrayDescriptor newArray(String name, double* data, int size);
			static ArrayDescriptor newArray(String name, bool* data, int size);
			
			inline static ArrayDescriptor* newArrayPtr(String name, byte* data, int size) { return new ArrayDescriptor(newArray(name, data, size)); }
			inline static ArrayDescriptor* newArrayPtr(String name, short* data, int size) { return new ArrayDescriptor(newArray(name, data, size)); }
			inline static ArrayDescriptor* newArrayPtr(String name, int* data, int size) { return new ArrayDescriptor(newArray(name, data, size)); }
			inline static ArrayDescriptor* newArrayPtr(String name, long long* data, int size) { return new ArrayDescriptor(newArray(name, data, size)); }
			inline static ArrayDescriptor* newArrayPtr(String name, float* data, int size) { return new ArrayDescriptor(newArray(name, data, size)); }
			inline static ArrayDescriptor* newArrayPtr(String name, double* data, int size) { return new ArrayDescriptor(newArray(name, data, size)); }
			inline static ArrayDescriptor* newArrayPtr(String name, bool* data, int size) { return new ArrayDescriptor(newArray(name, data, size)); }
			
			void editData(String name, byte* data, int size);
			void editData(String name, short* data, int size);
			void editData(String name, int* data, int size);
			void editData(String name, long long* data, int size);
			void editData(String name, float* data, int size);
			void editData(String name, double* data, int size);
			void editData(String name, bool* data, int size);
			
			byte* byteArray(void) const;
			short* shortArray(void) const;
			int* intArray(void) const;
			long long* longArray(void) const;
			float* floatArray(void) const;
			double* doubleArray(void) const;
			bool* boolArray(void) const;

		protected:
			ArrayDescriptor(void);
			inline ArrayDescriptor(byte ct) : ArrayDescriptor() { containerType = ct; }
			static ArrayDescriptor* get(String name, byte dt, int size, int& pointer, byte* data);
	};
	
	struct StringDescriptor : public ArrayDescriptor
	{
		public:
			StringDescriptor(String name, String data);
			void writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc = NULL);
			int getTotalSize(void);
			
			String stringValue(void) const;
			inline void edit(String name, String value) { editData(name, Utils::_c_str_to_b_arr(value), value.length()); }
			
			static const StringDescriptor& null_str(void) { return *(_str_cast(_null_arr)); }
	};
	
	struct FieldDescriptor : public DataDescriptor
	{
		private:
			int dataSize;
			
			static const FieldDescriptor* _null_fld;

		public:
			void writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc = NULL);
			int getTotalSize(void);

			static FieldDescriptor* deserialize(byte* data, int& pointer);
			
			static const FieldDescriptor& null_fld(void) { return *_null_fld; }

			static FieldDescriptor newField(String name, byte value);
			static FieldDescriptor newField(String name, short value);
			static FieldDescriptor newField(String name, int value);
			static FieldDescriptor newField(String name, long long value);
			static FieldDescriptor newField(String name, float value);
			static FieldDescriptor newField(String name, double value);
			static FieldDescriptor newField(String name, bool value);
			
			inline static FieldDescriptor* newFieldPtr(String name, byte value) { return new FieldDescriptor(newField(name, value)); }
			inline static FieldDescriptor* newFieldPtr(String name, short value) { return new FieldDescriptor(newField(name, value)); }
			inline static FieldDescriptor* newFieldPtr(String name, int value) { return new FieldDescriptor(newField(name, value)); }
			inline static FieldDescriptor* newFieldPtr(String name, long long value) { return new FieldDescriptor(newField(name, value)); }
			inline static FieldDescriptor* newFieldPtr(String name, float value) { return new FieldDescriptor(newField(name, value)); }
			inline static FieldDescriptor* newFieldPtr(String name, double value) { return new FieldDescriptor(newField(name, value)); }
			inline static FieldDescriptor* newFieldPtr(String name, bool value) { return new FieldDescriptor(newField(name, value)); }
			
			void editData(String name, byte value);
			void editData(String name, short value);
			void editData(String name, int value);
			void editData(String name, long long value);
			void editData(String name, float value);
			void editData(String name, double value);
			void editData(String name, bool value);
			
			byte byteValue(void) const;
			short shortValue(void) const;
			int intValue(void) const;
			long long longValue(void) const;
			float floatValue(void) const;
			double doubleValue(void) const;
			bool boolValue(void) const;

		private:
			FieldDescriptor(void);
			inline FieldDescriptor(byte ct) : FieldDescriptor() { containerType = ct; }
			static FieldDescriptor* get(String name, byte dt, int& pointer, byte* data);
	};
	
	struct ObjectDescriptor : public DataDescriptor
	{
		private:
			std::vector<DataDescriptor*> descriptors;
			int desc_size;
			int desc_count;

		private:
			short unique_id;
			static const ObjectDescriptor* _null_obj;

		public:
			ObjectDescriptor(String name);
			void writeTo(byte* dest, int& pointer, Utils::TableOfContents* toc = NULL);
			int getTotalSize(void);
			void addDescriptor(DataDescriptor* descriptor);
			static ObjectDescriptor* deserialize(byte* data, int& pointer, byte ct = RESERVED, int deep = 0, Utils::TableOfContents* toc = NULL);
			static inline const ObjectDescriptor& null_obj(void) { return *_null_obj; }
			
			const DataDescriptor& operator[] (String name) const;
			
			ObjectDescriptor& object(String name) const;
			ArrayDescriptor& array(String name) const;
			StringDescriptor& string(String name) const;
			FieldDescriptor& field(String name) const;
			
			inline unsigned short getUniqueID(void) { return unique_id; }
			inline const std::vector<DataDescriptor*>& getDescriptorList(void) const { return descriptors; }
			
		private:
			inline ObjectDescriptor(void) : ObjectDescriptor("NULL_OBJ") { containerType = NULL_DESCRIPTOR; }
	};
	
	class Serializable
	{
		public:
			virtual ObjectDescriptor serialize(String name) = 0;
			virtual void deserialize(const ObjectDescriptor& obj) = 0;
			virtual inline ~Serializable(void) {}
	};
}

#endif
