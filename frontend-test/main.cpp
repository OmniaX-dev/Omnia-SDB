#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <climits>
#include <time.h>
#include <sstream>
#include "sdb/sdb.h"

#include "tile.h"

void createStressDB(int n_objs)
{
	sdb::Database db("TestDB2");
	
	std::stringstream str;
	
	for (int i = 0; i < n_objs; i++)
	{
		str << "OBJECT_" << i;
		sdb::ObjectDescriptor* obj = new sdb::ObjectDescriptor(str.str());
		str.str(std::string());
		str.clear();
		str << "ARRAY_" << i;
		sdb::ArrayDescriptor* arr = new sdb::ArrayDescriptor(sdb::ArrayDescriptor::newArray(str.str(), new int[10] {524425, 235, 213, 123, 21, 66, 1, 6, 234, 5555}, 10));
		str.str(std::string());
		str.clear();
		str << "FLOAT_" << i;
		sdb::FieldDescriptor* f = new sdb::FieldDescriptor(sdb::FieldDescriptor::newField(str.str(), 3.14f));
		obj->addDescriptor(arr);
		obj->addDescriptor(f);
		db.addObject(obj);
		str.str(std::string());
		str.clear();
	}
	
	db.writeToFile("stressTest.sdb");
}
void createTestDB(void)
{
	sdb::ObjectDescriptor obj("TestOBJ");
	sdb::ObjectDescriptor obj2("TestOBJ2");
	sdb::ObjectDescriptor obj3("TestOBJ3");
	sdb::ObjectDescriptor obj4("INCEPTION OBJ...TAN TAN TAAAAAAN");
	
	sdb::FieldDescriptor i1 = sdb::FieldDescriptor::newField("IntField1", (int)556);
	sdb::ArrayDescriptor a1 = sdb::ArrayDescriptor::newArray("ArrayTest", new int[8] {234, 536, 46, 647, 67, 123, 8888, 9}, 8);
	sdb::FieldDescriptor i2 = sdb::FieldDescriptor::newField("IntField2", (int)24);
	sdb::FieldDescriptor b1 = sdb::FieldDescriptor::newField("bool 1", true);
	sdb::FieldDescriptor f1 = sdb::FieldDescriptor::newField("floaasdasdsadsat 1", 55.089f);
	sdb::StringDescriptor str("THIS STRING IS SO DAMN DEEP", "THE GAME");
	
	obj.addDescriptor(&i1);
	obj.addDescriptor(&a1);
	
	obj2.addDescriptor(&i2);
	obj2.addDescriptor(&b1);
	
	obj4.addDescriptor(&str);
	obj2.addDescriptor(&obj4);
	
	obj.addDescriptor(&obj2);
	
	obj3.addDescriptor(&f1);
	
	sdb::Database database("TestDB");
	database.addObject(&obj);
	database.addObject(&obj3);
	database.writeToFile("newDB.sdb");
	
	sdb::Database db = sdb::Database::deserialize("newDB.sdb");
}
void createTileDB(void)
{
	Tile t1;
	t1.set(true, Color(255, 0, 255, 127), 1920, 1080);
	Tile t2;
	t2.set(false, Color(0, 128, 255, 192), 800, 600);
	sdb::Database db("TilesDB");
	sdb::ObjectDescriptor t1_obj = t1.serialize("Tile_1");
	sdb::ObjectDescriptor t2_obj = t2.serialize("Tile_2");
	sdb::ObjectDescriptor obj("RootOBJ");
	obj.addDescriptor(&t1_obj);
	obj.addDescriptor(&t2_obj);
	db.addObject(&obj);
	db.writeToFile("tilesDB.sdb");
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	sdb::Utils::Timer timer;
	
	timer.reset();
	sdb::DynamicDatabase ddb("tilesDB.sdb");
	const sdb::ObjectDescriptor& root = ddb["RootOBJ"];
	
	//((const sdb::ObjectDescriptor&)root["Tile_1"]).object("color").field("a").editData("a", 111);
	
	std::cout << root.object("Tile_1").object("color").field("a").intValue() << "\n";
	double time = timer.elapsed();
	std::cout << time << "\n";
	
	Tile t1, t2;
	t1.deserialize(root.object("Tile_1"));
	t2.deserialize(root.object("Tile_2"));
	
	t1.print();
	std::cout << "\n\n\n";
	t2.print();
	
	return 0;
}
