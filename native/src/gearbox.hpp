// SPDX-License-Identifier: MIT
// Read and change a car's gears in the game, then have the game rebuild its gear table so the car drives on them.
#pragma once
#include "teams.hpp"

#include <swofty/mem.hpp>
#include <swofty/mixin.hpp>

#include <cstdint>
#include <cstring>

namespace gear_ratios {

constexpr uint32_t kMostCars = 22;
constexpr uintptr_t kCarNumber = 0xe8;
constexpr uintptr_t kCarGearbox = 0x538;
constexpr uintptr_t kPartOwner = 0x08;
constexpr uintptr_t kFirstGear = 0x58;
constexpr uintptr_t kGearStep = 0x10;
constexpr uintptr_t kSetupGears = 0x540;
constexpr uintptr_t kHiddenKey = 0x08;
// Rebuild the gear table and keep the car in its gear.
constexpr uint32_t kRebuildKeepGear = 0x2020;

using Gears = std::array<float, kGears>;

// A car's gearbox, found from the game's car table, or 0.
inline uintptr_t gearbox_of(uint32_t number)
{
    static uintptr_t table = swofty::api->resolve("g_Sess_VehicleTable");
    static uintptr_t count_at = swofty::api->resolve("g_Sess_VehicleCount");
    uint32_t count = 0;
    if (!table || !count_at || !swofty::mem::read(count_at, count) || number >= count || number >= kMostCars) {
        return 0;
    }
    uintptr_t car = 0;
    uint8_t car_number = 0;
    if (!swofty::mem::read(table + number * sizeof(uintptr_t), car) || !car) {
        return 0;
    }
    // A half-built car doesn't carry its number yet.
    if (!swofty::mem::read(car + kCarNumber, car_number) || car_number != number) {
        return 0;
    }
    uintptr_t gearbox = 0;
    uintptr_t owner = 0;
    if (!swofty::mem::read(car + kCarGearbox, gearbox) || !gearbox) {
        return 0;
    }
    if (!swofty::mem::read(gearbox + kPartOwner, owner) || owner != car) {
        return 0;
    }
    return gearbox;
}

// True when the car takes its gears from the player's setup instead (then we leave it alone).
inline bool uses_setup_gears(uintptr_t gearbox)
{
    uint8_t flag = 1;
    if (!swofty::mem::read(gearbox + kSetupGears, flag)) {
        return true;
    }
    return flag != 0;
}

// The game keeps each gear hidden: its bits mixed with a key stored next to it.
inline bool read_gear(uintptr_t gearbox, int gear, float &value)
{
    uintptr_t at = gearbox + kFirstGear + uintptr_t(gear) * kGearStep;
    uint32_t bits = 0;
    uint32_t key = 0;
    if (!swofty::mem::read(at, bits) || !swofty::mem::read(at + kHiddenKey, key)) {
        return false;
    }
    bits ^= key;
    std::memcpy(&value, &bits, sizeof value);
    return true;
}

inline bool write_gear(uintptr_t gearbox, int gear, float value)
{
    uintptr_t at = gearbox + kFirstGear + uintptr_t(gear) * kGearStep;
    uint32_t key = 0;
    if (!swofty::mem::read(at + kHiddenKey, key) || !swofty::mem::readable(at, sizeof(uint32_t))) {
        return false;
    }
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof bits);
    *reinterpret_cast<volatile uint32_t *>(at) = bits ^ key;
    return true;
}

inline bool read_gears(uintptr_t gearbox, Gears &gears)
{
    for (int gear = 0; gear < kGears; gear++) {
        if (!read_gear(gearbox, gear, gears[gear])) {
            return false;
        }
    }
    return true;
}

// Write all eight gears, then let the game rebuild the table it drives on.
inline bool write_gears(uintptr_t gearbox, const Gears &gears)
{
    static uintptr_t rebuild = swofty::api->resolve("Car_Tuning_SetFinalDrive");
    if (!rebuild) {
        return false;
    }
    for (int gear = 0; gear < kGears; gear++) {
        if (!write_gear(gearbox, gear, gears[gear])) {
            return false;
        }
    }
    using Rebuild = void (*)(void *gearbox, uint32_t flags);
    reinterpret_cast<Rebuild>(rebuild)(reinterpret_cast<void *>(gearbox), kRebuildKeepGear);
    return true;
}

inline bool same_gears(const Gears &left, const Gears &right)
{
    for (int gear = 0; gear < kGears; gear++) {
        float gap = left[gear] - right[gear];
        if (gap > 0.0001f || gap < -0.0001f) {
            return false;
        }
    }
    return true;
}

} // namespace gear_ratios
