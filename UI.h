#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#include <allegro5/bitmap.h>
#include <vector>
#include <tuple>
#include "./shapes/Point.h"
#include "./inventory/inventory.h"
#include <map>

class UI
{
public:
	UI() {}
	void init();
	void update();
	void draw();
	void add_to_inventory(ALLEGRO_BITMAP* item, int count = 1);
    void update_inventory(ALLEGRO_BITMAP* item, int count);
    void render_inventory();

private:
	enum class STATE
	{
		HALT,		// -> HOVER
		HOVER,	// -> HALT, SELECT
		SELECT, // -> HALT, PLACE
		USE			// -> HALT
	};
	STATE state;
	ALLEGRO_BITMAP *love;
	// tower menu bitmap, (top-left x, top-left y), price
	std::vector<std::tuple<ALLEGRO_BITMAP *, Point, int>> tower_items;
	std::map<ALLEGRO_BITMAP*, int> inventory;
	int on_item;
};

#endif