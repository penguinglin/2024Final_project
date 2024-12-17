#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Player_control.h"
#include "Level.h"
// #include "end_circle.h"
#include <vector>
#include <algorithm>
#include "global.h"
#include <cmath>

#include <cstring>

// fixed settings
// game icon
constexpr char game_icon_img_path[] = "./assets/image/icon.png";
// game start sound
constexpr char game_start_sound_path[] = "./assets/sound/GameBGM.wav";
// menu pic
constexpr char startbg_img_path[] = "./assets/image/MAP_Start.png";
// game start background
constexpr char startgamebg_img_path[] = "./assets/image/MAP_Main_Empty.png";
// info background
constexpr char infobg_img_path[] = "./assets/image/MAP_Info.png";
constexpr char good_bg[] = "./assets/image/good_bg.png";
// control background
constexpr char controlbg_img_path[] = "./assets/image/MAP_Control.png";
// game start background sound
constexpr char background_sound_path[] = "./assets/sound/CountDownBGM.wav";
constexpr char end_good_sound_path[] = "./assets/sound/Bgm.ogg";
constexpr char end_bad_sound_path[] = "./assets/sound/death.wav";
constexpr char init_path[] = "./assets/image/Main_StartEmpty.png";
static bool BGM_played = false;
static bool is_played = false;
static bool is_played2 = false; // goodend music

namespace MenuSetting
{
	static constexpr char startmenu_img_path[50] = "./assets/image/mainScene";
	static constexpr char text_postfix[][10] = {"_START", "_CONTROL", "_INFO", "_EXIT"};
}

namespace GameSetting_FIRE
{
	static constexpr char game_item_path[50] = "./assets/image/GameMap/Game_item";
	static constexpr char item_postfix[][10] = {"fire", "nofire"};
}

namespace GameSetting_BOAT
{
	static constexpr char game_item_path[50] = "./assets/image/GameMap/Game_item";
	static constexpr char item_postfix[][15] = {"boat", "brokenboat"};
}

