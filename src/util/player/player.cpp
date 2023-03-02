#include "player.h"
#include "handle/data/ammo_data.h"
#include "handle/handle/ammo_handle.h"
#include "setting/mcm_setting.h"
#include "util/helper.h"
#include "util/string_util.h"

namespace util {
    std::vector<RE::TESForm*> player::get_spells(const bool a_instant, const bool a_single) {
        //easier just to use items that have been favourited, just filter them
        std::vector<RE::TESForm*> spell_list;

        for (auto magic_favorites = RE::MagicFavorites::GetSingleton()->spells; auto form : magic_favorites) {
            if (form->Is(RE::FormType::Spell)) {
                if (const auto spell = form->As<RE::SpellItem>();
                    spell->GetSpellType() == RE::MagicSystem::SpellType::kSpell) {
                    logger::trace("spell is {}, casting {}, is two_handed {}, spelltype {}"sv,
                        spell->GetName(),
                        static_cast<uint32_t>(spell->GetCastingType()),
                        spell->IsTwoHanded(),
                        static_cast<uint32_t>(spell->GetSpellType()));
                    if (spell->GetCastingType() == RE::MagicSystem::CastingType::kConcentration && a_instant) {
                        logger::debug("skipping spell {} because it does not work will with instant cast"sv,
                            spell->GetName());
                        continue;
                    }

                    if ((spell->IsTwoHanded() && !a_single) || (!spell->IsTwoHanded() && a_single)) {
                        spell_list.push_back(form);
                    }
                } else {
                    logger::trace(" {} is not a spell, not needed here form type {}"sv,
                        form->GetName(),
                        form->GetFormType());
                }
            }
        }


        logger::trace("spell list is size {}. return."sv, spell_list.size());
        return spell_list;
    }

    std::vector<RE::TESForm*> player::get_powers() {
        //easier just to use items that have been favourited, just filter them
        std::vector<RE::TESForm*> power_list;

        for (auto magic_favorites = RE::MagicFavorites::GetSingleton()->spells; auto form : magic_favorites) {
            if (form->Is(RE::FormType::Spell)) {
                if (const auto spell = form->As<RE::SpellItem>();
                    spell->GetSpellType() == RE::MagicSystem::SpellType::kPower ||
                    spell->GetSpellType() == RE::MagicSystem::SpellType::kLesserPower) {
                    logger::trace("spell is {}, casting {}, is two_handed {}, spelltype {}"sv,
                        spell->GetName(),
                        static_cast<uint32_t>(spell->GetCastingType()),
                        spell->IsTwoHanded(),
                        static_cast<uint32_t>(spell->GetSpellType()));

                    power_list.push_back(form);
                } else {
                    logger::trace("{} is not a power, not needed here form type {}"sv,
                        form->GetName(),
                        form->GetFormType());
                }
            }
        }

        logger::trace("power list is size {}. return."sv, power_list.size());
        return power_list;
    }

    std::vector<RE::TESForm*> player::get_shouts() {
        //easier just to use items that have been favourited, just filter them
        std::vector<RE::TESForm*> shout_list;

        for (auto magic_favorites = RE::MagicFavorites::GetSingleton()->spells; auto form : magic_favorites) {
            if (form->Is(RE::FormType::Shout)) {
                logger::trace("shout name is {}"sv, form->GetName());
                shout_list.push_back(form);
            }
        }

        logger::trace("shout list is size {}. return."sv, shout_list.size());
        return shout_list;
    }

    std::map<RE::TESBoundObject*, std::pair<int, std::unique_ptr<RE::InventoryEntryData>>>
        player::get_inventory_weapon_items(RE::PlayerCharacter*& a_player) {
        return get_inventory(a_player, RE::FormType::Weapon);
    }

    std::map<RE::TESBoundObject*, std::pair<int, std::unique_ptr<RE::InventoryEntryData>>>
        player::get_inventory_armor_items(RE::PlayerCharacter*& a_player) {
        return get_inventory(a_player, RE::FormType::Armor);
    }

    std::map<RE::TESBoundObject*, std::pair<int, std::unique_ptr<RE::InventoryEntryData>>>
        player::get_inventory(RE::PlayerCharacter*& a_player, RE::FormType a_type) {
        return a_player->GetInventory([a_type](const RE::TESBoundObject& a_object) { return a_object.Is(a_type); });
    }

    bool player::is_item_worn(RE::TESBoundObject*& a_obj, RE::PlayerCharacter*& a_player) {
        auto worn = false;
        for (const auto& [item, inv_data] : get_inventory_armor_items(a_player)) {
            if (const auto& [count, entry] = inv_data; entry->object->formID == a_obj->formID && entry->IsWorn()) {
                worn = true;
                break;
            }
        }
        return worn;
    }

    uint32_t player::get_inventory_count(const RE::TESForm* a_form) {
        uint32_t count = 0;
        if (!a_form) {
            return count;
        }
        auto player = RE::PlayerCharacter::GetSingleton();
        if (a_form->IsWeapon()) {
            auto weapons = get_inventory(player, RE::FormType::Weapon);
            for (const auto& [item, inv_data] : weapons) {
                if (const auto& [num_items, entry] = inv_data; entry->object->formID == a_form->formID) {
                    count = num_items;
                    break;
                }
            }
        } else if (a_form->IsArmor()) {
            auto armor = get_inventory(player, RE::FormType::Armor);
            for (const auto& [item, inv_data] : armor) {
                if (const auto& [num_items, entry] = inv_data; entry->object->formID == a_form->formID) {
                    count = num_items;
                    break;
                }
            }
        }
        logger::trace("got {} in inventory for item {}"sv, count, a_form->GetName());

        return count;
    }

