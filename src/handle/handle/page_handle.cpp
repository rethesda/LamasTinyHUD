﻿#include "page_handle.h"
#include "equip/equip_slot.h"
#include "setting/custom_setting.h"
#include "handle/data/data_helper.h"
#include "handle/data/page/position_setting.h"
#include "setting/mcm_setting.h"
#include "util/string_util.h"

namespace handle {
    using mcm = config::mcm_setting;

    page_handle* page_handle::get_singleton() {
        static page_handle singleton;
        return std::addressof(singleton);
    }

    void page_handle::init_page(uint32_t a_page,
        const position_setting::position_type a_position,
        const std::vector<data_helper*>& data_helpers,
        const slot_setting::hand_equip a_hand,
        key_position_handle*& a_key_pos) {
        logger::trace("init page {}, position {}, data_size for settings {}, hand {} ..."sv,
            a_page,
            static_cast<uint32_t>(a_position),
            data_helpers.size(),
            static_cast<uint32_t>(a_hand));
        if (!this->data_) {
            this->data_ = new page_handle_data();
        }

        page_handle_data* data = this->data_;

        const auto slot_offset = mcm::get_hud_slot_position_offset();
        const auto key_offset = mcm::get_hud_key_position_offset();

        auto* page = new position_setting();
        page->position = a_position;
        page->page = a_page;

        auto* slots = new std::vector<slot_setting*>;
        for (const auto element : data_helpers) {
            logger::trace("processing form {}, type {}, action {}, left {}"sv,
                element->form ? util::string_util::int_to_hex(element->form->GetFormID()) : "null",
                static_cast<int>(element->type),
                static_cast<uint32_t>(element->action_type),
                element->left);
            auto* slot = new slot_setting();
            slot->form = element->form;
            slot->type = element->type;
            slot->action = element->action_type;
            slot->equip = a_hand;
            RE::BGSEquipSlot* equip_slot = nullptr;
            get_equip_slots(element->type, a_hand, equip_slot, element->left);
            get_item_count(element->form, slot->item_count, element->type);
            slot->equip_slot = equip_slot;

            slots->push_back(slot);
        }

        page->slot_settings = *slots;


        //TODO for now the right hand or the first setting defines the icon
        page->icon_type = get_icon_type(slots->front()->type, slots->front()->form);
        if (slots->size() == 2 && page->icon_type == ui::icon_image_type::icon_default) {
            logger::debug("Could not find an Icon with first setting, try next");
            page->icon_type = get_icon_type(slots->at(1)->type, slots->at(1)->form);
        }

        auto* draw = new position_draw_setting();
        draw->width_setting = mcm::get_hud_image_position_width();
        draw->height_setting = mcm::get_hud_image_position_height();
        draw->hud_image_scale_width = mcm::get_hud_image_scale_width();
        draw->hud_image_scale_height = mcm::get_hud_image_scale_height();
        draw->background_transparency = mcm::get_background_transparency();
        draw->key_icon_scale_width = mcm::get_key_icon_scale_width();
        draw->key_icon_scale_height = mcm::get_key_icon_scale_height();
        draw->icon_scale_width = mcm::get_icon_scale_width();
        draw->icon_scale_height = mcm::get_icon_scale_height();
        draw->background_icon_transparency = mcm::get_background_icon_transparency();
        draw->icon_transparency = mcm::get_icon_transparency();
        draw->key_transparency = mcm::get_key_transparency();
        draw->text_transparency = mcm::get_text_transparency();

        float offset_x = 0.f;
        float offset_y = 0.f;

        get_offset_values(a_position, slot_offset, offset_x, offset_y);

        draw->offset_slot_x = offset_x;
        draw->offset_slot_y = offset_y;

        get_offset_values(a_position, key_offset, offset_x, offset_y);
        draw->offset_key_x = offset_x;
        draw->offset_key_y = offset_y;

        //same for now
        draw->offset_text_x = config::mcm_setting::get_slot_count_text_offset();
        draw->offset_text_y = config::mcm_setting::get_slot_count_text_offset();

        page->draw_setting = draw;

        page->key = a_key_pos->get_key_for_position(a_position);
        page->font_size = config::mcm_setting::get_slot_count_text_font_size();


        if (mcm::get_elder_demon_souls()) {
            if (slots->front()->type != slot_setting::slot_type::empty || slots->size() == 2 && slots->at(1)->type !=
                slot_setting::slot_type::empty) {
                const auto config_page = static_cast<int>(a_page);
                if (const auto current_highest = get_highest_page_id_position(a_position); current_highest < config_page) {
                    set_highest_page_position(config_page, a_position);
                }
            }
        }

        data->page_settings[a_page][a_position] = page;
        logger::trace("done setting page {}, position {}."sv,
            a_page,
            static_cast<uint32_t>(a_position));
    }