void Game::execute()
{
	DataCenter *DC = DataCenter::get_instance();
	// main game loop
	bool run = true;
	while (run)
	{
		// process all events here
		al_wait_for_event(event_queue, &event);
		switch (event.type)
		{
		case ALLEGRO_EVENT_TIMER:
		{
			run &= game_update();
			game_draw();
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
		{ // stop game
			run = false;
			break;
		}
		case ALLEGRO_EVENT_KEY_DOWN:
		{
			DC->key_state[event.keyboard.keycode] = true;
			break;
		}
		case ALLEGRO_EVENT_KEY_UP:
		{
			DC->key_state[event.keyboard.keycode] = false;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			DC->mouse.x = event.mouse.x;
			DC->mouse.y = event.mouse.y;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			DC->mouse_state[event.mouse.button] = true;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		{
			DC->mouse_state[event.mouse.button] = false;
			break;
		}
		default:
			break;
		}
	}
}

Game::Game()
{
	printf("Game is iGame()...\n");
	DataCenter *DC = DataCenter::get_instance();
	GAME_ASSERT(al_init(), "failed to initialize allegro.");

	// initialize allegro addons
	bool addon_init = true;
	addon_init &= al_init_primitives_addon();
	addon_init &= al_init_font_addon();
	addon_init &= al_init_ttf_addon();
	addon_init &= al_init_image_addon();
	addon_init &= al_init_acodec_addon();
	GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

	// initialize events
	bool event_init = true;
	event_init &= al_install_keyboard();
	event_init &= al_install_mouse();
	event_init &= al_install_audio();
	GAME_ASSERT(event_init, "failed to initialize allegro events.");

	// initialize game body
	GAME_ASSERT(
			display = al_create_display(DC->window_width, DC->window_height),
			"failed to create display.");
	GAME_ASSERT(
			timer = al_create_timer(1.0 / DC->FPS),
			"failed to create timer.");
	GAME_ASSERT(
			event_queue = al_create_event_queue(),
			"failed to create event queue.");

	debug_log("Game initialized.\n");
	game_init();
}

void Game::game_init()
{
	printf("Game is initializing...\n");
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();

	// set window icon
	game_icon = IC->get(game_icon_img_path);
	al_set_display_icon(display, game_icon);

	// register events to event_queue
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_mouse_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	// init sound setting
	SC->init();
	// init font setting
	FC->init();
	ui = new UI();
	ui->init();

	DC->level->init();
	DC->playerControl->init();

	// game start
	background = IC->get(startbg_img_path);
	al_draw_scaled_bitmap(
			background,
			0, 0,
			IC->get_origin_weight(background),
			IC->get_origin_weight(background),
			0, 0,
			900,
			900,
			0);
	debug_log("Background image: startbg_img_path\n");
	debug_log("Game state: change to START\n");
	state = STATE::START;
	buttonstate = ButtonState::NOTHING;
	fire_state = GameState_fire::NOFIRE;
	boat_state = GameState_boat::BROKENBOAT;
	control_button = Control_Button::Nothing;
	info_button = Info_Button::Nothing;
	al_start_timer(timer);
	// DC->player->Fixboat == false;
}

bool Game::game_update()
{
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	static ALLEGRO_SAMPLE_INSTANCE *background = nullptr;
	static ALLEGRO_SAMPLE_INSTANCE *instance = nullptr;
	const Point &start_mouse = DC->mouse;

	switch (state)
	{
	case STATE::START:
	{
		if (!is_played)
		{
			instance = SC->play(game_start_sound_path, ALLEGRO_PLAYMODE_LOOP);
			al_set_sample_instance_gain(instance, 0.5);
			is_played = true;
		}
		update_background_music(instance);

		// button state
		switch (buttonstate)
		{
		case ButtonState::NOTHING:
		{
			// place on start button
			if (start_mouse.overlap(Rectangle{380.0f, 300.0f, 560.0f, 350.1f}))
			{
				debug_log("<Start Button> state: change to HOVER1\n");

				buttonstate = ButtonState::HOVER1;
				break;
			}
			// place on control button
			if (start_mouse.overlap(Rectangle{380.0f, 370.0f, 560.0f, 420.0f}))
			{
				debug_log("<CONTROL Button> state: change to HOVER2\n");
				buttonstate = ButtonState::HOVER2;
				break;
			}
			// place on info button
			if (start_mouse.overlap(Rectangle{380.0f, 440.0f, 560.0f, 490.0f}))
			{
				debug_log("<INFO Button> state: change to HOVER3\n");
				buttonstate = ButtonState::HOVER3;
				break;
			}
			// place on EXIT button
			if (start_mouse.overlap(Rectangle{380.0f, 510.0f, 560.0f, 560.0f}))
			{
				debug_log("<ENIT Button> state: change to HOVER4\n");
				buttonstate = ButtonState::HOVER4;
				break;
			}

			break;
		}
		case ButtonState::HOVER1:
		{
			if (!start_mouse.overlap(Rectangle{380.0f, 300.0f, 560.0f, 350.1f}))
			{
				debug_log("<Button> state: change to NOTHING\n");
				buttonstate = ButtonState::NOTHING;
				break;
			}
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1])
			{
				debug_log("<Game> state: change to level\n");
				al_stop_sample_instance(instance);
				is_played = false;
				state = STATE::LEVEL;
				buttonstate = ButtonState::NOTHING;
				break;
			}
			break;
		}
		case ButtonState::HOVER2:
		{
			if (!start_mouse.overlap(Rectangle{380.0f, 370.0f, 560.0f, 420.0f}))
			{
				debug_log("<Button> state: change to NOTHING\n");
				buttonstate = ButtonState::NOTHING;
				break;
			}
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1])
			{
				debug_log("<Game> state: change to CONTROL\n");
				// al_stop_sample_instance(instance);
				// is_played = false;
				state = STATE::CONTROLL;
				buttonstate = ButtonState::NOTHING;
				break;
			}
			break;
		}
		case ButtonState::HOVER3:
		{
			if (!start_mouse.overlap(Rectangle{380.0f, 440.0f, 560.0f, 490.0f}))
			{
				debug_log("<Button> state: change to NOTHING\n");
				buttonstate = ButtonState::NOTHING;
				break;
			}

			if (DC->mouse_state[1] && !DC->prev_mouse_state[1])
			{
				debug_log("<Game> state: change to INFO\n");
				// al_stop_sample_instance(instance);
				// is_played = false;
				state = STATE::INFO;
				buttonstate = ButtonState::NOTHING;
				break;
			}
			break;
		}
		case ButtonState::HOVER4:
		{
			if (!start_mouse.overlap(Rectangle{380.0f, 510.0f, 560.0f, 560.0f}))
			{
				debug_log("<Button> state: change to NOTHING\n");
				buttonstate = ButtonState::NOTHING;
				break;
			}
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1])
			{
				debug_log("<Game> state: change to EXIT\n");
				// al_stop_sample_instance(instance);
				// is_played = false;
				buttonstate = ButtonState::NOTHING;
				state = STATE::EXIT;
				break;
			}
			break;
		}
		}

		break;
	}
	case STATE::CONTROLL:
	{
		update_background_music(instance);
		switch (control_button)
		{
		case Control_Button::Nothing:
		{
			if (DC->mouse.overlap(Rectangle{0.0f, 0.0f, 150.0f, 60.0f}))
			{
				control_button = Control_Button::Back;
				break;
			}
			break;
		}
		case Control_Button::Back:
		{
			if (!DC->mouse.overlap(Rectangle{0.0f, 0.0f, 150.0f, 60.0f}))
			{
				control_button = Control_Button::Nothing;
				break;
			}
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1])
			{
				debug_log("<Game> state: change to START\n");
				state = STATE::START;
				control_button = Control_Button::Nothing;
				break;
			}
			break;
		}
		}
		if (DC->key_state[ALLEGRO_KEY_BACKSPACE] && !DC->prev_key_state[ALLEGRO_KEY_BACKSPACE])
		{
			debug_log("<Game> state: change to START\n");
			state = STATE::START;
		}
		break;
	}
	case STATE::INFO:
	{
		update_background_music(instance);
		switch (info_button)
		{
		case Info_Button::Nothing:
		{
			if (DC->mouse.overlap(Rectangle{0.0f, 0.0f, 150.0f, 60.0f}))
			{
				info_button = Info_Button::Back;
				break;
			}
			break;
		}
		case Info_Button::Back:
		{
			if (!DC->mouse.overlap(Rectangle{0.0f, 0.0f, 150.0f, 60.0f}))
			{
				info_button = Info_Button::Nothing;
				break;
			}
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1])
			{
				debug_log("<Game> state: change to START\n");
				state = STATE::START;
				info_button = Info_Button::Nothing;
				break;
			}
			break;
		}
		}
		if (DC->key_state[ALLEGRO_KEY_BACKSPACE] && !DC->prev_key_state[ALLEGRO_KEY_BACKSPACE])
		{
			debug_log("<Game> state: change to START\n");
			state = STATE::START;
		}
		break;
	}
	case STATE::LEVEL:
	{
		// print global_change_hint ;
		debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
		if (!BGM_played)
		{
			background = SC->play(background_sound_path, ALLEGRO_PLAYMODE_LOOP);
			al_set_sample_instance_gain(background, 0.5);
			BGM_played = true;
		}
		update_background_music(background);

		// FIX Fire
		if (DC->key_state[ALLEGRO_KEY_1] && !DC->prev_key_state[ALLEGRO_KEY_1])
		{
			debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
			debug_log("<Player> : get fire \n");
			DC->player->Get_fire = true;
			fire_state = GameState_fire::FIRE;
		}
		// FIX Boat
		if (DC->key_state[ALLEGRO_KEY_2] && !DC->prev_key_state[ALLEGRO_KEY_2])
		{
			debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
			debug_log("<Player> : fix boat \n");
			DC->player->Fixboat = true;
			boat_state = GameState_boat::BOAT;
			first_time_play_game = 0;
		}
		// Fast timer
		if (DC->key_state[ALLEGRO_KEY_3] && !DC->prev_key_state[ALLEGRO_KEY_3])
		{
			debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
			debug_log("<Player> : decrease time \n");
			DC->player->timer = std::max(0, DC->player->timer - 50);
			// DC->player->timer = max(0, DC->player->timer - 50);
		}
		// Fast hungry
		if (DC->key_state[ALLEGRO_KEY_4] && !DC->prev_key_state[ALLEGRO_KEY_4])
		{
			debug_log("<Player> : increase hungry \n");
			// DC->player->hungry += 20;
			DC->player->hungry = std::min(200, DC->player->hungry);
		}

		// pause
		if (DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P])
		{
			SC->toggle_playing(background);
			debug_log("<Game> state: change to PAUSE\n");
			state = STATE::PAUSE;
		}
		// bad end
		if (DC->key_state[ALLEGRO_KEY_B] && !DC->prev_key_state[ALLEGRO_KEY_B])
		{
			al_stop_sample_instance(background);
			debug_log("<Game> state: change to BADEND\n");
			state = STATE::BADEND;
		}
		// good end
		if (DC->key_state[ALLEGRO_KEY_G] && !DC->prev_key_state[ALLEGRO_KEY_G])
		{
			al_stop_sample_instance(background);
			debug_log("<Game> state: change to GOODEND\n");
			state = STATE::GOODEND;
		}

		// bad end - player die
		if (DC->player->timer == 0)
		{
			debug_log("<Player> lost her time\n");
			debug_log("<Game> state: change to END\n");
			al_stop_sample_instance(background);
			state = STATE::BADEND;
		}
		if (DC->player->hungry == 200)
		{
			debug_log("<Player> hungry and die\n");
			debug_log("<Game> state: change to END\n");
			al_stop_sample_instance(background);
			state = STATE::BADEND;
		}

		// TODO: has a crush problem
		if (DC->key_state[ALLEGRO_KEY_BACKSPACE] && !DC->prev_key_state[ALLEGRO_KEY_BACKSPACE])
		{

			debug_log("<Game> state: change to START\n");
			game_restart(background);
		}

		// decided the mode to do the switch case
		switch (DC->playerControl->global_change_hint)
		{
		case 0: // nothing
		{
			debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
			break;
		}
		case 1: // get closer to the wood
		{
			if (DC->key_state[ALLEGRO_KEY_ENTER] && !DC->prev_key_state[ALLEGRO_KEY_ENTER])
			{
				// cut one ax
				DC->player->GetWood == true;
				debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
				debug_log("<Player> : get wood \n");

				// TODO modify the bag item => wood =3
			}
			break;
		}
		case 2: // get closer to the fire
		{
			if (DC->key_state[ALLEGRO_KEY_ENTER] && !DC->prev_key_state[ALLEGRO_KEY_ENTER])
			{
				// fix the fire
				if (DC->player->GetWood == true)

				{
					// make fire and get fire
					// print dc player hint
					debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
					debug_log("<Player> : get fire \n");
					DC->player->Get_fire = true;
					fire_state = GameState_fire::FIRE;
					// DC->player->GetWood = true;
					DC->player->GetMoreTime = true;

					// TODO modify the bag item => wood = wood -1
				}
				else
				{
					debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
					debug_log("<Player> : don't have wood to get fire \n");
				}

				// DC->player->update(); // update timer
			}
			break;
		}
		case 3: // get closer to the boat
		{
			// debug_log("<Player> : get closer to the boat \n");

			if (DC->key_state[ALLEGRO_KEY_ENTER] && !DC->prev_key_state[ALLEGRO_KEY_ENTER])
			{
				// fix the boat
				if (DC->player->GetWood == true)
				{
					// make boat and fix boat
					debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
					debug_log("<Player> : fix boat \n");
					DC->player->Fixboat = true;
					boat_state = GameState_boat::BOAT;
					// SWITCHSTATUS();
					DC->playerControl->global_change_hint = 5;
					debug_log("after fix boat = %d\n", DC->playerControl->global_change_hint);
					first_time_play_game = 0;

					// TODO modify the bag item => wood =0
				}
				else
				{
					debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
					debug_log("<Player> : don't have wood to fix boat \n");
				}
			}
			break;
		}
		case 4: // near the slide
		{
			if (DC->key_state[ALLEGRO_KEY_ENTER] && !DC->prev_key_state[ALLEGRO_KEY_ENTER])
			{
				// slide
				debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
				al_stop_sample_instance(background);
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::BADEND;
			}
			break;
		}
		case 5: // ok
		{
			if (DC->key_state[ALLEGRO_KEY_ENTER] && !DC->prev_key_state[ALLEGRO_KEY_ENTER])
			{
				// slide
				debug_log("global_change_hint = %d\n", DC->playerControl->global_change_hint);
				al_stop_sample_instance(background);
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::GOODEND;
			}
			break;
		}
		}

		break;
	}
	case STATE::PAUSE:
	{
		if (DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P])
		{
			SC->toggle_playing(background);
			debug_log("<Game> state: change to LEVEL\n");
			state = STATE::LEVEL;
		}
		break;
	}
	case STATE::GOODEND:
	{
		if (!is_played2)
		{
			background = SC->play(end_good_sound_path, ALLEGRO_PLAYMODE_LOOP);
			al_set_sample_instance_gain(background, 0.5);
			is_played2 = true;
		}
		update_background_music(background);
		if (DC->key_state[ALLEGRO_KEY_BACKSPACE] && !DC->prev_key_state[ALLEGRO_KEY_BACKSPACE])
		{
			debug_log("<Game> state: change to START\n");
			game_restart(background);
		}
		if (DC->key_state[ALLEGRO_KEY_ESCAPE] && !DC->prev_key_state[ALLEGRO_KEY_ESCAPE])
		{
			debug_log("<Game> state: change to END\n");
			return false;
		}
		break;
	}
	case STATE::BADEND:
	{
		if (!is_played2)
		{
			background = SC->play(end_bad_sound_path, ALLEGRO_PLAYMODE_ONCE);
			al_set_sample_instance_gain(background, 0.2);
			is_played2 = true;
		}
		update_background_music(background);
		if (DC->key_state[ALLEGRO_KEY_BACKSPACE] && !DC->prev_key_state[ALLEGRO_KEY_BACKSPACE])
		{
			debug_log("<Game> state: change to START\n");
			game_restart(background);
		}
		if (DC->key_state[ALLEGRO_KEY_ESCAPE] && !DC->prev_key_state[ALLEGRO_KEY_ESCAPE])
		{
			debug_log("<Game> state: change to END\n");
			return false;
		}
		break;
	}
	case STATE::EXIT:
	{
		return false;
	}
	}

	// If the game is not paused, we should progress update.
	// for example, the player's coin should be updated every second.
	if (state == STATE::LEVEL)
	{
		DC->player->update();				 // player's coin, hungry, timer
		DC->playerControl->update(); // player's movement
		SC->update();								 // sound
		ui->update();								 // bag item

		// If the game is not paused, we should progress update.
		if (state != STATE::START)
		{
			// DC->level->update();
			OC->update(); // monster, draw towerBullet
		}
	}

	// game_update is finished. The states of current frame will be previous states of the next frame.
	memcpy(DC->prev_key_state, DC->key_state, sizeof(DC->key_state));
	memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));
	return true;
}

