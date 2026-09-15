#include "FavoritePenManager.h"
#include "ToolHandler.h"
#include "control/settings/Settings.h"
#include "util/Assert.h"

namespace {
constexpr auto* FAVORITE_SETTINGS_NAME = "mosuan.favoritePens";
}

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

void FavoritePenManager::load(Settings* settings) {
    initialize();
    if (settings == nullptr) return;

    auto& root = settings->getCustomElement(FAVORITE_SETTINGS_NAME);
    for (int i = 0; i < FAVORITE_PEN_COUNT; ++i) {
        auto& slot = root.child("slot" + std::to_string(i));
        int toolType = static_cast<int>(presets[i].toolType);
        int size = static_cast<int>(presets[i].size);
        int color = static_cast<int>(static_cast<uint32_t>(presets[i].color));
        if (slot.getInt("toolType", toolType) && slot.getInt("size", size) && slot.getInt("color", color)) {
            if ((toolType == TOOL_PEN || toolType == TOOL_HIGHLIGHTER) && size >= TOOL_SIZE_VERY_FINE &&
                size <= TOOL_SIZE_VERY_LARGE) {
                presets[i].toolType = static_cast<ToolType>(toolType);
                presets[i].size = static_cast<ToolSize>(size);
                presets[i].color = Color{static_cast<uint32_t>(color)};
            }
        }
    }
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

void FavoritePenManager::capture(ToolHandler* toolHandler, Settings* settings, int slot) {
    if (toolHandler == nullptr || settings == nullptr || slot < 0 || slot >= FAVORITE_PEN_COUNT) return;
    initialize();
    ToolType type = toolHandler->getToolType();
    if (type != TOOL_PEN && type != TOOL_HIGHLIGHTER) return;

    presets[slot] = {type, toolHandler->getSize(), toolHandler->getColor()};

    auto& root = settings->getCustomElement(FAVORITE_SETTINGS_NAME);
    auto& saved = root.child("slot" + std::to_string(slot));
    saved.setInt("toolType", static_cast<int>(presets[slot].toolType));
    saved.setInt("size", static_cast<int>(presets[slot].size));
    saved.setIntHex("color", static_cast<int>(static_cast<uint32_t>(presets[slot].color)));
    settings->customSettingsChanged();
}

const FavoritePenPreset& FavoritePenManager::get(int slot) {
    initialize();
    xoj_assert(slot >= 0 && slot < FAVORITE_PEN_COUNT);
    return presets[slot];
}
