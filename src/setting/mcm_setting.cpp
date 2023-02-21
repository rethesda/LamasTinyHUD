﻿#include "mcm_setting.h"
#include <SimpleIni.h>

namespace config {
    static const char* mcm_default_setting = R"(.\Data\MCM\Config\LamasTinyHUD\settings.ini)";
    static const char* mcm_config_setting = R"(.\Data\MCM\Settings\LamasTinyHUD.ini)";

    static uint32_t top_action_key;
    static uint32_t right_action_key;
    static uint32_t bottom_action_key;
    static uint32_t left_action_key;
    static uint32_t toggle_key;
    static uint32_t show_hide_key;
    static bool bottom_execute_key_combo_only;
    static bool draw_toggle_button;
    static uint32_t controller_set;
    static float config_button_hold_time;

    static float hud_image_scale_width;
    static float hud_image_scale_height;
    static float hud_image_position_width;
    static float hud_image_position_height;
    static float hud_slot_position_offset_x;
    static float hud_slot_position_offset_y;
    static float hud_key_position_offset;
    static float icon_scale_width;
    static float icon_scale_height;
    static float key_icon_scale_width;
    static float key_icon_scale_height;
    static float hud_arrow_image_scale_width;
    static float hud_arrow_image_scale_height;
    static float arrow_icon_scale_width;
    static float arrow_icon_scale_height;
    static float master_scale;
    static float slot_count_text_offset;
    static float toggle_key_offset_x;
    static float toggle_key_offset_y;
    static float current_items_offset_x;
    static float current_items_offset_y;
    static float slot_item_name_offset_horizontal_x;
    static float slot_item_name_offset_horizontal_y;
    static float slot_item_name_offset_vertical_x;
    static float slot_item_name_offset_vertical_y;
    static float arrow_slot_offset_x;
    static float arrow_slot_offset_y;
    static float arrow_slot_count_text_offset;

    static uint32_t background_transparency;
    static uint32_t background_icon_transparency;
    static uint32_t icon_transparency;
    static uint32_t key_transparency;
    static uint32_t text_transparency;
    static uint32_t icon_transparency_blocked;
    static float slot_count_text_font_size;
    static float current_items_font_size;
    static float arrow_count_font_size;
    static uint32_t current_items_red;
    static uint32_t current_items_green;
    static uint32_t current_items_blue;
    static uint32_t slot_count_red;
    static uint32_t slot_count_green;
    static uint32_t slot_count_blue;
    static uint32_t slot_button_feedback;
    static bool draw_current_items_text;
    static bool draw_item_name_text;

    static bool action_check;
    static bool empty_hand_setting;
    static bool hide_outside_combat;
    static float fade_timer_outside_combat;
    static bool disable_input_quick_loot;
    static bool elder_demon_souls;
    static uint32_t max_page_count;
    static uint32_t max_ammunition_type;
    static bool check_duplicate_items;
    static bool un_equip_ammo;
    static bool only_favorite_ammo;
    static bool prevent_consumption_of_last_dynamic_potion;
    static bool group_potions;
    static float potion_min_perfect;
    static float potion_max_perfect;
    static bool disable_re_equip_of_actives;
    static bool sort_arrow_by_quantity;

