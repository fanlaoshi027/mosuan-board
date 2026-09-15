/*
 * Xournal++
 *
 * A selection for editing, every selection (Rect, Lasso...) is
 * converted to this one if the selection is finished
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>
#include <memory>  // for unique_ptr
#include <string>
#include <utility>  // for pair
#include <vector>   // for vector

#include <cairo.h>  // for cairo_t, cairo_matrix_t

#include "control/ToolEnums.h"               // for ToolSize
#include "model/Element.h"                   // for Element, Element::Index
#include "model/ElementContainer.h"          // for ElementContainer
#include "model/ElementInsertionPosition.h"  // for InsertionOrder
#include "model/PageRef.h"                   // for PageRef
#include "undo/UndoAction.h"                 // for UndoAction (ptr only)
#include "util/Color.h"                      // for Color
#include "util/PointerContainerView.h"       // for PointerContainerView
#include "util/Rectangle.h"                  // for Rectangle
#include "util/raii/GSourceURef.h"           // for GSourceURef
#include "util/serializing/Serializable.h"   // for Serializable

#include "CursorSelectionType.h"     // for CursorSelectionType, CURS...
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

class UndoRedoHandler;
class Layer;
class XojPageView;
class Selection;
class EditSelectionContents;
class DeleteUndoAction;
class LineStyle;
class ObjectInputStream;
class ObjectOutputStream;
class XojFont;
class Document;
class EditSelection;
class Stroke;

namespace SelectionFactory {
auto createFromFloatingElement(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view, ElementPtr e)
        -> std::unique_ptr<EditSelection>;
auto createFromFloatingElements(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view,
                                InsertionOrder elts)  //
        -> std::pair<std::unique_ptr<EditSelection>, Range>;
auto createFromElementOnActiveLayer(Control* ctrl, const PageRef& page, XojPageView* view, const Element* e,
                                    Element::Index pos = Element::InvalidIndex)  //
        -> std::unique_ptr<EditSelection>;
auto createFromElementsOnActiveLayer(Control* ctrl, const PageRef& page, XojPageView* view, InsertionOrderRef elts)
        -> std::unique_ptr<EditSelection>;
/**
 * @brief Creates a new instance containing base->getElements() and *e. The content of *base is cleared but *base is not
 * destroyed.
 */
auto addElementFromActiveLayer(Control* ctrl, EditSelection* base, const Element* e, Element::Index pos)
        -> std::unique_ptr<EditSelection>;
/**
 * @brief Creates a new instance containing base->getElements() and the content of elts. The content of *base is cleared
 * but *base is not destroyed.
 */
auto addElementsFromActiveLayer(Control* ctrl, EditSelection* base, const InsertionOrderRef& elts)
        -> std::unique_ptr<EditSelection>;
};  // namespace SelectionFactory

class EditSelection: public ElementContainer, public Serializable {
public:
    EditSelection(Control* ctrl, InsertionOrder elts, const PageRef& page, Layer* layer, XojPageView* view,
                  const Range& bounds, const Range& snappingBounds);

    /// Construct an empty selection
    EditSelection(Control* ctrl, const PageRef& page, Layer* layer, XojPageView* view);

    ~EditSelection() override;

public:
    double getXOnView() const;
    double getYOnView() const;
    double getOriginalXOnView();
    double getOriginalYOnView();
    double getWidth() const;
    double getHeight() const;
    xoj::util::Rectangle<double> getRect() const;
    xoj::util::Rectangle<double> getSnappedBounds() const;
    xoj::util::Rectangle<double> getOriginalBounds() const;
    double getRotation() const;
    bool isRotationSupported() const;
    PageRef getSourcePage() const;
    Layer* getSourceLayer() const;
    int getXOnViewAbsolute() const;
    int getYOnViewAbsolute() const;
    inline XojPageView* getView() const { return view; }

public:
    UndoActionPtr setSize(ToolSize size, const double* thicknessPen, const double* thicknessHighlighter,
                          const double* thicknessEraser);
    UndoActionPtr setLineStyle(LineStyle style);
    UndoActionPtr setColor(Color color);
    UndoActionPtr setFont(const XojFont& font);
    void fillUndoItem(DeleteUndoAction* undo);
    UndoActionPtr setFill(int alphaPen, int alphaHighligther);

public:
    void addElement(ElementPtr e, Element::Index pos);
    auto getElementsView() const -> xoj::util::PointerContainerView<std::vector<Element*>>;
    void forEachElement(std::function<void(const Element*)> f) const override;
    auto getInsertionOrder() const -> InsertionOrder const&;

