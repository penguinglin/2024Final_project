#include "UI.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include <algorithm>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include "Player.h"
#include "Player_control.h"
// #include "towers/Tower.h"
#include "Level.h"
#include "towers/bag.h"
#include <vector>
#include <iostream>
#include <utility>
#include "global.h"

// fixed settings
constexpr char love_img_path[] = "./assets/image/love.png";
constexpr int love_img_padding = 5;
constexpr int bagitem_img_left_padding = 30;
constexpr int bagitem_img_top_padding = 30;
// draw tower shop items
constexpr int shop_offset_y = 30;
// 設定通用的 padding
const int top_padding = love_img_padding;
const int text_line_height = 20; // 每行文字間隔高度
// 生肉熟肉切換
bool meat = false;
int set_hint = 1;
// set timer for 2 second
int set_timer = 120;

void UI::init()
{
	DataCenter *DC = DataCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	love = IC->get(love_img_path);
	int tl_x = DC->game_field_length + bagitem_img_left_padding;
	int tl_y = bagitem_img_top_padding;
	int max_height = 0;

	// arrange item shop
	for (size_t i = 0; i < (size_t)(ItemType::ITEMTYPE_MAX); ++i)
	{
		ALLEGRO_BITMAP *bitmap = IC->get(ItemSetting::item_menu_img_path[i]);
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		if (tl_x + w > DC->window_width)
		{
			tl_x = DC->game_field_length + bagitem_img_left_padding;
			tl_y += max_height + bagitem_img_top_padding;
			max_height = 0;
		}
		bag_item.emplace_back(bitmap, Point{tl_x, tl_y}, std::make_pair(i, ItemSetting::have[i]));
		tl_x += w + bagitem_img_left_padding;
		max_height = std::max(max_height, h);
	}
	debug_log("<UI> state: change to HALT\n");
	state = STATE::HALT;
	on_item = -1;
}

void UI::update()
{
	DataCenter *DC = DataCenter::get_instance();
	const Point &mouse = DC->mouse;

	switch (state)
	{
	case STATE::HALT:
	{
		for (size_t i = 0; i < bag_item.size(); ++i)
		{
			auto &[bitmap, p, count] = bag_item[i];
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			// hover on a shop tower item
			if (mouse.overlap(Rectangle{p.x, p.y, p.x + w, p.y + h}))
			{
				on_item = i;
				debug_log("<UI> state: change to HOVER\n");
				state = STATE::HOVER;
				break;
			}
		}
		break;
	}
	case STATE::HOVER:
	{
		auto &[bitmap, p, count] = bag_item[on_item];
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		if (!mouse.overlap(Rectangle{p.x, p.y + text_line_height, p.x + w, p.y + h + text_line_height}))
		{
			on_item = -1;
			debug_log("<UI> state: change to HALT\n");
			state = STATE::HALT;
			break;
		}
		// click mouse left button
		if (DC->mouse_state[1] && !DC->prev_mouse_state[1])
		{
			// no money
			// if (price > DC->player->coin)
			// {
			// 	debug_log("<UI> Not enough money to buy tower %d.\n", on_item);
			// 	break;
			// }
			if (count.second == 0)
			{
				debug_log("<UI> Don't have the item %d.\n", on_item);
				break;
			}
			debug_log("<UI> state: change to SELECT\n");
			state = STATE::SELECT;
		}
		break;
	}
	case STATE::SELECT:
	{
		// click mouse left button: USE
		if (DC->mouse_state[1] && !DC->prev_mouse_state[1])
		{
			debug_log("<UI> state: change to USE\n");
			state = STATE::USE;
		}
		// click mouse right button: cancel
		if (DC->mouse_state[2] && !DC->prev_mouse_state[2])
		{
			on_item = -1;
			debug_log("<UI> state: change to HALT\n");
			state = STATE::HALT;
		}
		break;
	}
	case STATE::USE:
	{
		// check USEment legality
		// ALLEGRO_BITMAP *bitmap = Tower::get_bitmap(static_cast<TowerType>(on_item));
		// int w = al_get_bitmap_width(bitmap);
		// int h = al_get_bitmap_height(bitmap);
		// Rectangle USE_region{mouse.x - w / 2, mouse.y - h / 2, DC->mouse.x + w / 2, DC->mouse.y + h / 2};
		// bool USE = true;
		// // tower cannot be USEd on the road
		// USE &= (!DC->level->is_onroad(USE_region));
		// // tower cannot intersect with other towers
		// for (Tower *tower : DC->towers)
		// {
		// 	USE &= (!USE_region.overlap(tower->get_region()));
		// }
		// if (!USE)
		// {
		// 	debug_log("<UI> Tower USE failed.\n");
		// }
		// else
		// {
		// 	DC->towers.emplace_back(Tower::create_tower(static_cast<TowerType>(on_item), mouse));
		// 	DC->player->coin -= std::get<2>(tower_items[on_item]);
		// }
		auto &[bitmap, p, count] = bag_item[on_item];
		if (on_item != 0)
		{
			if (count.first == 3 && count.second == 3 && meat == false)
				meat = true;
			else
				count.second--;
			UI::draw();
		}
		debug_log("<UI> state: change to HALT\n");
		state = STATE::HALT;
		break;
	}
	}
}