    void mcm_setting::read_setting() {
        logger::info("reading mcm ini files");

        const auto read_mcm = [&](const std::filesystem::path& path) {
            CSimpleIniA mcm;
            mcm.SetUnicode();
            mcm.LoadFile(path.string().c_str());

            top_action_key = static_cast<uint32_t>(mcm.GetLongValue("Controls", "uTopActionKey", 10));
            right_action_key = static_cast<uint32_t>(mcm.GetLongValue("Controls", "uRightActionKey", 11));
            bottom_action_key = static_cast<uint32_t>(mcm.GetLongValue("Controls", "uBottomActionKey", 12));
            left_action_key = static_cast<uint32_t>(mcm.GetLongValue("Controls", "uLeftActionKey", 13));
            toggle_key = static_cast<uint32_t>(mcm.GetLongValue("Controls", "uToggleKey", 27));
            show_hide_key = static_cast<uint32_t>(mcm.GetLongValue("Controls", "uShowHideKey", 26));
            bottom_execute_key_combo_only = mcm.GetBoolValue("Controls", "bBottomExecuteKeyComboOnly", false);
            controller_set = static_cast<uint32_t>(mcm.GetLongValue("Controls", "uControllerSet", 0));
            config_button_hold_time = static_cast<float>(mcm.GetDoubleValue("Controls", "fConfigButtonHoldTime", 5));
            draw_toggle_button = mcm.GetBoolValue("Controls", "bDrawToggleButton", true);

            hud_image_scale_width = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudImageScaleWidth", 0.23));
            hud_image_scale_height = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudImageScaleHeight", 0.23));
            hud_image_position_width =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudImagePositionWidth", 200));
            hud_image_position_height =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudImagePositionHeight", 775));
            hud_slot_position_offset_x =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudSlotPositionOffsetX", 105));
            hud_slot_position_offset_y =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudSlotPositionOffsetY", 105));
            hud_key_position_offset = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudKeyPositionOffset", 38));
            icon_scale_width = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fIconScaleWidth", 0.10));
            icon_scale_height = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fIconScaleHeight", 0.10));
            key_icon_scale_width = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fKeyIconScaleWidth", 0.38));
            key_icon_scale_height = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fKeyIconScaleHeight", 0.38));
            hud_arrow_image_scale_width =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudArrowImageScaleWidth", 0.13));
            hud_arrow_image_scale_height =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fHudArrowImageScaleHeight", 0.13));
            arrow_icon_scale_width = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fArrowIconScaleWidth", 0.05));
            arrow_icon_scale_height =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fArrowIconScaleHeight", 0.05));
            master_scale = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fMasterScale", 1));
            toggle_key_offset_x = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fToggleKeyOffsetX", 115));
            toggle_key_offset_y = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fToggleKeyOffsetY", 115));
            current_items_offset_x = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fCurrentItemsOffsetX", -15));
            current_items_offset_y = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fCurrentItemsOffsetY", 200));
            slot_count_text_offset = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fSlotCountTextOffset", 20));
            slot_item_name_offset_horizontal_x =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fSlotItemNameOffsetHorizontalX", -15));
            slot_item_name_offset_horizontal_y =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fSlotItemNameOffsetHorizontalY", 85));
            slot_item_name_offset_vertical_x =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fSlotItemNameOffsetVerticalX", 10));
            slot_item_name_offset_vertical_y =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fSlotItemNameOffsetVerticalY", 65));
            arrow_slot_offset_x = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fArrowSlotOffsetX", -115));
            arrow_slot_offset_y = static_cast<float>(mcm.GetDoubleValue("HudSetting", "fArrowSlotOffsetY", 115));
            arrow_slot_count_text_offset =
                static_cast<float>(mcm.GetDoubleValue("HudSetting", "fArrowSlotCountTextOffset", 12));

            background_transparency =
                static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uBackgroundTransparency", 150));
            background_icon_transparency =
                static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uBackgroundIconTransparency", 175));
            icon_transparency = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uIconTransparency", 125));
            key_transparency = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uKeyTransparency", 225));
            text_transparency = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uTextTransparency", 255));
            icon_transparency_blocked =
                static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uIconTransparencyBlocked", 50));
            slot_count_text_font_size =
                static_cast<float>(mcm.GetDoubleValue("GraphicSetting", "fSlotCountTextFontSize", 20));
            current_items_font_size =
                static_cast<float>(mcm.GetDoubleValue("GraphicSetting", "fCurrentItemsFontSize", 20));
            arrow_count_font_size = static_cast<float>(mcm.GetDoubleValue("GraphicSetting", "fArrowCountFontSize", 20));
            current_items_red = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uCurrentItemsRed", 255));
            current_items_green = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uCurrentItemsGreen", 255));
            current_items_blue = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uCurrentItemsBlue", 255));
            slot_count_red = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uSlotCountRed", 255));
            slot_count_green = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uSlotCountGreen", 255));
            slot_count_blue = static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uSlotCountBlue", 255));
            slot_button_feedback =
                static_cast<uint32_t>(mcm.GetLongValue("GraphicSetting", "uSlotButtonFeedback", 150));
            draw_current_items_text = mcm.GetBoolValue("GraphicSetting", "bDrawCurrentItemsText", true);
            draw_item_name_text = mcm.GetBoolValue("GraphicSetting", "bDrawItemNameText", true);

            action_check = mcm.GetBoolValue("MiscSetting", "bActionCheck", false);
            empty_hand_setting = mcm.GetBoolValue("MiscSetting", "bEmptyHandSetting", false);
            hide_outside_combat = mcm.GetBoolValue("MiscSetting", "bHideOutsideCombat", false);
            fade_timer_outside_combat =
                static_cast<float>(mcm.GetDoubleValue("MiscSetting", "fFadeTimerOutsideCombat", 5));
            disable_input_quick_loot = mcm.GetBoolValue("MiscSetting", "bDisableInputQuickLoot", false);
            elder_demon_souls = mcm.GetBoolValue("MiscSetting", "bEldenDemonSouls", false);
            max_page_count = static_cast<uint32_t>(mcm.GetLongValue("MiscSetting", "uMaxPageCount", 4));
            max_ammunition_type = static_cast<uint32_t>(mcm.GetLongValue("MiscSetting", "uMaxAmmunitionType", 3));
            check_duplicate_items = mcm.GetBoolValue("MiscSetting", "bCheckDuplicateItems", true);
            un_equip_ammo = mcm.GetBoolValue("MiscSetting", "bUnEquipAmmo", false);
            only_favorite_ammo = mcm.GetBoolValue("MiscSetting", "bOnlyFavoriteAmmo", false);
            prevent_consumption_of_last_dynamic_potion =
                mcm.GetBoolValue("MiscSetting", "bPreventConsumptionOfLastDynamicPotion", true);
            group_potions = mcm.GetBoolValue("MiscSetting", "bGroupPotions", false);
            potion_min_perfect = static_cast<float>(mcm.GetDoubleValue("MiscSetting", "fPotionMinPerfect", 0.7));
            potion_max_perfect = static_cast<float>(mcm.GetDoubleValue("MiscSetting", "fPotionMaxPerfect", 1.2));
            disable_re_equip_of_actives = mcm.GetBoolValue("MiscSetting", "bDisableReEquipOfActives", false);
            sort_arrow_by_quantity = mcm.GetBoolValue("MiscSetting", "bSortArrowByQuantity", false);
        };

        read_mcm(mcm_default_setting);
        read_mcm(mcm_config_setting);


        logger::info("finished reading mcm ini files. return.");
    }