    void page_handle::init_actives(uint32_t a_page, position_setting::position_type a_position) {
        if (!this->data_) {
            this->data_ = new page_handle_data();
        }
        page_handle_data* data = this->data_;
        logger::trace("init active page {} for postion {}"sv, a_page, static_cast<uint32_t>(a_position));
        data->active_page_per_position[a_position] = a_page;
    }

    void page_handle::set_active_page(const uint32_t a_page) const {
        if (!this->data_) {
            return;
        }
        page_handle_data* data = this->data_;

        logger::trace("set active page to {}"sv, a_page);
        data->active_page = a_page;
    }

    void page_handle::set_active_page_position(const uint32_t a_page, position_setting::position_type a_pos) const {
        if (!this->data_) {
            return;
        }
        page_handle_data* data = this->data_;
        logger::trace("set active page {} for postion {}"sv, a_page, static_cast<uint32_t>(a_pos));
        data->active_page_per_position[a_pos] = a_page;
    }

    void page_handle::set_highest_page_position(int a_page, position_setting::position_type a_pos) const {
        if (!this->data_) {
            return;
        }
        page_handle_data* data = this->data_;
        logger::trace("set highest page {} for postion {}"sv, a_page, static_cast<uint32_t>(a_pos));
        data->highest_set_page_per_position[a_pos] = a_page;
    }

    position_setting* page_handle::get_page_setting(const uint32_t a_page,
        const position_setting::position_type a_position) const {
        if (const page_handle_data* data = this->data_;
            data && !data->page_settings.empty() && data->page_settings.contains(a_page) && data->page_settings.
            at(a_page).contains(a_position)) {
            return data->page_settings.at(a_page).at(a_position);
        }
        return nullptr;
    }

    std::map<position_setting::position_type, position_setting*> page_handle::get_page(const uint32_t a_page) const {
        if (const page_handle_data* data = this->data_;
            data && !data->page_settings.empty() && data->page_settings.contains(a_page)) {
            return data->page_settings.at(a_page);
        }
        return {};
    }

    std::map<uint32_t, std::map<position_setting::position_type, position_setting*>> page_handle::get_pages() const {
        if (const page_handle_data* data = this->data_; data && !data->page_settings.empty()) {
            return data->page_settings;
        }
        return {};
    }

    std::map<position_setting::position_type, position_setting*> page_handle::get_active_page() const {
        if (config::mcm_setting::get_elder_demon_souls()) {
            std::map<position_setting::position_type, position_setting*> a_active;
            for (auto i = 0; i < static_cast<int>(position_setting::position_type::total); ++i) {
                const auto pos = static_cast<position_setting::position_type>(i);
                const auto page = get_active_page_id_position(pos);
                auto page_setting = get_page_setting(page, pos);
                a_active.insert({ pos, page_setting });
            }
            return a_active;
        }

        if (const page_handle_data* data = this->data_;
            data && !data->page_settings.empty() && data->page_settings.contains(data->active_page)) {
            return data->page_settings.at(data->active_page);
        }

        return {};
    }

    uint32_t page_handle::get_active_page_id() const {
        if (config::mcm_setting::get_elder_demon_souls()) {
            return 0;
        }
        if (const page_handle_data* data = this->data_; data) {
            return data->active_page;
        }
        return {};
    }

    uint32_t page_handle::get_next_page_id() const {
        if (const page_handle_data* data = this->data_; data) {
            //lets make it easy for now
            //we start at 0 so it is max count -1
            if (data->active_page < config::mcm_setting::get_max_page_count() - 1) {
                return data->active_page + 1;
            }
            return 0;
        }
        return {};
    }

    std::map<position_setting::position_type, position_setting*> page_handle::get_active_page_position(
        const position_setting::position_type a_position) const {
        if (const page_handle_data* data = this->data_; data) {
            if (const auto page = get_active_page_id_position(a_position);
                !data->page_settings.empty() && data->page_settings.contains(page)) {
                return data->page_settings.at(page);
            }
        }
        return {};
    }