void UI::draw()
{
	DataCenter *DC = DataCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	const Point &mouse = DC->mouse;
	al_draw_filled_rectangle(DC->game_field_length, 0, DC->window_width, DC->window_height, al_map_rgb(100, 100, 100));

	// draw HP
	const int &game_field_length = DC->game_field_length;
	// const int &player_HP = DC->player->HP;
	// int love_width = al_get_bitmap_width(love);
	// for (int i = 1; i <= player_HP; ++i)
	// {
	//   al_draw_bitmap(love, game_field_length - (love_width + love_img_padding) * i, love_img_padding, 0);
	// }

	// draw time bar
	const int &player_timer = DC->player->timer;
	// 設定通用的 padding
	const int top_padding = love_img_padding;
	const int text_line_height = 20; // 每行文字間隔高度
	al_draw_textf(
			FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
			game_field_length + love_img_padding, love_img_padding,
			ALLEGRO_ALIGN_LEFT, "Time: %5d", player_timer);

	// draw player hungry
	const int &player_hungry = DC->player->hungry;
	al_draw_textf(
			FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
			game_field_length + love_img_padding, top_padding + text_line_height,
			ALLEGRO_ALIGN_LEFT, "hungry: %3d", player_hungry);

	// draw a talking box under the window
	al_draw_filled_rectangle(0.f, 670.0f, 900.0f, 720.0f, al_map_rgba(20, 20, 20, 70));
	switch (DC->playerControl->global_change_hint)
	{
	case 0: // nothing
	{
		break;
	}
	case 1: // get close to the wood
	{
		al_draw_text(
				FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
				400.0f, 550.0f,
				ALLEGRO_ALIGN_CENTRE, "There are woods, you can get some if you want.");

		break;
	}
	case 2: // get close to the fire
	{
		if (DC->player->Get_fire == false)
			al_draw_text(
					FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
					400.0f, 550.0f,
					ALLEGRO_ALIGN_CENTRE, "Get something to let it warm.");
		else
		{
			al_draw_text(
					FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
					400.0f, 550.0f,
					ALLEGRO_ALIGN_CENTRE, "That's sooooooo nice!.");
		}

		break;
	}
	case 3: // get close to the boat
	{
		if (first_time_play_game)
		{
			al_draw_text(
					FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
					400.0f, 550.0f,
					ALLEGRO_ALIGN_CENTRE, "......????");
		}
		else if (DC->player->Fixboat == false)
			al_draw_text(
					FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
					400.0f, 550.0f,
					ALLEGRO_ALIGN_CENTRE, "You need something to fix it.");
		else
		{
			al_draw_text(
					FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
					400.0f, 550.0f,
					ALLEGRO_ALIGN_CENTRE, "You can leave now.Or look around?");
		}
		break;
	}
	case 4: // get close to two slide
	{
		al_draw_text(
				FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
				400.0f, 550.0f,
				ALLEGRO_ALIGN_CENTRE, "Do you want to look more?.");

		break;
	}
	case 5:
	{
		al_draw_text(
				FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
				400.0f, 550.0f,
				ALLEGRO_ALIGN_CENTRE, "You can leave now.Or look around?");
		break;
	}
	default:
		break;
	}

	for (auto &[bitmap, p, count] : bag_item)
	{
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);

		// 調整 y 座標以加入偏移量
		int draw_x = p.x;									// 保持 x 不變
		int draw_y = p.y + shop_offset_y; // 加入垂直偏移量

		if (count.second != 0)
		{
			if (count.first == 3 && count.second == 3 && meat == true)
			{
				auto &[_bitmap, _p, _count] = bag_item[4];
				int w = al_get_bitmap_width(_bitmap);
				int h = al_get_bitmap_height(_bitmap);
				al_draw_bitmap(_bitmap, draw_x, draw_y, 0);
				al_draw_rectangle(
						draw_x - 1, draw_y - 1,
						draw_x + w + 1, draw_y + h + 1,
						al_map_rgb(0, 0, 0), 1);
				al_draw_textf(
						FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
						draw_x + w / 2, draw_y + h,
						ALLEGRO_ALIGN_CENTRE, "%d", count.second);
			}
			else if (count.first == 3 && count.second == 2 && meat == true)
			{
				auto &[_bitmap, _p, _count] = bag_item[5];
				int w = al_get_bitmap_width(_bitmap);
				int h = al_get_bitmap_height(_bitmap);
				al_draw_bitmap(_bitmap, draw_x, draw_y, 0);
				al_draw_rectangle(
						draw_x - 1, draw_y - 1,
						draw_x + w + 1, draw_y + h + 1,
						al_map_rgb(0, 0, 0), 1);
				al_draw_textf(
						FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
						draw_x + w / 2, draw_y + h,
						ALLEGRO_ALIGN_CENTRE, "%d", count.second);
			}
			else if (count.first == 3 && count.second == 1 && meat == true)
			{
				auto &[_bitmap, _p, _count] = bag_item[6];
				int w = al_get_bitmap_width(_bitmap);
				int h = al_get_bitmap_height(_bitmap);
				al_draw_bitmap(_bitmap, draw_x, draw_y, 0);
				al_draw_rectangle(
						draw_x - 1, draw_y - 1,
						draw_x + w + 1, draw_y + h + 1,
						al_map_rgb(0, 0, 0), 1);
				al_draw_textf(
						FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
						draw_x + w / 2, draw_y + h,
						ALLEGRO_ALIGN_CENTRE, "%d", count.second);
			}
			else
			{
				int w = al_get_bitmap_width(bitmap);
				int h = al_get_bitmap_height(bitmap);
				al_draw_bitmap(bitmap, draw_x, draw_y, 0);
				al_draw_rectangle(
						draw_x - 1, draw_y - 1,
						draw_x + w + 1, draw_y + h + 1,
						al_map_rgb(0, 0, 0), 1);
				al_draw_textf(
						FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
						draw_x + w / 2, draw_y + h,
						ALLEGRO_ALIGN_CENTRE, "%d", count.second);
			}
		}

		// // 繪製塔的圖片
		// al_draw_bitmap(bitmap, draw_x, draw_y, 0);

		// // 繪製塔的邊框
		// al_draw_rectangle(
		// 		draw_x - 1, draw_y - 1,
		// 		draw_x + w + 1, draw_y + h + 1,
		// 		al_map_rgb(0, 0, 0), 1);

		// // 繪製塔的價格
		// al_draw_textf(
		// 		FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
		// 		draw_x + w / 2, draw_y + h,
		// 		ALLEGRO_ALIGN_CENTRE, "%d", price);
	}

	switch (state)
	{
		static Tower *selected_tower = nullptr;
	case STATE::HALT:
	{
		// No tower should be selected for HALT state.
		if (selected_tower != nullptr)
		{
			delete selected_tower;
			selected_tower = nullptr;
		}
		break;
	}
	case STATE::HOVER:
	{
		auto &[bitmap, p, count] = bag_item[on_item];
		if (count.second != 0)
		{
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			// Create a semitransparent mask covered on the hovered tower.
			al_draw_filled_rectangle(p.x, p.y + shop_offset_y, p.x + w, p.y + h + shop_offset_y, al_map_rgba(50, 50, 50, 64));
		}
		break;
	}
	case STATE::SELECT:
	{
		// // If a tower is selected, we new a corresponding tower for previewing purpose.
		// if (selected_tower == nullptr)
		// {
		// 	selected_tower = Tower::create_tower(static_cast<TowerType>(on_item), mouse);
		// }
		// else
		// {
		// 	selected_tower->shape->update_center_x(mouse.x);
		// 	selected_tower->shape->update_center_y(mouse.y);
		// }
	}
	case STATE::USE:
	{
		// // If we select a tower from menu, we need to preview where the tower will be built and its attack range.
		// ALLEGRO_BITMAP *bitmap = Tower::get_bitmap(static_cast<TowerType>(on_item));
		// al_draw_filled_circle(mouse.x, mouse.y, selected_tower->attack_range(), al_map_rgba(255, 0, 0, 32));
		// int w = al_get_bitmap_width(bitmap);
		// int h = al_get_bitmap_height(bitmap);
		// al_draw_bitmap(bitmap, mouse.x - w / 2, mouse.y - h / 2, 0);
		break;
	}
	}
}