﻿#include "setting_execute.h"
#include "key_position.h"
#include "page_handle.h"
#include "equip/equip_slot.h"
#include "equip/item/potion.h"
#include "equip/item/weapon.h"
#include "equip/magic/power.h"
#include "equip/magic/shout.h"
#include "equip/magic/spell.h"
#include "setting/mcm_setting.h"
#include "util/string_util.h"

namespace handle {
    using mcm = config::mcm_setting;

    void setting_execute::execute_settings(const std::vector<slot_setting*>& a_slots) {
        logger::trace("got {} settings execute"sv, a_slots.size());
        auto player = RE::PlayerCharacter::GetSingleton();
        for (auto slot : a_slots) {
            if (!slot->form || slot->type == slot_setting::slot_type::unset) {
                logger::warn("form is null, skipping."sv);
                continue;
            }
            logger::trace("executing setting for type {}, action {}, form {}, left {} ..."sv,
                static_cast<uint32_t>(slot->type),
                static_cast<uint32_t>(slot->action),
                slot->form ? util::string_util::int_to_hex(slot->form->GetFormID()) : "null",
                slot->equip_slot == item::equip_slot::get_left_hand_slot());
            execute_setting(slot, player);
        }
    }

    page_setting* setting_execute::get_page_setting_for_key(const uint32_t a_key) {
        const auto position = key_position::get_singleton()->get_position_for_key(a_key);
        if (position == page_setting::position::total) {
            logger::warn("nothing to do, nothing set. return."sv);
            return nullptr;
        }

        const auto page_handle = page_handle::get_singleton();
        page_setting* page_setting;
        uint32_t page = 0;
        if (mcm::get_elder_demon_souls()) {
            page_setting = page_handle->get_active_position(position);
        } else {
            page = page_handle->get_active_page_id();
            page_setting = page_handle->get_page_setting(page, position);
        }
        if (page_setting == nullptr) {
            logger::warn("nothing to do, nothing set. return."sv);
        }
        logger::debug("page {}, position is {}, setting count {}"sv,
            page,
            static_cast<uint32_t>(position),
            page_setting->slot_settings.size());

        return page_setting;
    }

    void setting_execute::execute_setting(slot_setting*& a_slot, RE::PlayerCharacter*& a_player) {
        switch (a_slot->type) {
            case slot_setting::slot_type::unset:
                logger::warn("nothing to do, nothing set"sv);
                break;
            case slot_setting::slot_type::consumable:
                item::potion::consume_potion(a_slot, a_player);
                break;
            case slot_setting::slot_type::magic:
                magic::spell::cast_magic(a_slot->form, a_slot->action, a_slot->equip_slot, a_player);
                break;
            case slot_setting::slot_type::shout:
                magic::shout::equip_shout(a_slot->form, a_player);
                break;
            case slot_setting::slot_type::power:
                magic::power::equip_or_cast_power(a_slot->form, a_slot->action, a_player);
                break;
            case slot_setting::slot_type::weapon:
                item::weapon::equip_weapon_or_shield(a_slot->form, a_slot->equip_slot, a_player);
                break;
            case slot_setting::slot_type::shield:
                item::weapon::equip_weapon_or_shield(a_slot->form, a_slot->equip_slot, a_player, false);
                break;
            case slot_setting::slot_type::armor:
                item::weapon::equip_armor(a_slot->form, a_player);
                break;
            case slot_setting::slot_type::scroll:
                magic::spell::cast_scroll(a_slot->form, a_slot->action, a_player);
                break;
            case slot_setting::slot_type::misc:
                //TODO
                logger::warn("ignoring miscitem."sv);
                break;
            case slot_setting::slot_type::empty:
                item::equip_slot::unequip_hand(a_slot->equip_slot, a_player, a_slot->action);
                break;
        }
    }
}