    uint32_t mcm_setting::get_top_action_key() { return top_action_key; }
    uint32_t mcm_setting::get_right_action_key() { return right_action_key; }
    uint32_t mcm_setting::get_bottom_action_key() { return bottom_action_key; }
    uint32_t mcm_setting::get_left_action_key() { return left_action_key; }
    uint32_t mcm_setting::get_toggle_key() { return toggle_key; }
    uint32_t mcm_setting::get_show_hide_key() { return show_hide_key; }
    bool mcm_setting::get_bottom_execute_key_combo_only() { return bottom_execute_key_combo_only; }
    uint32_t mcm_setting::get_controller_set() { return controller_set; }
    bool mcm_setting::get_draw_toggle_button() { return draw_toggle_button; }
    float mcm_setting::get_config_button_hold_time() { return config_button_hold_time; }

    float mcm_setting::get_hud_image_scale_width() { return hud_image_scale_width * master_scale; }
    float mcm_setting::get_hud_image_scale_height() { return hud_image_scale_height * master_scale; }
    float mcm_setting::get_hud_image_position_width() { return hud_image_position_width; }
    float mcm_setting::get_hud_image_position_height() { return hud_image_position_height; }
    float mcm_setting::get_hud_slot_position_offset_x() { return hud_slot_position_offset_x * master_scale; }
    float mcm_setting::get_hud_slot_position_offset_y() { return hud_slot_position_offset_y * master_scale; }
    float mcm_setting::get_hud_key_position_offset() { return hud_key_position_offset * master_scale; }
    float mcm_setting::get_icon_scale_width() { return icon_scale_width * master_scale; }
    float mcm_setting::get_icon_scale_height() { return icon_scale_height * master_scale; }
    float mcm_setting::get_key_icon_scale_width() { return key_icon_scale_width * master_scale; }
    float mcm_setting::get_key_icon_scale_height() { return key_icon_scale_height * master_scale; }
    float mcm_setting::get_hud_arrow_image_scale_width() { return hud_arrow_image_scale_width * master_scale; }
    float mcm_setting::get_hud_arrow_image_scale_height() { return hud_arrow_image_scale_height * master_scale; }
    float mcm_setting::get_arrow_icon_scale_width() { return arrow_icon_scale_width * master_scale; }
    float mcm_setting::get_arrow_icon_scale_height() { return arrow_icon_scale_height * master_scale; }
    [[maybe_unused]] float mcm_setting::get_master_scale() { return master_scale; }
    float mcm_setting::get_slot_count_text_offset() { return slot_count_text_offset * master_scale; }
    float mcm_setting::get_toggle_key_offset_x() { return toggle_key_offset_x * master_scale; }
    float mcm_setting::get_toggle_key_offset_y() { return toggle_key_offset_y * master_scale; }
    float mcm_setting::get_current_items_offset_x() { return current_items_offset_x * master_scale; }
    float mcm_setting::get_current_items_offset_y() { return current_items_offset_y * master_scale; }
    float mcm_setting::get_slot_item_name_offset_horizontal_x() {
        return slot_item_name_offset_horizontal_x * master_scale;
    }
    float mcm_setting::get_slot_item_name_offset_horizontal_y() {
        return slot_item_name_offset_horizontal_y * master_scale;
    }
    float mcm_setting::get_slot_item_name_offset_vertical_x() {
        return slot_item_name_offset_vertical_x * master_scale;
    }
    float mcm_setting::get_slot_item_name_offset_vertical_y() {
        return slot_item_name_offset_vertical_y * master_scale;
    }
    float mcm_setting::get_arrow_slot_offset_x() { return arrow_slot_offset_x * master_scale; }
    float mcm_setting::get_arrow_slot_offset_y() { return arrow_slot_offset_y * master_scale; }
    float mcm_setting::get_arrow_slot_count_text_offset() { return arrow_slot_count_text_offset * master_scale; }

