#pragma once

#include "control/FavoritePenManager.h"
#include "control/Control.h"
#include "enums/Action.enum.h"

/**
 * 墨写收藏笔槽：toolbar.ini 可用 FAVORITE_PEN(0..4) 直接调用。
 */
template <>
struct ActionProperties<Action::FAVORITE_PEN> {
    using parameter_type = int;

    static void callback(GSimpleAction*, GVariant* parameter, Control* ctrl) {
        const int slot = g_variant_get_int32(parameter);
        FavoritePenManager::apply(ctrl->getToolHandler(), slot);
    }
};
