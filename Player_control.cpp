#include "Player_control.h"
#include "data/DataCenter.h"
#include "Player.h"
#include "data/GIFCenter.h"
#include "algif5/algif.h"
#include "shapes/Rectangle.h"
#include "Utils.h"
#include "Game.h"
#include "global.h"

// extern Game *game;

namespace PlayerSetting
{
  static constexpr char gif_root_path[50] = "./assets/gif/player";
  // static constexpr char gif_root_path[50] = "./assets/gif/Hero";
  static constexpr char gif_postfix[][10] = {"left", "right", "front", "back"};
  static constexpr int playmove_weight = 870;
  static constexpr int playmove_height = 490;
}

void PlayerControl::init()
{
  Pstate = PlayerState::MAIN_SCENE;
  playernow = PlayerNowState::NOTHING;
  move_up = true;
  move_down = true;
  move_left = true;
  move_right = true;
  global_change_hint = 0;

  tmp = 0;
  // Load GIFs
  for (size_t type = 0; type < static_cast<size_t>(PlayerMove::PlayerState_Max); ++type)
  {
    char buffer[100];
    sprintf(buffer, "%s/player_%s.gif", PlayerSetting::gif_root_path, PlayerSetting::gif_postfix[static_cast<int>(type)]);
    gifPath[static_cast<PlayerMove>(type)] = std::string{buffer};
  }

  // Set initial position
  GIFCenter *GIFC = GIFCenter::get_instance();
  ALGIF_ANIMATION *gif = GIFC->get(gifPath[PMstate]);
  DataCenter *DC = DataCenter::get_instance();
  shape.reset(new Rectangle{DC->window_width / 2, DC->window_height / 2, DC->window_width / 2 + gif->width, DC->window_height / 2 + gif->height});
}
void PlayerControl::SWITCHSTATUS()
{
  global_change_hint = 5;
}
void PlayerControl::update()
{
  DataCenter *DC = DataCenter::get_instance();
  /*
  enum class STATE
  {
    START, // -> LEVEL, Info, Exit, CONTROLL
    CONTROLL,
    INFO,
    LEVEL,			// -> PAUSE, END
    LEVEL_UP,		//->level
    LEVEL_LEFT, //->level
    PAUSE,			// -> LEVEL
    END,				// -> LEVEL, BADEND
    BADEND,			// GAME OVER
    GOODEND,		// WIN
    EXIT
  };
  */

  // y<=320 && x>=430 && x<=440
  if (shape->center_x() <= 45 || shape->center_x() >= 855)
  {
    // near the slide
    global_change_hint = 4;
    // debug_log("near the slide\n");
  }
  else if (shape->center_y() <= 335)
  {
    // get closer to the wood
    global_change_hint = 1;
    // debug_log("get closer to the wood\n");
  }
  else if (shape->center_x() >= 420 && shape->center_x() <= 485)
  {
    if (shape->center_y() >= 350 && shape->center_y() <= 435)
      // get closer to the fire (420, 350, 485, 435)
      global_change_hint = 2;
    // debug_log("get closer to the fire\n");
  }
  else if (shape->center_x() >= 50 && shape->center_x() <= 270)
  {
    if (shape->center_y() >= 425 && shape->center_y() <= 605)
    {
      if (first_time_play_game)
        global_change_hint = 3;
      else if (DC->player->Fixboat == true)
        global_change_hint = 5;
      else
        global_change_hint = 3;
    }
  }
  else
  {
    global_change_hint = 0;
  }

  if (DC->key_state[ALLEGRO_KEY_W] || DC->key_state[ALLEGRO_KEY_UP])
  {
    if ((shape->center_y() - speed >= 320) && (shape->center_y() - speed <= PlayerSetting::playmove_height))
    {
      shape->update_center_y(shape->center_y() - speed);
      PMstate = PlayerMove::Back;
    }
  }
  else if (DC->key_state[ALLEGRO_KEY_S] || DC->key_state[ALLEGRO_KEY_DOWN])
  {
    if ((shape->center_y() + speed >= 320) && (shape->center_y() + speed <= PlayerSetting::playmove_height))
    {
      shape->update_center_y(shape->center_y() + speed);
      PMstate = PlayerMove::Front;
    }
  }
  else if (DC->key_state[ALLEGRO_KEY_A] || DC->key_state[ALLEGRO_KEY_LEFT])
  {
    if ((shape->center_x() - speed >= 30) && (shape->center_x() - speed <= PlayerSetting::playmove_weight))
    {
      shape->update_center_x(shape->center_x() - speed);
      PMstate = PlayerMove::Left;
    }
  }
  else if (DC->key_state[ALLEGRO_KEY_D] || DC->key_state[ALLEGRO_KEY_RIGHT])
  {
    if ((shape->center_x() + speed >= 30) && (shape->center_x() + speed <= PlayerSetting::playmove_weight))
    {
      shape->update_center_x(shape->center_x() + speed);
      PMstate = PlayerMove::Right;
    }
  }
}

void PlayerControl::draw()
{
  GIFCenter *GIFC = GIFCenter::get_instance();
  ALGIF_ANIMATION *gif = GIFC->get(gifPath[PMstate]);
  algif_draw_gif(gif, shape->center_x() - gif->width / 2, shape->center_y() - gif->height / 2, 0);
}