    uint32_t page_handle::get_active_page_id_position(const position_setting::position_type a_position) const {
        if (const page_handle_data* data = this->data_;
            data && !data->active_page_per_position.empty() && data->active_page_per_position.contains(a_position)) {
            return data->active_page_per_position.at(a_position);
        }
        return 0;
    }

    uint32_t page_handle::get_next_page_id_position(const position_setting::position_type a_position) const {
        if (const page_handle_data* data = this->data_;
            data && !data->active_page_per_position.empty() && data->active_page_per_position.contains(a_position)) {
            if (const auto current = data->active_page_per_position.at(a_position);
                current < mcm::get_max_page_count() - 1) {
                return current + 1;
            }
            return 0;
        }
        return 0;
    }

    //could return the page as well here
    //use highest_page_id_position
    //since we reorder 0 to highest is always set
    uint32_t page_handle::get_next_non_empty_setting_for_position(
        const position_setting::position_type a_position) const {
        //if non found it will be 0
        const auto next = static_cast<int>(get_next_page_id_position(a_position));
        const auto highest_set = get_highest_page_id_position(a_position);
        if (next > highest_set) {
            return 0;
        }
        return next;

        /*auto next = static_cast<int>(get_next_page_id_position(a_position));
        auto max = static_cast<int>(mcm::get_max_page_count() - 1);
        logger::trace("checking up from next {} to max"sv, next, max);

        //if pos left check entry 2
        const position_setting* page_setting = nullptr;
        for (auto i = next; i <= max; ++i) {
            page_setting = get_page_setting(i, a_position);
            if (!page_setting->slot_settings.empty() && page_setting->slot_settings[0]->type !=
                slot_setting::slot_type::empty) {
                logger::trace("found setting at page {}, form is {}"sv,
                    i,
                    util::string_util::int_to_hex(page_setting->slot_settings[0]->form->formID));
                break;
            }
            if (page_setting->slot_settings.size() == 2 && !page_setting->slot_settings.empty() && page_setting->
                slot_settings[1]->type != slot_setting::slot_type::empty) {
                logger::trace("found (left) setting at page {}, form is {}"sv,
                    i,
                    util::string_util::int_to_hex(page_setting->slot_settings[1]->form->formID));
                break;
            }
            page_setting = nullptr;
        }
        if (page_setting) {
            logger::trace("Returning next {}"sv, page_setting->page);
            return page_setting->page;
        }

        for (auto i = 0; i < next; ++i) {
            page_setting = get_page_setting(i, a_position);
            if (!page_setting->slot_settings.empty() && page_setting->slot_settings[0]->type !=
                slot_setting::slot_type::empty) {
                logger::trace("found setting at page {}, form is {}"sv,
                    i,
                    util::string_util::int_to_hex(page_setting->slot_settings[0]->form->formID));
                break;
            }
            if (page_setting->slot_settings.size() == 2 && !page_setting->slot_settings.empty() && page_setting->
                slot_settings[1]->type != slot_setting::slot_type::empty) {
                logger::trace("found (left) setting at page {}, form is {}"sv,
                    i,
                    util::string_util::int_to_hex(page_setting->slot_settings[1]->form->formID));
                break;
            }
            page_setting = nullptr;
        }
        if (page_setting) {
            logger::trace("Returning next {}"sv, page_setting->page);
            return page_setting->page;
        }

        return 0;*/
        //const position_setting* page_setting = nullptr;
        /*for (auto i = next; i <= max; ++i) {
            page_setting = get_page_setting(i, a_position);
            if (page_setting && page_setting->type != slot_setting::slot_type::empty) {
                break;
            }
        }
        if (page_setting) {
            logger::trace("Returning next {}"sv, page_setting->page);
            return page_setting->page;
        }

        logger::trace("checking up from 0 to next {}"sv, next);
        for (auto i = 0; i <= next; ++i) {
            page_setting = get_page_setting(i, a_position);
            if (page_setting && page_setting->type != slot_setting::slot_type::empty) {
                break;
            }
        }
        
        if (page_setting) {
            logger::trace("Returning next {}"sv, page_setting->page);
            return page_setting->page;
        }

        //still nothing, just get the first one
        page_setting = get_page_setting(0, a_position);
        if (page_setting) {
            logger::trace("Returning next {}"sv, page_setting->page);
            return page_setting->page;
        }
        return 0;*/
    }

