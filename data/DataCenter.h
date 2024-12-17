#ifndef DATACENTER_H_INCLUDED
#define DATACENTER_H_INCLUDED

#include <map>
#include <vector>
#include <allegro5/keycodes.h>
#include <allegro5/mouse.h>
#include "../shapes/Point.h"

class Player;
class Level;
class Monster;
class Tower;
class Bullet;
class PlayerControl;
// class MENU;

/**
 * @brief Stores generic global data and relatively small data structures.
 * @details The globally used data such as FPS (frames per second), windows size, game region, and states of input devices (mouse and keyboard).
 * Player and Level are stored here mainly because the objects are small enough that do not require complex management.
 * Other specific data like game objects (towers, monsters ... etc) or ALLEGRO_BITMAP will be managed by other center objects.
 */
class DataCenter
{
public:
	static DataCenter *get_instance()
	{
		static DataCenter DC;
		return &DC;
	}
	~DataCenter();

public:
	double FPS;
	int window_width, window_height;

	int game_field_length;
	int game_field_height;

	bool key_state[ALLEGRO_KEY_MAX];

	bool prev_key_state[ALLEGRO_KEY_MAX];

	Point mouse;

	bool mouse_state[ALLEGRO_MOUSE_MAX_EXTRA_AXES];

	bool prev_mouse_state[ALLEGRO_MOUSE_MAX_EXTRA_AXES];

public:
	Player *player;
	PlayerControl *playerControl;
	// MENU *menu;
	Level *level;

	std::vector<Monster *> monsters;

	std::vector<Tower *> towers;

	std::vector<Bullet *> towerBullets;

private:
	DataCenter();
};

#endif