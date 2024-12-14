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
#include "towers/Tower.h"
#include "Level.h"
#include "towers/bag.h"
#include <vector>

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

void UI::init()
{
	DataCenter *DC = DataCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	love = IC->get(love_img_path);
	int tl_x = DC->game_field_length + bagitem_img_left_padding;
	int tl_y = bagitem_img_top_padding;
	int max_height = 0;
	for (size_t i = 0; i < (size_t)(InventoryType::INVENTORYTYPE_MAX); ++i) {
		ALLEGRO_BITMAP * bitmap = IC->get(InventorySetting::inventory_full_img_path[i]);
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		if (tl_x + w > DC->window_width)
		{
            tl_x = DC->game_field_length + bagitem_img_left_padding;
            tl_y += max_height + bagitem_img_top_padding;
            max_height = 0;
        }
		inventory[bitmap] = InventorySetting::item_init_num[i];
		tl_x += w + bagitem_img_left_padding;
		max_height = std::max(max_height, h);
	}
	// arrange item shop
	for (size_t i = 0; i < (size_t)(TowerType::TOWERTYPE_MAX); ++i)
	{
		ALLEGRO_BITMAP *bitmap = IC->get(TowerSetting::tower_menu_img_path[i]);
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		if (tl_x + w > DC->window_width)
		{
			tl_x = DC->game_field_length + bagitem_img_left_padding;
			tl_y += max_height + bagitem_img_top_padding;
			max_height = 0;
		}
		tower_items.emplace_back(bitmap, Point{tl_x, tl_y}, TowerSetting::tower_price[i]);
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
		for (size_t i = 0; i < tower_items.size(); ++i)
		{
			auto &[bitmap, p, price] = tower_items[i];
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			// hover on a shop tower item
			if (mouse.overlap(Rectangle{p.x, p.y + text_line_height, p.x + w, p.y + h + text_line_height}))
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
		auto &[bitmap, p, price] = tower_items[on_item];
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
			if (price > DC->player->coin)
			{
				debug_log("<UI> Not enough money to buy tower %d.\n", on_item);
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
		ALLEGRO_BITMAP *bitmap = Tower::get_bitmap(static_cast<TowerType>(on_item));
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		Rectangle USE_region{mouse.x - w / 2, mouse.y - h / 2, DC->mouse.x + w / 2, DC->mouse.y + h / 2};
		bool USE = true;
		// tower cannot be USEd on the road
		USE &= (!DC->level->is_onroad(USE_region));
		// tower cannot intersect with other towers
		for (Tower *tower : DC->towers)
		{
			USE &= (!USE_region.overlap(tower->get_region()));
		}
		if (!USE)
		{
			debug_log("<UI> Tower USE failed.\n");
		}
		else
		{
			DC->towers.emplace_back(Tower::create_tower(static_cast<TowerType>(on_item), mouse));
			DC->player->coin -= std::get<2>(tower_items[on_item]);
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
	al_draw_textf(
			FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
			game_field_length + love_img_padding, love_img_padding,
			ALLEGRO_ALIGN_LEFT, "Time: %5d", player_timer);

	// // draw coin
	// const int &player_coin = DC->player->coin;
	// al_draw_textf(
	//     FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
	//     game_field_length + love_img_padding, top_padding + text_line_height,
	//     ALLEGRO_ALIGN_LEFT, "coin: %5d", player_coin);

	// draw player hungry
	const int &player_hungry = DC->player->hungry;
	al_draw_textf(
			FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
			game_field_length + love_img_padding, top_padding + text_line_height,
			ALLEGRO_ALIGN_LEFT, "hungry: %3d", player_hungry);

	// draw a talking box under the window
	al_draw_filled_rectangle(0.f, 670.0f, 900.0f, 720.0f, al_map_rgba(50, 50, 50, 70));


	const int item_size = 64;      // Fixed size for inventory slots
    const int margin_x = 15;       // Horizontal margin between items
    const int margin_y = 30;       // Vertical margin between rows
    const int start_x = 915;       // Starting X position
    const int start_y = 80;        // Starting Y position
    const int max_row_width = 150; // Maximum width for a row before wrapping

    int current_x = start_x;
    int current_y = start_y;

    for (const auto& [item, count] : inventory) {
        if (!item || count <= 0) continue; // Skip invalid items or those with zero count

        // Get the item's dimensions
        int w = al_get_bitmap_width(item);
        int h = al_get_bitmap_height(item);

        // Draw the scaled bitmap
        al_draw_scaled_bitmap(item, 0, 0, w, h, current_x, current_y, item_size, item_size, 0);

        // Draw a border around the item for visibility
        al_draw_rectangle(
            current_x - 1, current_y - 1,
            current_x + item_size + 1, current_y + item_size + 1,
            al_map_rgb(0, 0, 0), 1);

        // Draw the count below the item
        al_draw_textf(
            FC->courier_new[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
            current_x + item_size / 2, current_y + item_size + 5, // Text position
            ALLEGRO_ALIGN_CENTER, "x%d", count);

        // Move to the next slot
        current_x += item_size + margin_x;

        // Wrap to the next row if the current row exceeds `max_row_width`
        if (current_x > start_x + max_row_width) {
            current_x = start_x;       // Reset to the start of the row
            current_y += item_size + margin_y; // Move down to the next row
        }
    }

	// for (auto &[bitmap, p, price] : tower_items)
	// {
	// 	int w = al_get_bitmap_width(bitmap);
	// 	int h = al_get_bitmap_height(bitmap);

	// 	// 調整 y 座標以加入偏移量
	// 	int draw_x = p.x;											// 保持 x 不變
	// 	int draw_y = p.y + shop_offset_y * 2; // 加入垂直偏移量

	// 	// 繪製塔的圖片
	// 	al_draw_bitmap(bitmap, draw_x, draw_y, 0);

	// 	// 繪製塔的邊框
	// 	al_draw_rectangle(
	// 			draw_x - 1, draw_y - 1,
	// 			draw_x + w + 1, draw_y + h + 1,
	// 			al_map_rgb(0, 0, 0), 1);

	// 	// 繪製塔的價格
	// 	al_draw_textf(
	// 			FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
	// 			draw_x + w / 2, draw_y + h,
	// 			ALLEGRO_ALIGN_CENTRE, "%d", price);
	// }

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
		auto &[bitmap, p, price] = tower_items[on_item];
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);

		// Create a semitransparent mask covered on the hovered tower.
		al_draw_filled_rectangle(p.x, p.y + shop_offset_y, p.x + w, p.y + h + shop_offset_y, al_map_rgba(50, 50, 50, 64));
		break;
	}
	case STATE::SELECT:
	{
		// If a tower is selected, we new a corresponding tower for previewing purpose.
		if (selected_tower == nullptr)
		{
			selected_tower = Tower::create_tower(static_cast<TowerType>(on_item), mouse);
		}
		else
		{
			selected_tower->shape->update_center_x(mouse.x);
			selected_tower->shape->update_center_y(mouse.y);
		}
	}
	case STATE::USE:
	{
		// If we select a tower from menu, we need to preview where the tower will be built and its attack range.
		ALLEGRO_BITMAP *bitmap = Tower::get_bitmap(static_cast<TowerType>(on_item));
		al_draw_filled_circle(mouse.x, mouse.y, selected_tower->attack_range(), al_map_rgba(255, 0, 0, 32));
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		al_draw_bitmap(bitmap, mouse.x - w / 2, mouse.y - h / 2, 0);
		break;
	}
	}
}

void UI::add_to_inventory(ALLEGRO_BITMAP* item, int count) {
    if (inventory.find(item) != inventory.end()) {
        inventory[item] += count; // Increment count if item already exists
    } else {
        inventory[item] = count; // Add new item with its count
    }
}

void UI::update_inventory(ALLEGRO_BITMAP* item, int count) {
    if (inventory.find(item) != inventory.end()) {
        inventory[item] += count; // Update item count
        if (inventory[item] <= 0) {
            inventory.erase(item); // Remove item if count is zero or negative
        }
    }
}

void UI::render_inventory() {
    const int margin = 10;        // Margin between items
    const int item_size = 64;     // Fixed size for inventory slots
    int x = 900;                  // Starting X position for sidebar
    int y = 30;                   // Starting Y position for items
    FontCenter* FC = FontCenter::get_instance(); // Assume FontCenter for text rendering

    for (const auto& [item, count] : inventory) {
        // Draw item image
        al_draw_scaled_bitmap(item, 0, 0, 
                              al_get_bitmap_width(item), al_get_bitmap_height(item), 
                              x, y, item_size, item_size, 
                              0);

        // Draw item count
        al_draw_textf(FC->courier_new[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
                      x + item_size + margin, y + item_size / 2,
                      ALLEGRO_ALIGN_LEFT, "x%d", count);

        y += item_size + margin; // Move to the next slot
    }
}
