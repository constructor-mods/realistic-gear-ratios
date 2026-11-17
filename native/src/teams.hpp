// SPDX-License-Identifier: MIT
// Each 2026 team's gearing, as a change to the game's own gears. The game gives every 2026 car the same eight gears;
// these numbers make a team's gears shorter (above 1) or longer (below 1), one number per gear, 1st to 8th.
// Teams that share a gearbox in real life share a row.
#pragma once
#include <array>
#include <string>

namespace gear_ratios {

constexpr int kGears = 8;
using GearChange = std::array<float, kGears>;

// The gears the game gives every 2026 car, 1st to 8th. Our gears always start from these, never from what's on the
// car, so a car that still carries an earlier car's gears can't get changed twice.
constexpr std::array<float, kGears> kGame2026Gears = {3.480f, 2.620f, 2.100f, 1.740f, 1.475f, 1.275f, 1.120f, 1.000f};

struct Team {
    const char *name;
    // Words the game's team key can contain for this team (the key looks like "ferrari_26").
    std::array<const char *, 6> words;
    GearChange change;
};

// McLaren: short everywhere, the shortest top gears on the grid.
constexpr GearChange kMcLaren = {1.030f, 1.030f, 1.030f, 1.034f, 1.038f, 1.042f, 1.042f, 1.042f};
// Red Bull Powertrains: a long 1st, a short 3rd, close gears, short-ish top.
constexpr GearChange kRedBull = {0.930f, 0.970f, 1.000f, 1.000f, 1.013f, 1.027f, 1.027f, 1.027f};
// Ferrari: the middle of the grid, the game's own gears.
constexpr GearChange kFerrari = {1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f, 1.000f};
// Aston Martin: a little shorter low down, the middle at the top.
constexpr GearChange kAstonMartin = {1.015f, 1.015f, 1.015f, 1.010f, 1.005f, 1.000f, 1.000f, 1.000f};
// Mercedes: long low gears and long top gears.
constexpr GearChange kMercedes = {0.970f, 0.970f, 0.970f, 0.975f, 0.979f, 0.984f, 0.984f, 0.984f};
// Audi: the widest spread, a short 1st and the longest 8th.
constexpr GearChange kAudi = {1.030f, 1.030f, 1.030f, 1.000f, 0.970f, 0.940f, 0.940f, 0.940f};

// Racing Bulls comes before Red Bull, so its key never matches Red Bull's words.
inline const std::array<Team, 11> kTeams = {{
    {"McLaren", {"mclaren"}, kMcLaren},
    {"Racing Bulls", {"racing_bull", "racingbull", "toro_rosso", "alphatauri", "vcarb", "visa"}, kRedBull},
    {"Red Bull", {"red_bull", "redbull"}, kRedBull},
    {"Ferrari", {"ferrari"}, kFerrari},
    {"Haas", {"haas"}, kFerrari},
    {"Cadillac", {"cadillac"}, kFerrari},
    {"Aston Martin", {"aston", "force_india"}, kAstonMartin},
    {"Mercedes", {"mercedes"}, kMercedes},
    {"Williams", {"williams"}, kMercedes},
    {"Alpine", {"alpine", "lotus"}, kMercedes},
    {"Audi", {"audi", "sauber", "kick", "alfa_romeo"}, kAudi},
}};

// True for a 2026 car: the game's key ends in "_26".
inline bool is_2026(const std::string &key)
{
    const std::string season = "_26";
    if (key.size() < season.size()) {
        return false;
    }
    return key.compare(key.size() - season.size(), season.size(), season) == 0;
}

// The team for a key like "mclaren_26", or nothing for another season or a team not in the list.
inline const Team *team_for_key(const std::string &key)
{
    if (!is_2026(key)) {
        return nullptr;
    }
    for (const Team &team : kTeams) {
        for (const char *word : team.words) {
            if (word && key.find(word) != std::string::npos) {
                return &team;
            }
        }
    }
    return nullptr;
}

} // namespace gear_ratios
