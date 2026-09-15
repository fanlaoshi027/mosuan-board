#include "FavoritePenManager.h"
#include "ToolHandler.h"
#include "util/Assert.h"

std::array<FavoritePenPreset, FAVORITE_PEN_COUNT> FavoritePenManager::presets{};
bool FavoritePenManager::initialized = false;

void FavoritePenManager::initialize() {
    if (initialized) return;
    presets[0] = {TOOL_PEN, TOOL_SIZE_MEDIUM, Colors::black};
    presets[1] = {TOOL_PEN, TOOL_SIZE_MEDIUM, Colors::red};
    presets[2] = {TOOL_PEN, TOOL_SIZE_MEDIUM, Colors::xopp_royalblue};
    presets[3] = {TOOL_PEN, TOOL_SIZE_VERY_FINE, Colors::black};
    presets[4] = {TOOL_HIGHLIGHTER, TOOL_SIZE_FINE, Colors::yellow};
    initialized = true;
}

void FavoritePenManager::apply(ToolHandler* toolHandler, int slot) {
    if (toolHandler == nullptr || slot < 0 || slot >= FAVORITE_PEN_COUNT) return;
    initialize();
    const auto& preset = presets[slot];
    toolHandler->selectTool(preset.toolType);
    toolHandler->setColor(preset.color, false);
    toolHandler->setSize(preset.size);
    toolHandler->fireToolChanged();
}

void FavoritePenManager::capture(ToolHandler* toolHandler, int slot) {
    if (toolHandler == nullptr || slot < 0 || slot >= FAVORITE_PEN_COUNT) return;
    initialize();
    ToolType type = toolHandler->getToolType();
    if (type != TOOL_PEN && type != TOOL_HIGHLIGHTER) return;
    presets[slot] = {type, toolHandler->getSize(), toolHandler->getColor()};
}

const FavoritePenPreset& FavoritePenManager::get(int slot) {
    initialize();
    xoj_assert(slot >= 0 && slot < FAVORITE_PEN_COUNT);
    return presets[slot];
}
