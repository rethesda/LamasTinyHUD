#pragma once
namespace util {
    constexpr int32_t const_log_trace = 0;
    constexpr int32_t const_log_debug = 1;
    constexpr int32_t const_log_info = 2;

    constexpr auto empty_enum_string = "$LamasTinyHUD_ENUM"sv;

    enum class selection_type { unset = -1, weapon = 0, magic = 1, shield = 2, shout = 3, power = 4, item = 5 };
}
