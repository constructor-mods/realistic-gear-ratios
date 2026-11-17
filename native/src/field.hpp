// SPDX-License-Identifier: MIT
// Every car on the grid: which team it is, the game's own gears, and ours. Puts our gears on, and puts them on again
// when the game rebuilds a car.
#pragma once
#include "gearbox.hpp"
#include "teams.hpp"

#include <swofty/live.hpp>
#include <swofty/mixin.hpp>

#include <array>
#include <optional>
#include <string>

namespace gear_ratios {

using swofty::log;

struct CarState {
    uintptr_t gearbox = 0;
    const Team *team = nullptr;
    bool uses_setup = false;
    bool changed = false;
    Gears game{};
    Gears ours{};
};

inline std::array<CarState, kMostCars> g_cars;

inline std::string gears_text(const Gears &gears)
{
    std::string text;
    char number[16];
    for (int gear = 0; gear < kGears; gear++) {
        std::snprintf(number, sizeof number, gear == 0 ? "%.3f" : " %.3f", gears[gear]);
        text += number;
    }
    return text;
}

// A car seen for the first time (or rebuilt into a new gearbox): find its team. False while the game can't say yet.
inline bool meet_car(uint32_t number, uintptr_t gearbox, CarState &state)
{
    std::optional<std::string> key = swofty::live::team_badge(number);
    if (!key) {
        return false;
    }
    state = CarState{};
    state.gearbox = gearbox;
    state.team = team_for_key(*key);
    if (!state.team) {
        log("[realistic-gear-ratios] car %u (%s): left alone", number, key->c_str());
        return true;
    }
    state.uses_setup = uses_setup_gears(gearbox);
    if (state.uses_setup) {
        log("[realistic-gear-ratios] car %u (%s): takes its gears from the setup, left alone", number, key->c_str());
        return true;
    }
    log("[realistic-gear-ratios] car %u (%s): %s", number, key->c_str(), state.team->name);
    return true;
}

inline void put_ours_on(uint32_t number, CarState &state, const Gears &now)
{
    state.game = now;
    for (int gear = 0; gear < kGears; gear++) {
        state.ours[gear] = state.game[gear] * state.team->change[gear];
    }
    if (!write_gears(state.gearbox, state.ours)) {
        log("[realistic-gear-ratios] car %u: couldn't write the gears", number);
        return;
    }
    state.changed = true;
    log("[realistic-gear-ratios] car %u %s: %s (game %s)", number, state.team->name, gears_text(state.ours).c_str(),
        gears_text(state.game).c_str());
}

inline void update_car(uint32_t number)
{
    CarState &state = g_cars[number];
    uintptr_t gearbox = gearbox_of(number);
    if (!gearbox) {
        return;
    }
    if (gearbox != state.gearbox && !meet_car(number, gearbox, state)) {
        return;
    }
    if (!state.team || state.uses_setup) {
        return;
    }
    Gears now{};
    if (!read_gears(gearbox, now)) {
        return;
    }
    if (state.changed && same_gears(now, state.ours)) {
        return;
    }
    if (state.changed) {
        log("[realistic-gear-ratios] car %u: the game put its own gears back, changing them again", number);
    }
    put_ours_on(number, state, now);
}

} // namespace gear_ratios
