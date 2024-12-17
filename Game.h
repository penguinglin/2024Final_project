#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include "UI.h"
// int first_time_play_game;

class Game
{
public:
	void execute();

public:
	Game();
	~Game();
	void game_init();
	bool game_update();
	void game_draw();
	void update_background_music(ALLEGRO_SAMPLE_INSTANCE *background);
	int get_state() const { return (int)state; }
	void game_restart(ALLEGRO_SAMPLE_INSTANCE *background);

private:
	/**
	 * @brief States of the game process in game_update.
	 * @see Game::game_update()
	 */
	enum class STATE
	{
		START, // -> LEVEL, Info, Exit, CONTROLL
		CONTROLL,
		INFO,
		LEVEL,			// -> PAUSE, END
		LEVEL_UP,		//->level
		LEVEL_LEFT, //->level
		PAUSE,			// -> LEVEL
		BADEND,			// GAME OVER
		GOODEND,		// WIN
		EXIT
	};
	enum class ButtonState
	{
		NOTHING,
		HOVER1,
		HOVER2,
		HOVER3,
		HOVER4
	};
	enum class MenuState
	{
		_START,
		_CONTROLL,
		_INFO,
		_EXIT,
		MenuState_Max
	};
	enum class GameState_fire
	{
		FIRE,
		NOFIRE,
		GameState_fire_Max
	};
	enum class GameState_boat
	{
		BOAT,
		BROKENBOAT,
		GameState_boat_Max
	};
	enum class Info_Button
	{
		Nothing,
		Back
	};
	enum class Control_Button
	{
		Nothing,
		Back
	};

	GameState_fire fire_state;
	GameState_boat boat_state;
	ButtonState buttonstate;
	Info_Button info_button;
	Control_Button control_button;
	ALLEGRO_EVENT event;
	ALLEGRO_BITMAP *game_icon;
	ALLEGRO_BITMAP *background;
	STATE state;

private:
	ALLEGRO_DISPLAY *display;
	ALLEGRO_TIMER *timer;
	ALLEGRO_EVENT_QUEUE *event_queue;
	UI *ui;
	// std::map<MenuState, std::string> textPath;
	// MenuState state = MenuState::_START;
};
#endif
