#include <string>
#include <map>
#include "Object.h"
// extern int global_change_hint = 0;

enum class PlayerMove
{
  // Movement
  Left,
  Right,
  Front,
  Back,
  Die,
  PlayerState_Max
};

class PlayerControl : public Object
{
public:
  void init();
  void update();
  void draw();
  void SWITCHSTATUS();
  bool move_up = true;
  bool move_down = true;
  bool move_left = true;
  bool move_right = true;
  int tmp = 0;
  int global_change_hint;

  enum class PlayerState
  {
    MAIN_SCENE, // stand in the main scene
    UP_SCENE,   // stand in the up scene
    LEFT_SCENE, // stand in the left scene
    PlayerState_Max
  };
  PlayerState Pstate = PlayerState::MAIN_SCENE;
  enum class PlayerNowState
  {
    LIMIT_UP,    // go to the up scene
    LIMIT_DOWN,  // go to the down scene
    LIMIT_LEFT,  // go th left scene(if there is)
    LIMIT_RIGHT, // go to the right scene(if there is)
    NOTHING,     // nothing
    PlayerNowState_Max
  };
  PlayerNowState playernow = PlayerNowState::NOTHING;

private:
  PlayerMove PMstate = PlayerMove::Front;
  double speed = 3;
  std::map<PlayerMove, std::string> gifPath;
};