void Game::game_draw()
{
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();

	// Flush the screen first.
	al_clear_to_color(al_map_rgb(100, 100, 100));

	if (state != STATE::BADEND && state != STATE::GOODEND)
	{
		al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(0, 0, 0, 255));
		if (DC->game_field_length < DC->window_width)
			al_draw_filled_rectangle(
					DC->game_field_length, 0,
					DC->window_width, DC->window_height,
					al_map_rgb(100, 100, 100));
		if (DC->game_field_height < DC->window_height)
			al_draw_filled_rectangle(
					0, DC->game_field_height,
					DC->window_width, DC->window_height,
					al_map_rgb(0, 0, 0));
		background = IC->get(startbg_img_path);
		al_draw_scaled_bitmap(
				background, 0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 900, 900, 0);
	}

	switch (state)
	{
	case STATE::START:
	{
		background = IC->get(startbg_img_path);
		al_draw_scaled_bitmap(
				background, 0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 900, 900, 0);

		char buffer[100];
		sprintf(buffer, "%s/%s.png", MenuSetting::startmenu_img_path, MenuSetting::text_postfix[static_cast<int>(MenuState::_START)]);
		ALLEGRO_BITMAP *menu_item0 = IC->get(buffer);
		al_draw_scaled_bitmap(
				menu_item0, 0, 0,
				IC->get_origin_weight(menu_item0), IC->get_origin_height(menu_item0),
				380, 307, 162, 45, 0);

		// al_draw_filled_rectangle(380, 370, 560, 420, al_map_rgb(0, 0, 0));
		sprintf(buffer, "%s/%s.png", MenuSetting::startmenu_img_path, MenuSetting::text_postfix[static_cast<int>(MenuState::_CONTROLL)]);
		ALLEGRO_BITMAP *menu_item1 = IC->get(buffer);
		al_draw_scaled_bitmap(
				menu_item1, 0, 0,
				IC->get_origin_weight(menu_item1), IC->get_origin_height(menu_item1),
				385, 375, 150, 40, 0);

		sprintf(buffer, "%s/%s.png", MenuSetting::startmenu_img_path, MenuSetting::text_postfix[static_cast<int>(MenuState::_INFO)]);
		ALLEGRO_BITMAP *menu_item2 = IC->get(buffer);
		al_draw_scaled_bitmap(
				menu_item2, 0, 0,
				IC->get_origin_weight(menu_item2), IC->get_origin_height(menu_item2),
				375, 445, 155, 45, 0);

		sprintf(buffer, "%s/%s.png", MenuSetting::startmenu_img_path, MenuSetting::text_postfix[static_cast<int>(MenuState::_EXIT)]);
		ALLEGRO_BITMAP *menu_item3 = IC->get(buffer);
		al_draw_scaled_bitmap(
				menu_item3, 0, 0,
				IC->get_origin_weight(menu_item3), IC->get_origin_height(menu_item3),
				378, 515, 158, 45, 0);

		switch (buttonstate)
		{
		case ButtonState::NOTHING:
		{
			break;
		}
		case ButtonState::HOVER1:
		{
			al_draw_filled_rectangle(380.0f, 304.0f, 538.0f, 350.1f, al_map_rgba(50, 50, 50, 70));
			break;
		}
		case ButtonState::HOVER2:
		{
			al_draw_filled_rectangle(380.0f, 372.0f, 538.0f, 420.0f, al_map_rgba(50, 50, 50, 70));
			break;
		}
		case ButtonState::HOVER3:
		{
			al_draw_filled_rectangle(380.0f, 440.0f, 538.0f, 490.0f, al_map_rgba(50, 50, 50, 70));
			break;
		}
		case ButtonState::HOVER4:
		{
			al_draw_filled_rectangle(380.0f, 510.0f, 538.0f, 558.0f, al_map_rgba(50, 50, 50, 70));
			break;
		}

		default:
			break;
		}

		break;
	}
	case STATE::CONTROLL:
	{
		al_draw_scaled_bitmap(
				IC->get(controlbg_img_path),
				0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 900, 900, 0);
		if (control_button == Control_Button::Back)
			al_draw_filled_rectangle(0, 0, 150, 60, al_map_rgba(50, 50, 50, 70));
		// al_draw_filled_rectangle(0, 0, 200, 70, al_map_rgba(50, 50, 50, 70));
		break;
	}
	case STATE::INFO:
	{
		al_draw_scaled_bitmap(
				IC->get(infobg_img_path),
				0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 900, 900, 0);
		if (info_button == Info_Button::Back)
			al_draw_filled_rectangle(0, 0, 150, 60, al_map_rgba(50, 50, 50, 70));
		break;
	}
	case STATE::LEVEL:
	{
		al_draw_scaled_bitmap(
				IC->get(startgamebg_img_path),
				0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 900, 900, 0);

		// fire
		char buffer[100];
		sprintf(buffer, "%s/%s.png", GameSetting_FIRE::game_item_path, GameSetting_FIRE::item_postfix[static_cast<int>(fire_state)]);
		ALLEGRO_BITMAP *fire_item = IC->get(buffer);
		al_draw_scaled_bitmap(
				fire_item, 0, 0,
				IC->get_origin_weight(fire_item), IC->get_origin_height(fire_item),
				420, 350, 65, 85, 0);
		// al_draw_filled_rectangle(420, 350, 485, 435, al_map_rgb(255, 255, 255));

		// boat
		sprintf(buffer, "%s/%s.png", GameSetting_BOAT::game_item_path, GameSetting_BOAT::item_postfix[static_cast<int>(boat_state)]);
		ALLEGRO_BITMAP *boat_item = IC->get(buffer);
		al_draw_scaled_bitmap(
				boat_item, 0, 0,
				IC->get_origin_weight(boat_item), IC->get_origin_height(boat_item),
				50, 425, 220, 180, 0);
		// al_draw_filled_rectangle(50, 425, 250, 550, al_map_rgb(255, 0, 0));

		al_draw_filled_rectangle(0, 520, 900, 620, al_map_rgba(100, 100, 100, 100));

		// user interface
		if (state != STATE::START)
		{
			// DC->level->draw();				 // map
			DC->playerControl->draw(); // player
			ui->draw();								 // tower
			OC->draw();								 // monster, towerBullet
		}
		break;
	}
	case STATE::PAUSE:
	{
		// background WITHOUT SCALE
		al_draw_scaled_bitmap(
				IC->get(init_path), 0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 900, 900, 0);
		// game layout cover
		al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
		al_draw_text(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width / 2., DC->window_height / 2.,
				ALLEGRO_ALIGN_CENTRE, "GAME PAUSED");
		break;
	}
	case STATE::BADEND:
	{
		// background WITHOUT SCALE
		al_draw_scaled_bitmap(
				IC->get(init_path), 0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 900, 900, 0);
		al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
		al_draw_text(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width / 2., DC->window_height / 2.,
				ALLEGRO_ALIGN_CENTRE, "You lose");

		break;
	}
	case STATE::GOODEND:
	{

		// background WITHOUT SCALE
		al_draw_scaled_bitmap(
				IC->get(init_path), 0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 900, 900, 0);
		// start the video
		al_draw_scaled_bitmap(
				IC->get(good_bg), 0, 0,
				IC->get_origin_weight(background),
				IC->get_origin_weight(background),
				0, 0, 2500, 2400, 0);
		al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
		al_draw_text(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width / 2., DC->window_height / 2.,
				ALLEGRO_ALIGN_CENTRE, "Pass the Game");

		break;
	}
	case STATE::EXIT:
	{
		break;
	}
	}

	al_flip_display();
}

