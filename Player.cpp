#include "Player.h"
#include "data/DataCenter.h"

// fixed settings
namespace PlayerSetting
{
	constexpr int init_HP = 3;

	constexpr int init_coin = 100;
	constexpr int coin_freq = 60;
	constexpr int coin_increase = 5;

	// ADD Timer
	constexpr int init_timer = 300;
	constexpr int timer_freq = 60;
	constexpr int timer_increase = 20;

	// ADD Hungry Counter
	constexpr int init_hungry = 20;
	constexpr int hungry_freq = 600;
	constexpr int hungry_decrease = 15;
};

Player::Player() : HP(PlayerSetting::init_HP), coin(PlayerSetting::init_coin), timer(PlayerSetting::init_timer), hungry(PlayerSetting::init_hungry)
{
	this->coin_freq = PlayerSetting::coin_freq;
	this->coin_increase = PlayerSetting::coin_increase;
	coin_counter = PlayerSetting::coin_freq;

	// ADD Timer
	this->Time_freq = PlayerSetting::timer_freq;
	this->Time_increase = PlayerSetting::timer_increase;
	Time_counter = PlayerSetting::timer_freq;

	// ADD Hungry Counter
	this->Hungry_freq = PlayerSetting::hungry_freq;
	this->Hungry_decrease = PlayerSetting::hungry_decrease;
	Hungry_counter = PlayerSetting::hungry_freq;
}

void Player::init()
{
	// TODO CHECK THE INIT OF PLAYER
	// KILLmonster = false;
	GetMoreTime = false;
	GetFood = false;
	Fixboat = false;
	Get_fire = false;
	GetWood = true;
	precounter = 180;
	HP = 3;
	coin = 0;
	timer = 300;
	hungry = 20;
}

void Player::update()
{
	DataCenter *DC = DataCenter::get_instance();
	if (precounter)
	{
		--precounter;
	}
	else
	{
		// precounter = 180;

		// ADD Timer
		if (Time_counter)
		{
			--Time_counter;
		}
		else if (GetMoreTime)
		{
			timer += Time_increase;
			GetMoreTime = false;
		}
		else if (Time_counter == 0)
		{
			--timer;
			Time_counter = Time_freq;
		}

		// ADD Hungry Counter
		if (Hungry_counter)
		{
			--Hungry_counter;
		}
		else if (GetFood)
		{
			hungry -= Hungry_decrease;
			GetFood = false;
		}
		else if (Hungry_counter == 0)
		{
			hungry += 10;
			Hungry_counter = Hungry_freq;
		}
	}
}
