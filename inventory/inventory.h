#ifndef INVENTORY_H_INCLUDED
#define INVENTORY_H_INCLUDED

#include "../Object.h"
#include "../shapes/Rectangle.h"
#include <allegro5/allegro.h> // Corrected include for ALLEGRO_BITMAP
#include <string>
#include <array>

// Enumeration for InventoryType
enum class InventoryType {
    AX,
    MEAT_RAW,
    MEAT,
    WOOD,
    INVENTORYTYPE_MAX // Always keep this as the last item for array sizing
};

namespace InventorySetting {
    // Define paths to inventory item images
    const std::array<std::string, static_cast<int>(InventoryType::INVENTORYTYPE_MAX)> inventory_full_img_path = {
        "./assets/image/item/ax.png",
        "./assets/image/item/meat_raw.png",
        "./assets/image/item/meat.png",
        "./assets/image/item/Wood.png"
    };
    const std::array<int, static_cast<int>(InventoryType::INVENTORYTYPE_MAX)> item_init_num = {2, 3, 0, 2};
};

class Inventory : public Object {
public:
    /**
     * @brief Get the ALLEGRO_BITMAP* instance of the full image for a specific InventoryType.
     * @param type The InventoryType for which the bitmap is required.
     * @return A pointer to the ALLEGRO_BITMAP instance.
     */
    static ALLEGRO_BITMAP* get_bitmap(InventoryType type);

    /**
     * @brief Constructor for the Inventory class.
     */
    Inventory(InventoryType type);

    /**
     * @brief Destructor for the Inventory class.
     */
    virtual ~Inventory() {}


    /**
     * @brief Set the quantity of the inventory item.
     * @param num The quantity to set.
     */
    void set_number(int num) {
        number = num;
    }

    /**
     * @brief Get the quantity of the inventory item.
     * @return The quantity of the item.
     */
    int get_number() const {
        return number;
    }

    /**
     * @brief Draw the inventory on the screen.
     */
    void draw();

private:
    ALLEGRO_BITMAP* bitmap; ///< Bitmap for the inventory item.
    int number; ///< Quantity of the item in the inventory.
};

#endif // INVENTORY_H_INCLUDED