    int page_handle::get_highest_page_id_position(const position_setting::position_type a_position) const {
        if (const page_handle_data* data = this->data_;
            data && !data->highest_set_page_per_position.empty() && data->highest_set_page_per_position.contains(
                a_position)) {
            return data->highest_set_page_per_position.at(a_position);
        }
        return -1;
    }

    void page_handle::get_offset_values(const position_setting::position_type a_position,
        const float a_setting,
        float& offset_x,
        float& offset_y) {
        offset_x = 0.f;
        offset_y = 0.f;
        // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
        // ReSharper disable once CppIncompleteSwitchStatement
        switch (a_position) {
            case position_setting::position_type::top:
                offset_y = -a_setting;
                break;
            case position_setting::position_type::right:
                offset_x = a_setting;
                break;
            case position_setting::position_type::bottom:
                offset_y = a_setting;
                break;
            case position_setting::position_type::left:
                offset_x = -a_setting;
                break;
        }
    }

    void page_handle::get_equip_slots(const slot_setting::slot_type a_type,
        const slot_setting::hand_equip a_hand,
        RE::BGSEquipSlot*& a_slot,
        const bool a_left) {
        a_slot = nullptr;
        if ((a_type == slot_setting::slot_type::magic || a_type == slot_setting::slot_type::weapon) && a_hand
            == slot_setting::hand_equip::single || a_type == slot_setting::slot_type::empty) {
            a_slot = a_left ? item::equip_slot::get_left_hand_slot() : item::equip_slot::get_right_hand_slot();
        }
    }

    ui::icon_image_type page_handle::get_icon_type(const slot_setting::slot_type a_type, RE::TESForm*& a_form) {
        auto icon = ui::icon_image_type::icon_default;
        switch (a_type) {
            case slot_setting::slot_type::weapon:
                get_icon_for_weapon_type(a_form, icon);
                break;
            case slot_setting::slot_type::magic:
                get_icon_for_spell(a_form, icon);
                break;
            case slot_setting::slot_type::shout:
                icon = ui::icon_image_type::shout;
                break;
            case slot_setting::slot_type::power:
                icon = ui::icon_image_type::power;
                break;
            case slot_setting::slot_type::consumable:
                get_icon_for_consumable(a_form, icon);
                break;
            case slot_setting::slot_type::shield:
                //kinda useless atm, icon is set by the first setting, basically right hand
                icon = ui::icon_image_type::shield;
                break;
            case slot_setting::slot_type::armor:
                get_icon_for_item(a_form, icon);
                break;
            case slot_setting::slot_type::scroll:
                icon = ui::icon_image_type::scroll;
                break;
            case slot_setting::slot_type::misc:
            case slot_setting::slot_type::empty:
                icon = ui::icon_image_type::icon_default;
                break;
        }
        return icon;
    }

    void page_handle::get_icon_for_weapon_type(RE::TESForm*& a_form, ui::icon_image_type& a_icon) {
        if (!a_form || !a_form->IsWeapon()) {
            a_icon = ui::icon_image_type::icon_default;
            return;
        }
        switch (const auto weapon = a_form->As<RE::TESObjectWEAP>(); weapon->GetWeaponType()) {
            case RE::WEAPON_TYPE::kHandToHandMelee:
                a_icon = ui::icon_image_type::icon_default;
                break;
            case RE::WEAPON_TYPE::kOneHandSword:
                a_icon = ui::icon_image_type::sword_one_handed;
                break;
            case RE::WEAPON_TYPE::kOneHandDagger:
                a_icon = ui::icon_image_type::dagger;
                break;
            case RE::WEAPON_TYPE::kOneHandAxe:
                a_icon = ui::icon_image_type::axe_one_handed;
                break;
            case RE::WEAPON_TYPE::kOneHandMace:
                a_icon = ui::icon_image_type::mace;
                break;
            case RE::WEAPON_TYPE::kTwoHandSword:
                a_icon = ui::icon_image_type::sword_two_handed;
                break;
            case RE::WEAPON_TYPE::kTwoHandAxe:
                a_icon = ui::icon_image_type::axe_two_handed;
                break;
            case RE::WEAPON_TYPE::kBow:
                a_icon = ui::icon_image_type::bow;
                break;
            case RE::WEAPON_TYPE::kStaff:
                a_icon = ui::icon_image_type::staff;
                break;
            case RE::WEAPON_TYPE::kCrossbow:
                a_icon = ui::icon_image_type::crossbow;
                break;
        }
    }

