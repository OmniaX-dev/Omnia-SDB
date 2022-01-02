#ifndef __TILE__H__
#define __TILE__H__

#include "sdb/sdb.h"
#include <iostream>

struct Color
{
	public:
		int r;
		int g;
		int b;
		int a;
		
	public:
		inline Color(void) : r(0), g(0), b(0), a(255) {}
		inline Color(int rr, int gg, int bb, int aa) : r(rr), g(gg), b(bb), a(aa) {}
};


class Tile : public sdb::Serializable
{
public:
	Tile(void);
	
	sdb::ObjectDescriptor serialize(String name);
	void deserialize(const sdb::ObjectDescriptor& obj);

	inline int getID(void) { return id; }
	void set(bool solid, Color col, int x = 0, int y = 0);
	
	inline void print(void)
	{
		std::cout << "ID: " << id << "\n";
		std::cout << "X: " << x << "\n";
		std::cout << "Y: " << y << "\n";
		if (solid)
			std::cout << "Solid: True\n";
		else
			std::cout << "Solid: False\n";
		std::cout << "Color: r = " << color.r << "; g = " << color.g << "; b = " << color.b << "; a = " << color.a << "\n";
	}

private:
	int id;

public:
	int x;
	int y;
	bool solid;
	Color color;
};

#endif
