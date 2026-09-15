/**
 * Xournal++ action properties.
 */
#pragma once

#include <type_traits>

#include "control/AudioController.h"
#include "control/Control.h"
#include "control/ScrollHandler.h"
#include "control/ToolEnums.h"
#include "control/UndoRedoController.h"
#include "control/layer/LayerController.h"
#include "control/settings/Settings.h"
#include "control/zoom/ZoomControl.h"
#include "enums/Action.enum.h"
#include "gui/MainWindow.h"
#include "gui/SearchBar.h"
#include "gui/XournalView.h"
#include "gui/dialog/RenameLayerDialog.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "model/Document.h"
#include "model/Font.h"
#include "model/StrokeStyle.h"
#include "model/XojPage.h"
#include "plugin/PluginController.h"
#include "util/Assert.h"
#include "util/PopupWindowWrapper.h"
#include "util/Util.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"

#include "ActionDatabase.h"
#include "FavoritePenActions.h"

/**
 * Template class to store Action properties. Expected members are:
 *      * a static member function callback(GSimpleAction*, GVariant*, Control*);
 *      * optional state_type / parameter_type / accelerators / initiallyEnabled / app_namespace.
 */
template <Action action>
struct ActionProperties {};

/** @brief true if the Action has a parameter. */
template <Action a, class U = void>
struct has_param: std::false_type {};
template <Action a>
struct has_param<a, std::void_t<typename ActionProperties<a>::parameter_type>>: std::true_type {};

/** @brief true if the Action has a state. */
template <Action a, class U = void>
struct has_state: std::false_type {};
template <Action a>
struct has_state<a, std::void_t<typename ActionProperties<a>::state_type>>: std::true_type {};

/*** SPECIALIZATIONS ***/
