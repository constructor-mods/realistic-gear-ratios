// SPDX-License-Identifier: MIT
// Realistic Gear Ratios: each 2026 team gets its own gearing in Grand Prix and Time Trial.
// Include mixin.hpp first: registry.hpp redefines SWOFTY_ADDON so the DLL exports SwoftyAddonRegister.
#include <swofty/mixin.hpp>
#include <swofty/mem.hpp>

#include <swofty/events.hpp>
#include <swofty/live.hpp>
#include <swofty/registry.hpp>

#include "field.hpp"

#include <atomic>
#include <chrono>
#include <optional>

using namespace swofty;
using namespace gear_ratios;

namespace {

using Clock = std::chrono::steady_clock;

SWOFTY_SETTING(bool, team_gear_ratios, true);

Clock::time_point g_next_check{};
live::GameMode g_last_mode = live::GameMode::unknown;

// Only Grand Prix and Time Trial get the fixed set. Career and My Team are never touched.
bool fixed_set_mode(live::GameMode mode)
{
    return mode == live::GameMode::grand_prix || mode == live::GameMode::time_trial;
}

const char *mode_name(live::GameMode mode)
{
    if (mode == live::GameMode::grand_prix) {
        return "Grand Prix";
    }
    if (mode == live::GameMode::time_trial) {
        return "Time Trial";
    }
    return "another mode";
}

void frame(double, void *)
{
    auto now = Clock::now();
    if (now < g_next_check) {
        return;
    }
    g_next_check = now + std::chrono::milliseconds(250);
    std::optional<live::Session> session = live::session();
    if (!session) {
        return;
    }
    live::GameMode mode = live::game_mode(*session);
    if (mode != g_last_mode) {
        g_last_mode = mode;
        log("[realistic-gear-ratios] %s", mode_name(mode));
    }
    if (!fixed_set_mode(mode)) {
        return;
    }
    // Big game heaps: trust their memory checks for 2 s, so a frame doesn't redo them.
    mem::Trust trust(2000);
    bool on = team_gear_ratios.get();
    for (uint32_t number = 0; number < session->cars && number < kMostCars; number++) {
        update_car(number, on);
    }
}

} // namespace

SWOFTY_ON_CHANGE(team_gear_ratios, bool value)
{
    log("[realistic-gear-ratios] 2026 team gear ratios %s", value ? "on" : "off");
}

SWOFTY_ON_LOAD
{
    events::on_frame(frame);
    log("[realistic-gear-ratios] loaded");
}

SWOFTY_ADDON()