void Game::game_restart(ALLEGRO_SAMPLE_INSTANCE *background)
{
	first_time_play_game = 0;
	DataCenter *DC = DC->get_instance();
	DataCenter *SC = SC->get_instance();
	if (BGM_played)
	{
		al_stop_sample_instance(background);
		BGM_played = false;
	}
	is_played = false;
	is_played2 = false;
	state = STATE::START;
	buttonstate = ButtonState::NOTHING;
	fire_state = GameState_fire::NOFIRE;
	boat_state = GameState_boat::BROKENBOAT;
	control_button = Control_Button::Nothing;
	info_button = Info_Button::Nothing;

	DC->playerControl->init();
	DC->player->init();
	DC->level->init();
}

void Game::update_background_music(ALLEGRO_SAMPLE_INSTANCE *background)
{
	DataCenter *DC = DataCenter::get_instance();
	if (DC->key_state[ALLEGRO_KEY_0] && !DC->prev_key_state[ALLEGRO_KEY_0])
	{

		al_set_sample_instance_gain(background, 0);
	}
	if (DC->key_state[ALLEGRO_KEY_9] && !DC->prev_key_state[ALLEGRO_KEY_9])
	{
		al_set_sample_instance_gain(background, 0.5);
	}
}

Game::~Game()
{
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);
}
