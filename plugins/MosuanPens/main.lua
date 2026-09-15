-- 墨算收藏笔
-- 第一阶段：用 Xournal++ 原生 Lua API 实现可直接点击的预设笔。
-- 后续可继续扩展为用户可保存/覆盖的真正收藏槽位。

local pens = {
  {
    name = "墨算笔 1 · 黑",
    color = 0x202124,
    size = "MEDIUM",
    icon = "xopp-tool-pencil",
    id = "MOSUAN_PEN_1"
  },
  {
    name = "墨算笔 2 · 蓝",
    color = 0x2F6FED,
    size = "MEDIUM",
    icon = "xopp-tool-pencil",
    id = "MOSUAN_PEN_2"
  },
  {
    name = "墨算笔 3 · 红",
    color = 0xE5484D,
    size = "MEDIUM",
    icon = "xopp-tool-pencil",
    id = "MOSUAN_PEN_3"
  },
  {
    name = "墨算笔 4 · 绿",
    color = 0x2E9D63,
    size = "MEDIUM",
    icon = "xopp-tool-pencil",
    id = "MOSUAN_PEN_4"
  }
}

local function selectPen(pen)
  app.uiAction({ action = "ACTION_TOOL_PEN" })
  app.uiAction({ action = "ACTION_TOOL_LINE_STYLE_PLAIN" })
  app.uiAction({ action = "ACTION_SIZE_" .. pen.size })
  app.uiAction({ action = "ACTION_TOOL_FILL", enabled = false })
  app.changeToolColor({ color = pen.color, tool = "pen", selection = true })
end

function initUi()
  for _, pen in ipairs(pens) do
    local current = pen
    app.registerUi({
      menu = current.name,
      callback = function()
        selectPen(current)
      end,
      toolbarId = current.id,
      iconName = current.icon,
      description = current.name
    })
  end
end
