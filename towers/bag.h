#ifndef TOWER_H_INCLUDED
#define TOWER_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include <allegro5/bitmap.h>
#include <string>
#include <array>

class Bullet;

// fixed settings
enum class ItemType
{
	ax,
	wood,
	torch,
	meat_raw,
	meat,
	meat_many,
	meat_some,
	// meat_raw_some,
	// meat_raw_many,
	ITEMTYPE_MAX
};
namespace ItemSetting
{
	const std::array<std::string, static_cast<int>(ItemType::ITEMTYPE_MAX)> item_menu_img_path = {
			"./assets/image/item/ax.png",				 // 斧頭
			"./assets/image/item/wood.png",			 // 木頭
			"./assets/image/item/torch.png",		 // 火柴
			"./assets/image/item/meat_raw.png",	 // 生肉- 還未使用
			"./assets/image/item/meat.png",			 // 熟肉- 還未使用
			"./assets/image/item/meat_many.png", // 熟肉 - 使用第一次
			"./assets/image/item/meat_some.png", // 熟肉 - 使用二次
			// "./assets/image/item/meat_raw_some.png",

			// "./assets/image/item/meat_raw_many.png",
	};
	const std::array<std::string, static_cast<int>(ItemType::ITEMTYPE_MAX)> item_menu = {
			"ax",
			"wood",
			"torch", // 火柴
			"meat_raw",
			"meat",
			"meat_many",
			"meat_some",
			// "meat_raw_some",

			// "meat_raw_many",
	};
	const std::array<int, static_cast<int>(ItemType::ITEMTYPE_MAX)> have = {2, 0, 0, 0, 0, 0, 0}; // 初始擁有數量 = 斧頭兩次使用
};

class Item : public Object
{
public:
	/**
	 * @brief Get the ALLEGRO_BITMAP* instance of the full image of a specific TowerType.
	 */
	static ALLEGRO_BITMAP *get_bitmap(ItemType type);
	/**
	 * @brief Create a Tower* instance by the type.
	 * @param type the type of a tower.
	 * @param p center point of the tower.
	 */
public:
	virtual ~Item() {}
	void update();
	// virtual bool attack(Object *target);
	void draw();
	Rectangle get_region() const;
	// irtual Bullet *create_bullet(Object *target) = 0;
	// virtual const double attack_range() const = 0;
	ItemType type;

private:
	/**
	 * @var attack_freq
	 * @brief Tower attack frequency. This variable will be set by its child classes.
	 **
	 * @var counter
	 * @brief Tower attack cooldown.
	 */
	// int attack_freq;
	int counter;
	bool have;
	ALLEGRO_BITMAP *bitmap;
};

#endif
