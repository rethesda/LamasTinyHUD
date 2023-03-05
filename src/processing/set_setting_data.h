﻿#pragma once
#include "handle/data/data_helper.h"
#include "handle/data/page/position_setting.h"
#include "handle/key_position_handle.h"
#include "handle/page_handle.h"

namespace processing {
    class set_setting_data {
    public:
        static void read_and_set_data();
        static void set_new_item_count_if_needed(const RE::TESBoundObject* a_object, int32_t a_count);
        static void set_single_slot(uint32_t a_page,
            handle::position_setting::position_type a_position,
            const std::vector<data_helper*>& a_data);
        static void set_queue_slot(handle::position_setting::position_type a_pos,
            const std::vector<data_helper*>& a_data);
        static void get_actives_and_equip();
        static void check_if_location_needs_block(RE::TESForm*& a_form, bool a_equipped);
        static void check_config_data();

    private:
        static void set_empty_slot(int a_page, int a_pos, handle::key_position_handle*& a_key_pos);
        static void set_slot(uint32_t a_page,
            handle::position_setting::position_type a_position,
            const std::string& a_form,
            uint32_t a_type,
            uint32_t a_hand,
            uint32_t a_action,
            const std::string& a_form_left,
            uint32_t a_type_left,
            uint32_t a_action_left,
            RE::ActorValue a_actor_value,
            handle::key_position_handle*& a_key_pos);
        static void set_new_item_count(RE::TESBoundObject* a_object, int32_t a_count);
        static void set_active_and_equip(handle::page_handle*& a_page_handle);
        static void process_config_data(handle::key_position_handle*& a_key_position);
        static void write_empty_config_and_init_active(handle::key_position_handle*& a_key_position);
        static void clear_hands();
        static void check_slot_data(uint32_t a_page,
            handle::position_setting::position_type a_position,
            const std::string& a_form,
            uint32_t a_type,
            uint32_t a_hand,
            uint32_t a_action,
            const std::string& a_form_left,
            uint32_t a_type_left,
            uint32_t a_action_left,
            RE::ActorValue a_actor_value,
            const std::string& a_section);
    };
}