    std::vector<data_helper*> player::get_hand_assignment(RE::TESForm*& a_form) {
        bool two_handed = false;
        if (a_form) {
            two_handed = util::helper::is_two_handed(a_form);
        }

        logger::trace("Item {} is two handed {}"sv, a_form ? a_form->GetName() : "null", two_handed);
        return get_hand_assignment(two_handed);
    }

    std::vector<data_helper*> player::get_hand_assignment(bool a_two_handed) {
        std::vector<data_helper*> data;
        const auto player = RE::PlayerCharacter::GetSingleton();
        auto right_obj = player->GetActorRuntimeData().currentProcess->GetEquippedRightHand();
        auto left_obj = player->GetActorRuntimeData().currentProcess->GetEquippedLeftHand();

        const auto empty_handle = config::mcm_setting::get_empty_hand_setting();

        const auto item = new data_helper();
        item->form = nullptr;
        item->left = false;
        item->type = handle::slot_setting::slot_type::empty;
        item->action_type = empty_handle ? handle::slot_setting::acton_type::un_equip :
                                           handle::slot_setting::acton_type::default_action;
        data.push_back(item);

        const auto item2 = new data_helper();
        item2->form = nullptr;
        item2->left = true;
        item2->type = handle::slot_setting::slot_type::empty;
        item2->action_type = empty_handle ? handle::slot_setting::acton_type::un_equip :
                                            handle::slot_setting::acton_type::default_action;
        data.push_back(item2);

        if (!a_two_handed) {
            a_two_handed = right_obj && util::helper::is_two_handed(right_obj);
        }

        logger::trace("got form {}, name {} on both/right hand"sv,
            right_obj ? string_util::int_to_hex(right_obj->GetFormID()) : "null",
            right_obj ? right_obj->GetName() : "null");

        logger::trace("got form {}, name {} on left hand"sv,
            left_obj ? string_util::int_to_hex(left_obj->GetFormID()) : "null",
            left_obj ? left_obj->GetName() : "null");

        if (a_two_handed && right_obj && left_obj && right_obj->formID == left_obj->formID) {
            data[0]->form = right_obj;
            data[0]->left = false;
            data[0]->type = util::helper::get_type(right_obj);
            data[0]->action_type = handle::slot_setting::acton_type::default_action;
            data.erase(data.begin() + 1);
        }

        if (right_obj) {
            data[0]->form = right_obj;
            data[0]->left = false;
            data[0]->type = util::helper::get_type(right_obj);
            data[0]->action_type = handle::slot_setting::acton_type::default_action;
        }

        if (left_obj) {
            data[1]->form = left_obj;
            data[1]->left = true;
            data[1]->type = util::helper::get_type(left_obj);
            data[1]->action_type = handle::slot_setting::acton_type::default_action;
        }

        logger::trace("got {} items in List now. return."sv, data.size());
        return data;
    }

    void player::look_for_ammo(const bool a_crossbow) {
        bool only_favorite = config::mcm_setting::get_only_favorite_ammo();
        bool sort_by_quantity = config::mcm_setting::get_sort_arrow_by_quantity();
        const auto max_items = config::mcm_setting::get_max_ammunition_type();
        auto player = RE::PlayerCharacter::GetSingleton();
        const auto inv = util::player::get_inventory(player, RE::FormType::Ammo);
        std::multimap<uint32_t, handle::ammo_data*, std::greater<>> ammo_list;
        for (const auto& [item, inv_data] : inv) {
            const auto& [num_items, entry] = inv_data;
            const auto ammo = item->As<RE::TESAmmo>();
            if (!ammo->GetPlayable() || ammo->GetRuntimeData().data.flags.any(RE::AMMO_DATA::Flag::kNonPlayable)) {
                continue;
            }

            if (only_favorite && !entry->IsFavorited()) {
                continue;
            }

            if (a_crossbow && ammo->GetRuntimeData().data.flags.none(RE::AMMO_DATA::Flag::kNonBolt) && num_items != 0) {
                logger::trace("found bolt {}, damage {}, count {}"sv,
                    ammo->GetName(),
                    ammo->GetRuntimeData().data.damage,
                    num_items);
                auto* ammo_data = new handle::ammo_data();
                ammo_data->form = ammo;
                ammo_data->item_count = num_items;
                if (sort_by_quantity) {
                    ammo_list.insert({ static_cast<uint32_t>(num_items), ammo_data });
                } else {
                    ammo_list.insert({ static_cast<uint32_t>(ammo->GetRuntimeData().data.damage), ammo_data });
                }
            } else if (!a_crossbow && num_items != 0 &&
                       ammo->GetRuntimeData().data.flags.all(RE::AMMO_DATA::Flag::kNonBolt)) {
                logger::trace("found arrow {}, damage {}, count {}"sv,
                    ammo->GetName(),
                    ammo->GetRuntimeData().data.damage,
                    num_items);
                auto* ammo_data = new handle::ammo_data();
                ammo_data->form = ammo;
                ammo_data->item_count = num_items;
                if (sort_by_quantity) {
                    ammo_list.insert({ static_cast<uint32_t>(num_items), ammo_data });
                } else {
                    ammo_list.insert({ static_cast<uint32_t>(ammo->GetRuntimeData().data.damage), ammo_data });
                }
            }
        }
        std::vector<handle::ammo_data*> sorted_ammo;
        const auto ammo_handle = handle::ammo_handle::get_singleton();
        for (auto [dmg, data] : ammo_list) {
            sorted_ammo.push_back(data);
            logger::trace("got {} count {}"sv, data->form->GetName(), data->item_count);
            if (sorted_ammo.size() == max_items) {
                break;
            }
        }
        ammo_list.clear();
        ammo_handle->init_ammo(sorted_ammo);
    }

}  // util
