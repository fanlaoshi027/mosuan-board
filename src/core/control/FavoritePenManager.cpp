#include "FavoritePenManager.h"

#include "ToolHandler.h"
#include "settings/Settings.h"
#include "util/Assert.h"

std::array<FavoritePenPreset, FAVORITE_PEN_COUNT> FavoritePenManager::presets{};
bool FavoritePenManager::initialized = false;

void FavoritePenManager::initialize(Settings* settings) {
    if (initialized) {
        return;
    }

    // 初始五槽：常用数学板书颜色与粗细。
    presets[0] = {TOOL_PEN, TOOL_SIZE_MEDIUM, Colors::black};
    presets[1] = {TOOL_PEN, TOOL_SIZE_MEDIUM, Colors::red};
    presets[2] = {TOOL_PEN, TOOL_SIZE_MEDIUM, Colors::xopp_royalblue};
    presets[3] = {TOOL_PEN, TOOL_SIZE_VERY_FINE, Colors::black};
    presets[4] = {TOOL_HIGHLIGHTER, TOOL_SIZE_FINE, Colors::yellow};

    // 从墨写专属设置树恢复用户收藏。旧版本没有这些字段时保持上述默认值。
    if (settings != nullptr) {
        auto& favorites = settings->getCustomElement("mosuan.favoritePens");
        for (int slot = 0; slot < FAVORITE_PEN_COUNT; ++slot) {
            auto& preset = favorites.child(std::to_string(slot));
            int toolType = 0;
            int size = 0;
            int color = 0;
            if (preset.getInt("toolType", toolType) && preset.getInt("size", size) && preset.getInt("color", color)) {
                if ((toolType == TOOL_PEN || toolType == TOOL_HIGHLIGHTER) && size >= TOOL_SIZE_VERY_FINE &&
                    size < TOOL_SIZE_COUNT) {
                    presets[slot] = {static_cast<ToolType>(toolType), static_cast<ToolSize>(size), Color(color)};
                }
            }
        }
    }

    initialized = true;
}

void FavoritePenManager::apply(ToolHandler* toolHandler, Settings* settings, int slot) {
    if (toolHandler == nullptr || slot < 0 || slot >= FAVORITE_PEN_COUNT) {
        return;
    }

    initialize(settings);
    const auto& preset = presets[slot];

    toolHandler->selectTool(preset.toolType);
    toolHandler->setColor(preset.color, false);
    toolHandler->setSize(preset.size);
    toolHandler->fireToolChanged();
}

void FavoritePenManager::capture(ToolHandler* toolHandler, Settings* settings, int slot) {
    if (toolHandler == nullptr || slot < 0 || slot >= FAVORITE_PEN_COUNT) {
        return;
    }

    initialize(settings);

    ToolType type = toolHandler->getToolType();
    if (type != TOOL_PEN && type != TOOL_HIGHLIGHTER) {
        return;
    }

    presets[slot] = {type, toolHandler->getSize(), toolHandler->getColor()};

    if (settings != nullptr) {
        auto& preset = settings->getCustomElement("mosuan.favoritePens").child(std::to_string(slot));
        preset.setInt("toolType", static_cast<int>(presets[slot].toolType));
        preset.setInt("size", static_cast<int>(presets[slot].size));
        preset.setIntHex("color", static_cast<int>(static_cast<uint32_t>(presets[slot].color)));
        settings->customSettingsChanged();
    }
}

const FavoritePenPreset& FavoritePenManager::get(int slot) {
    initialize();
    xoj_assert(slot >= 0 && slot < FAVORITE_PEN_COUNT);
    return presets[slot];
}