    enum class OrderChange {
        BringToFront,
        BringForward,
        SendBackward,
        SendToBack,
    };

    static constexpr std::array<std::string_view, 4> orderChangeNames{"bringToFront", "bringForward", "sendBackward",
                                                                      "sendToBack"};
    static constexpr std::array<OrderChange, 4> allChanges = {OrderChange::BringToFront, OrderChange::BringForward,
                                                              OrderChange::SendBackward, OrderChange::SendToBack};

    static constexpr auto orderChangeToString(const OrderChange change) -> std::string_view {
        return orderChangeNames.at(static_cast<size_t>(change));
    }

    auto rearrangeInsertionOrder(const OrderChange change) -> UndoActionPtr;

    void mouseUp();
    void moveSelection(double dx, double dy, bool addMoveUndo = false);
    CursorSelectionType getSelectionTypeForPos(double x, double y, double zoom);
    void paint(cairo_t* cr, double zoom);
    auto getBoundingBoxInView() const -> xoj::util::Rectangle<double>;
    void ensureWithinVisibleArea();

public:
    void mouseDown(CursorSelectionType type, double x, double y);
    void mouseMove(double x, double y, bool alt);
    bool isMoving() const;
    bool isDeleting() const;
    void copySelection();

public:
    XojPageView* getView();

public:
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;
    InsertionOrder makeMoveEffective();

private:
    void drawAnchorRect(cairo_t* cr, double x, double y, double zoom);
    void drawAnchorRotation(cairo_t* cr, double x, double y, double zoom);
    void drawDeleteRect(cairo_t* cr, double x, double y, double zoom) const;
    void finalizeSelection();
    XojPageView* getPageViewUnderCursor();
    void translateToView(XojPageView* v);
    void updateMatrix();
    void scaleShift(double fx, double fy, bool changeLeft, bool changeTop);
    Point getRotationCenter() const;
    Point snapRotationCenter(double x, double y) const;
    void drawRotationCenter(cairo_t* cr, double zoom);
    void setEdgePan(bool edgePan);
    bool isEdgePanning() const;
    static bool handleEdgePan(EditSelection* self);

private:  // DATA
    double x{};
    double y{};
    double rotation = 0;
    cairo_matrix_t cmatrix{};
    double width{};
    double height{};
    xoj::util::Rectangle<double> snappedBounds{};

    CursorSelectionType mouseDownType = CURSOR_SELECTION_NONE;
    double relMousePosX{};
    double relMousePosY{};
    double relMousePosRotX{};
    double relMousePosRotY{};

    // 墨写: temporary custom rotation pivot. It resets when a selection is recreated.
    double rotationCenterX{};
    double rotationCenterY{};
    bool customRotationCenter = false;

    // 墨写: active vertex while editing a single geometry.
    Stroke* vertexStroke = nullptr;
    int vertexIndex = -1;
    std::vector<Point> vertexOriginalPoints;

    bool preserveAspectRatio = false;
    bool supportMirroring = true;
    bool supportRotation = true;
    int btnWidth{8};

    PageRef sourcePage;
    Layer* sourceLayer{};
    std::unique_ptr<EditSelectionContents> contents;

private:  // HANDLER
    XojPageView* view{};
    UndoRedoHandler* undo{};
    SnapToGridInputHandler snappingHandler;
    xoj::util::GSourceURef edgePanHandler;
    bool edgePanInhibitNext = false;
};
