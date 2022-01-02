#include "tile.h"

Tile::Tile(void)
{
	id = rand() % 2000;
	x = 0;
	y = 0;
	solid = false;
}

sdb::ObjectDescriptor Tile::serialize(String name)
{
	sdb::ObjectDescriptor obj(name);
	obj.addDescriptor(sdb::FieldDescriptor::newFieldPtr("id", id));
	obj.addDescriptor(sdb::FieldDescriptor::newFieldPtr("x", x));
	obj.addDescriptor(sdb::FieldDescriptor::newFieldPtr("y", y));
	obj.addDescriptor(sdb::FieldDescriptor::newFieldPtr("solid", solid));
	sdb::ObjectDescriptor* col_obj = new sdb::ObjectDescriptor("color");
	col_obj->addDescriptor(sdb::FieldDescriptor::newFieldPtr("r", color.r));
	col_obj->addDescriptor(sdb::FieldDescriptor::newFieldPtr("g", color.g));
	col_obj->addDescriptor(sdb::FieldDescriptor::newFieldPtr("b", color.b));
	col_obj->addDescriptor(sdb::FieldDescriptor::newFieldPtr("a", color.a));
	obj.addDescriptor(col_obj);
	return obj;
}

void Tile::deserialize(const sdb::ObjectDescriptor& obj)
{
	id = obj.field("id").intValue();
	x = obj.field("x").intValue();
	y = obj.field("y").intValue();
	solid = obj.field("solid").boolValue();
	color.r = obj.object("color").field("r").intValue();
	color.g = obj.object("color").field("g").intValue();
	color.b = obj.object("color").field("b").intValue();
	color.a = obj.object("color").field("a").intValue();
}

void Tile::set(bool solid, Color color, int x, int y)
{
	this->x = x;
	this->y = y;
	this->solid = solid;
	this->color = color;
}
