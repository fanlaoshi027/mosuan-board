/*
 * 墨写 favorite pen presets.
 * A preset is a small snapshot of the native Xournal++ tool state.
 */
#pragma once

#include <array>

#include "control/ToolEnums.h"
#include "util/Color.h"

class ToolHandler;

auto constexpr FAVORITE_PEN_COUNT = 5;

struct FavoritePenPreset {
    ToolType toolType = TOOL_PEN;
    ToolSize size = TOOL_SIZE_MEDIUM;
    Color color = Colors::black;
};

class FavoritePenManager {
public:
    static void apply(ToolHandler* toolHandler, int slot);
    static void capture(ToolHandler* toolHandler, int slot);
    static const FavoritePenPreset& get(int slot);

private:
    static void initialize();
    static std::array<FavoritePenPreset, FAVORITE_PEN_COUNT> presets;
    static bool initialized;
};