    uint32_t mcm_setting::get_background_transparency() { return background_transparency; }
    uint32_t mcm_setting::get_background_icon_transparency() { return background_icon_transparency; }
    uint32_t mcm_setting::get_icon_transparency() { return icon_transparency; }
    uint32_t mcm_setting::get_key_transparency() { return key_transparency; }
    uint32_t mcm_setting::get_text_transparency() { return text_transparency; }
    uint32_t mcm_setting::get_icon_transparency_blocked() { return icon_transparency_blocked; }
    float mcm_setting::get_slot_count_text_font_size() { return slot_count_text_font_size * master_scale; }
    float mcm_setting::get_current_items_font_size() { return current_items_font_size * master_scale; }
    float mcm_setting::get_arrow_count_font_size() { return arrow_count_font_size * master_scale; }
    uint32_t mcm_setting::get_current_items_red() { return current_items_red; }
    uint32_t mcm_setting::get_current_items_green() { return current_items_green; }
    uint32_t mcm_setting::get_current_items_blue() { return current_items_blue; }
    uint32_t mcm_setting::get_slot_count_red() { return slot_count_red; }
    uint32_t mcm_setting::get_slot_count_green() { return slot_count_green; }
    uint32_t mcm_setting::get_slot_count_blue() { return slot_count_blue; }
    bool mcm_setting::get_draw_current_items_text() { return draw_current_items_text; }
    uint32_t mcm_setting::get_slot_button_feedback() { return slot_button_feedback; }
    bool mcm_setting::get_draw_item_name_text() { return draw_item_name_text; }

    bool mcm_setting::get_action_check() { return action_check; }
    bool mcm_setting::get_empty_hand_setting() { return empty_hand_setting; }
    bool mcm_setting::get_hide_outside_combat() { return hide_outside_combat; }
    float mcm_setting::get_fade_timer_outside_combat() { return fade_timer_outside_combat; }
    bool mcm_setting::get_disable_input_quick_loot() { return disable_input_quick_loot; }
    bool mcm_setting::get_elden_demon_souls() { return elder_demon_souls; }
    uint32_t mcm_setting::get_max_page_count() { return max_page_count; }
    uint32_t mcm_setting::get_max_ammunition_type() { return max_ammunition_type; }
    bool mcm_setting::get_check_duplicate_items() { return check_duplicate_items; }
    bool mcm_setting::get_un_equip_ammo() { return un_equip_ammo; }
    bool mcm_setting::get_only_favorite_ammo() { return only_favorite_ammo; }
    bool mcm_setting::get_prevent_consumption_of_last_dynamic_potion() {
        return prevent_consumption_of_last_dynamic_potion;
    }
    bool mcm_setting::get_group_potions() { return group_potions; }
    float mcm_setting::get_potion_min_perfect() { return potion_min_perfect; }
    float mcm_setting::get_potion_max_perfect() { return potion_max_perfect; }
    bool mcm_setting::get_disable_re_equip_of_actives() { return disable_re_equip_of_actives; }
    bool mcm_setting::get_sort_arrow_by_quantity() { return sort_arrow_by_quantity; }
}
