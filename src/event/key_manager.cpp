﻿#include "key_manager.h"
#include "handle/handle/edit_handle.h"
#include "handle/handle/page_handle.h"
#include "handle/setting/setting_execute.h"
#include "handle/setting/set_setting_data.h"
#include "setting/mcm_setting.h"
#include "util/helper.h"

namespace event {
    using event_result = RE::BSEventNotifyControl;

    key_manager* key_manager::get_singleton() {
        static key_manager singleton;
        return std::addressof(singleton);
    }

    void key_manager::sink() {
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(get_singleton());
        logger::info("start listening for input events."sv);
    }

    event_result key_manager::ProcessEvent(RE::InputEvent* const* a_event,
        [[maybe_unused]] RE::BSTEventSource<RE::InputEvent*>* a_event_source) {
        using event_type = RE::INPUT_EVENT_TYPE;
        using device_type = RE::INPUT_DEVICE;


        key_top_action_ = config::mcm_setting::get_top_action_key();
        key_right_action_ = config::mcm_setting::get_right_action_key();
        key_bottom_action_ = config::mcm_setting::get_bottom_action_key();
        key_left_action_ = config::mcm_setting::get_left_action_key();
        key_toggle_ = config::mcm_setting::get_toggle_key();
        button_press_modify_ = config::mcm_setting::get_slot_button_feedback();


        if (!is_key_valid(key_top_action_) || !is_key_valid(key_right_action_) || !is_key_valid(key_bottom_action_) || !
            is_key_valid(key_left_action_)) {
            return event_result::kContinue;
        }

        if (!a_event) {
            return event_result::kContinue;
        }

        const auto ui = RE::UI::GetSingleton();
        if (!ui) {
            return event_result::kContinue;
        }
        const auto interface_strings = RE::InterfaceStrings::GetSingleton();

        if (ui->IsMenuOpen(interface_strings->console)) {
            return event_result::kContinue;
        }

        for (auto event = *a_event; event; event = event->next) {
            if (event->eventType != event_type::kButton) {
                continue;
            }

            //this stays static_cast
            const auto button = static_cast<RE::ButtonEvent*>(event);
            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)


            key_ = button->idCode;
            if (key_ == k_invalid) {
                continue;
            }

            switch (button->device.get()) {
                // NOLINT(clang-diagnostic-switch-enum)
                case device_type::kMouse:
                    key_ += k_mouse_offset;
                    break;
                case device_type::kKeyboard:
                    key_ += k_keyboard_offset;
                    break;
                case device_type::kGamepad:
                    key_ = get_gamepad_index(static_cast<RE::BSWin32GamepadDevice::Key>(key_));
                    break;
                default:
                    continue;
            }

            if (const auto control_map = RE::ControlMap::GetSingleton(); !control_map->IsMovementControlsEnabled()) {
                continue;
            }

            /*if the game is not paused with the menu, it triggers the menu always in the background*/
            if (ui->GameIsPaused() || !ui->IsCursorHiddenWhenTopmost() ||
                !ui->IsShowingMenus() || !ui->GetMenu<RE::HUDMenu>()) {
                continue;
            }

            if (RE::UI::GetSingleton()->IsMenuOpen("LootMenu") && config::mcm_setting::get_disable_input_quick_loot()) {
                continue;
            }

            if (const auto control_map = RE::ControlMap::GetSingleton();
                !control_map || !control_map->IsMovementControlsEnabled() ||
                control_map->contextPriorityStack.back() != RE::UserEvents::INPUT_CONTEXT_ID::kGameplay) {
                continue;
            }

            if (config::mcm_setting::get_hide_outside_combat() && !RE::PlayerCharacter::GetSingleton()->IsInCombat()) {
                continue;
            }


            if (is_position_button(key_)) {
                if (button->IsHeld() && button->HeldDuration() >= config::mcm_setting::get_config_button_hold_time()) {
                    do_button_hold(key_);
                }
            }

            if (button->IsDown() && is_position_button(key_)) {
                logger::debug("configured key ({}) is down"sv, key_);
                //set slot to a different color
                const auto position_setting = handle::setting_execute::get_position_setting_for_key(key_);
                if (position_setting == nullptr) {
                    logger::trace("setting for key {} is null. return."sv, key_);
                    break;
                }
                position_setting->button_press_modify = button_press_modify_;
            }

            if (button->IsUp() && is_position_button(key_)) {
                logger::debug("configured Key ({}) is up"sv, key_);
                //set slot back to normal color
                const auto position_setting = handle::setting_execute::get_position_setting_for_key(key_);
                if (position_setting == nullptr) {
                    logger::trace("setting for key {} is null. return."sv, key_);
                    break;
                }
                position_setting->button_press_modify = ui::draw_full;
            }

            if (key_ == key_toggle_ && button->IsUp() && !is_toggle_down_) {
                is_toggle_down_ = false;
            }

            if (key_ == key_toggle_ && button->IsDown()) {
                is_toggle_down_ = true;
            }

            if (!button->IsDown()) {
                continue;
            }

            if (button->IsPressed() && is_key_valid(key_toggle_) && key_ == key_toggle_) {
                logger::debug("configured toggle key ({}) is pressed"sv, key_);
                if (!config::mcm_setting::get_elder_demon_souls()) {
                    const auto handler = handle::page_handle::get_singleton();
                    handler->set_active_page(handler->get_next_page_id());
                }

                reset_edit();
            }

            //TODO move into separate function
            if (is_toggle_down_ && scroll_position(key_) && button->IsPressed() &&
                is_key_valid(key_bottom_action_)) {
                const auto page_setting = handle::setting_execute::get_position_setting_for_key(key_);
                handle::setting_execute::execute_settings(page_setting->slot_settings, page_setting->pos);
            }

            if (is_position_button(key_)) {
                if (button->IsPressed()) {
                    do_button_press(key_);
                }
            }
        }
        return event_result::kContinue;
    }

    uint32_t key_manager::get_gamepad_index(const RE::BSWin32GamepadDevice::Key a_key) {
        using key = RE::BSWin32GamepadDevice::Key;

        uint32_t index;
        switch (a_key) {
            case key::kUp:
                index = 0;
                break;
            case key::kDown:
                index = 1;
                break;
            case key::kLeft:
                index = 2;
                break;
            case key::kRight:
                index = 3;
                break;
            case key::kStart:
                index = 4;
                break;
            case key::kBack:
                index = 5;
                break;
            case key::kLeftThumb:
                index = 6;
                break;
            case key::kRightThumb:
                index = 7;
                break;
            case key::kLeftShoulder:
                index = 8;
                break;
            case key::kRightShoulder:
                index = 9;
                break;
            case key::kA:
                index = 10;
                break;
            case key::kB:
                index = 11;
                break;
            case key::kX:
                index = 12;
                break;
            case key::kY:
                index = 13;
                break;
            case key::kLeftTrigger:
                index = 14;
                break;
            case key::kRightTrigger:
                index = 15;
                break;
            default: // NOLINT(clang-diagnostic-covered-switch-default)
                index = k_invalid;
                break;
        }

        return index != k_invalid ? index + k_gamepad_offset : k_invalid;
    }

    bool key_manager::is_key_valid(const uint32_t a_key) {
        if (a_key == k_invalid) {
            return false;
        }
        return true;
    }

    void key_manager::reset_edit() {
        if (edit_active_ != k_invalid) {
            //remove everything
            edit_active_ = k_invalid;
            handle::edit_handle::get_singleton()->init_edit(handle::position_setting::position::total);
        }
    }

    void key_manager::do_button_press(uint32_t a_key) {
        logger::debug("configured Key ({}) pressed"sv, a_key);

        const auto edit_handle = handle::edit_handle::get_singleton();
        auto position_setting = handle::setting_execute::get_position_setting_for_key(a_key);
        auto edit_page = edit_handle->get_page();
        auto edit_position = edit_handle->get_position();

        if (const auto page_handle = handle::page_handle::get_singleton();
            edit_position == position_setting->pos && edit_page == page_handle->
            get_active_page_id() && a_key == edit_active_) {
            util::helper::write_notification(fmt::format("Exit Edit Mode for Position {}, persisting Setting."sv,
                static_cast<uint32_t>(position_setting->pos)));

            const auto edit_data = edit_handle->get_hold_data();
            logger::trace("edit was active, setting new configuration for page {}, position {}, data size {}"sv,
                edit_page,
                static_cast<uint32_t>(edit_position),
                edit_data.size());

            if (config::mcm_setting::get_elder_demon_souls()) {
                handle::set_setting_data::set_queue_slot(edit_position, edit_data);
            } else {
                handle::set_setting_data::set_single_slot(edit_page,
                    edit_position,
                    edit_data);
            }

            //remove everything
            reset_edit();
            return;
        }

        if (position_setting == nullptr) {
            logger::trace("setting for key {} is null. return."sv, a_key);
        }


        if (config::mcm_setting::get_elder_demon_souls()) {
            //show next position for that
            //handler->set_active_page(handler->get_next_page_id());
            //TODO for now, trigger it with toggle button and its key

            const auto handler = handle::page_handle::get_singleton();
            const auto next_page = handler->get_next_page_id_position(position_setting->pos);
            if (!scroll_position(a_key)) {
                if (const auto next_position_setting = handler->get_page_setting(next_page, position_setting->pos);
                    handle::setting_execute::execute_settings(next_position_setting->slot_settings,
                        next_position_setting->pos)) {
                    handler->set_active_page_position(next_page, position_setting->pos);
                }
            } else {
                handler->set_active_page_position(next_page, position_setting->pos);
            }
        } else {
            handle::setting_execute::execute_settings(position_setting->slot_settings, position_setting->pos);
        }
    }

    void key_manager::do_button_hold(uint32_t a_key) {
        if (RE::PlayerCharacter::GetSingleton()->IsInCombat()) {
            reset_edit();
            return;
        }

        const auto edit_handle = handle::edit_handle::get_singleton();
        const auto page_setting = handle::setting_execute::get_position_setting_for_key(a_key);
        if (edit_handle->get_position() == handle::position_setting::position::total && edit_active_ ==
            k_invalid) {
            logger::debug("configured key ({}) is held, enter edit mode"sv, a_key);
            edit_handle->init_edit(page_setting->pos);

            util::helper::write_notification(fmt::format("Entered Edit Mode for Position {}"sv,
                static_cast<uint32_t>(page_setting->pos)));
            edit_active_ = a_key;
        }
    }

    bool key_manager::is_position_button(const uint32_t a_key) const {
        if (a_key == key_top_action_ || a_key == key_right_action_ || a_key == key_bottom_action_
            || a_key == key_left_action_) {
            return true;
        }
        return false;
    }

    bool key_manager::scroll_position(const uint32_t a_key) const {
        if (a_key == key_bottom_action_ || a_key == key_top_action_) {
            return true;
        }
        return false;
    }
}