    void page_handle::get_icon_for_spell(RE::TESForm*& a_form, ui::icon_image_type& a_icon) {
        if (!a_form && !a_form->Is(RE::FormType::Spell)) {
            return;
        }
        const auto spell = a_form->As<RE::SpellItem>();
        const auto effect = spell->GetCostliestEffectItem()->baseEffect;
        auto actor_value = effect->GetMagickSkill();
        if (actor_value == RE::ActorValue::kNone) {
            actor_value = effect->data.primaryAV;
        }

        switch (actor_value) {
            case RE::ActorValue::kAlteration:
            case RE::ActorValue::kConjuration:
            case RE::ActorValue::kDestruction:
                switch (effect->data.resistVariable) {
                    case RE::ActorValue::kResistFire:
                        a_icon = ui::icon_image_type::spell_fire;
                        break;
                    case RE::ActorValue::kResistFrost:
                        a_icon = ui::icon_image_type::spell_frost;
                        break;
                    case RE::ActorValue::kResistShock:
                        a_icon = ui::icon_image_type::spell_shock;
                        break;
                    default:
                        a_icon = ui::icon_image_type::spell_default_alt;
                }
                break;
            case RE::ActorValue::kIllusion:
                a_icon = ui::icon_image_type::spell_default;
                break;
            case RE::ActorValue::kRestoration:
                //might not fit all spells
                a_icon = ui::icon_image_type::spell_heal;
                break;
            default:
                a_icon = ui::icon_image_type::spell_default;
        }
    }

    void page_handle::get_icon_for_consumable(RE::TESForm*& a_form, ui::icon_image_type& a_icon) {
        if (!a_form && !a_form->Is(RE::FormType::AlchemyItem)) {
            return;
        }
        const auto alchemy_potion = a_form->As<RE::AlchemyItem>();

        if (alchemy_potion->IsFood()) {
            a_icon = ui::icon_image_type::food;
            return;
        }
        if (alchemy_potion->IsPoison()) {
            a_icon = ui::icon_image_type::poison_default;
            return;
        }

        const auto effect = alchemy_potion->GetCostliestEffectItem()->baseEffect;
        auto actor_value = effect->GetMagickSkill();

        if (actor_value == RE::ActorValue::kNone) {
            actor_value = effect->data.primaryAV;
        }

        switch (actor_value) {
            case RE::ActorValue::kHealth:
            case RE::ActorValue::kHealRateMult:
            case RE::ActorValue::kHealRate:
                a_icon = ui::icon_image_type::potion_health;
                break;
            default:
                a_icon = ui::icon_image_type::potion_default;
        }
    }

    void page_handle::get_item_count(RE::TESForm*& a_form,
        int32_t& a_count,
        const slot_setting::slot_type a_type) {
        if (a_type == slot_setting::slot_type::empty || !a_form) {
            a_count = 0;
            return;
        }
        if (a_type == slot_setting::slot_type::consumable || a_type == slot_setting::slot_type::scroll) {
            const auto player = RE::PlayerCharacter::GetSingleton();
            for (auto potential_items = player->GetInventory(); const auto& [item, invData] : potential_items) {
                if (invData.second->object->formID == a_form->formID) {
                    a_count = invData.first;
                    break;
                }
            }
        } else {
            a_count = 0;
        }
        logger::trace("Item {}, count {}"sv, a_form->GetName(), a_count);
    }

    void page_handle::get_icon_for_item(RE::TESForm*& a_form, ui::icon_image_type& a_icon) {
        if (!a_form && !a_form->IsArmor()) {
            return;
        }
        switch (const auto armor = a_form->As<RE::TESObjectARMO>(); armor->GetArmorType()) {
            case RE::BIPED_MODEL::ArmorType::kLightArmor:
                a_icon = ui::icon_image_type::armor_light;
                break;
            case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
                a_icon = ui::icon_image_type::armor_heavy;
                break;
            case RE::BIPED_MODEL::ArmorType::kClothing:
                a_icon = ui::icon_image_type::armor_clothing;
                break;
        }
    }
}