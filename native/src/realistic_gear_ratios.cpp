// SPDX-License-Identifier: MIT
// Realistic Gear Ratios: each 2026 team gets its own gearing, in every mode.
// Include mixin.hpp first: registry.hpp redefines SWOFTY_ADDON so the DLL exports SwoftyAddonRegister.
#include <swofty/mixin.hpp>
#include <swofty/mem.hpp>

#include <swofty/events.hpp>
#include <swofty/live.hpp>
#include <swofty/registry.hpp>

#include "field.hpp"

#include <chrono>
#include <optional>

using namespace swofty;
using namespace gear_ratios;

namespace {

using Clock = std::chrono::steady_clock;

Clock::time_point g_next_check{};

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
    // Big game heaps: trust their memory checks for 2 s, so a frame doesn't redo them.
    mem::Trust trust(2000);
    for (uint32_t number = 0; number < session->cars && number < kMostCars; number++) {
        update_car(number);
    }
}

} // namespace

SWOFTY_ON_LOAD
{
    events::on_frame(frame);
    log("[realistic-gear-ratios] loaded");
}

SWOFTY_ADDON()
