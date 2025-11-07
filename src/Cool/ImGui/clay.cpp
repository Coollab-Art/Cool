#include "clay.hpp"
#include <imgui.h>
#include "clay/clay.h"

namespace Cool {

static auto clay_vec2(ImVec2 v) -> Clay_Vector2
{
    return {v.x, v.y};
}
static auto clay_dim(ImVec2 v) -> Clay_Dimensions
{
    return {v.x, v.y};
}
static auto clay_col(ImVec4 v) -> Clay_Color
{
    return {v.x, v.y, v.z, v.w};
}
static auto clay_padding(ImVec2 v) -> Clay_Padding
{
    return Clay_Padding{
        .left   = static_cast<uint16_t>(v.x),
        .right  = static_cast<uint16_t>(v.x),
        .top    = static_cast<uint16_t>(v.y),
        .bottom = static_cast<uint16_t>(v.y),
    };
}
static auto im_col(Clay_Color const& col) -> ImU32
{
    return IM_COL32((int)col.r, (int)col.g, (int)col.b, (int)col.a);
}

// TODO(Clay)
static void HandleClayErrors(Clay_ErrorData errorData)
{
    // See the Clay_ErrorData struct for more information
    printf("%s", errorData.errorText.chars);
    switch (errorData.errorType)
    {
        // etc
    }
}
// void HandleClayErrors(Clay_ErrorData errorData) {
//     printf("%s", errorData.errorText.chars);
//     if (errorData.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
//         reinitializeClay = true;
//         Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);
//     } else if (errorData.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
//         reinitializeClay = true;
//         Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
//     }
// }

static auto MeasureText(Clay_StringSlice text, Clay_TextElementConfig* /* config */, void* /* userData */)
    -> Clay_Dimensions
{
    // Clay_TextElementConfig contains members such as fontId, fontSize, letterSpacing etc
    // Note: Clay_String->chars is not guaranteed to be null terminated
    // TODO(Clay) Take fontId into account? currently we assume that we always use the default font
    return clay_dim(ImGui::CalcTextSize(text.chars, text.chars + text.length));
}

class ClayManager {
private:
    friend auto clay_manager() -> ClayManager&;

    ClayManager()
    {
        // TODO(Clay) share the arena and only instantiate clay once, and instead creat one ClayContext per window
        // Or maybe i don't even need several contexts ? I'm just scared that Clay_SetLayoutDimensions might conflict. But actually since be BeginLayout, EndLayout, end then apply the imgui draw commands immediately, we should then be able to start fresh (to clay it will look like a new frame has started. Maybe just there will be ID conflicts between the windows if they are in the same context ?)
        uint64_t const totalMemorySize = Clay_MinMemorySize();

        _arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));

        Clay_Initialize(_arena, Clay_Dimensions{0, 0}, Clay_ErrorHandler{HandleClayErrors});
        Clay_SetMeasureTextFunction(&MeasureText, nullptr);
    }

    ~ClayManager()
    {
        // free(_arena.memory); // TODO(Clay) I guess it doesn't matter to free it since it will only be destroyed at the end of the program
    }

    Clay_Arena _arena;
};

static auto clay_manager() -> ClayManager&
{
    static auto instance = ClayManager{};
    return instance;
}

// TODO(Clay)
// add a way to toggle Clay_SetDebugModeEnabled

auto const node_width   = 100;
auto const node_spacing = 30;

struct Category {
    std::string name;
};

struct Tab {
    std::string           name;
    std::vector<Category> category;
};

auto const tabs = std::vector<Tab>{
    {"2D", {}},
    {"3D", {}},
    {"Particles", {}},
    {"Advanced", {}},
};

size_t selectedTabIndex = 0;

void node()
{
    CLAY(
        CLAY_ID("Child"), // TODO(Clay) id for each node
        {
            .layout = {
                .sizing = {CLAY_SIZING_FIXED(node_width), CLAY_SIZING_FIXED(node_width)},
            },
            .backgroundColor = {255, 255, 255, 255},
        },
    ){};
}

void tab(size_t index, Clay_String name)
{
    CLAY(
        CLAY_SID(name),
        Clay_ElementDeclaration{
            .layout = {
                .sizing  = {CLAY_SIZING_GROW(), CLAY_SIZING_FIT()},
                .padding = clay_padding({5, 5}),
            },
            .backgroundColor = index == selectedTabIndex ? Clay_Color{200, 0, 0, 255} : Clay_Color{255, 0, 0, 255},
        }
    )
    {
        CLAY_TEXT(
            name,
            CLAY_TEXT_CONFIG(
                {
                    .textColor = {0, 0, 0, 255},
                },
            )
        );
    }
}

void imgui_window_test_clay()
{
    clay_manager(); // Make sure it's init

    auto const padding = ImGui::GetStyle().WindowPadding;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("Test");
    {
        Clay_SetLayoutDimensions(clay_dim(ImGui::GetContentRegionAvail()));
        Clay_SetPointerState(clay_vec2(ImGui::GetMousePos()), ImGui::IsMouseDown(ImGuiMouseButton_Left));
        // TODO(Clay) should pass pos relative to this window, not in screen space
        // TODO(Clay) Clay_SetPointerState(mousePosition, IsMouseButtonDown(0) && !scrollbarData.mouseDown);
        Clay_UpdateScrollContainers(false /*enableDragScrolling*/ /* TODO(Clay) try setting it to true? */, clay_vec2({ImGui::GetScrollX(), ImGui::GetScrollY()}), ImGui::GetIO().DeltaTime);
        // Maybe create the scrollabr at the imgui level ?
        Clay_BeginLayout();

        CLAY(
            CLAY_ID("Container"),
            {
                .layout = {
                    .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()},
                },
                .backgroundColor = {255, 0, 0, 255},
            },
        )
        {
            CLAY(
                CLAY_ID("Left Panel"),
                Clay_ElementDeclaration{
                    .layout = {
                        .sizing = {
                            .width  = CLAY_SIZING_FIT(),
                            .height = CLAY_SIZING_GROW(),
                        },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    },
                    .backgroundColor = {0, 255, 0, 255},
                },
            )
            {
                for (size_t i = 0; i < tabs.size(); ++i)
                {
                    tab(i, Clay_String{
                               .isStaticallyAllocated = false,
                               .length                = static_cast<int32_t>(tabs[i].name.size()),
                               .chars                 = tabs[i].name.c_str(),
                           });
                }
            }
            CLAY(
                CLAY_ID("Right Panel"),
                {
                    .layout = {
                        .sizing = {
                            .width  = CLAY_SIZING_GROW(),
                            .height = CLAY_SIZING_GROW(),
                        },
                        .padding  = clay_padding({20, 20}),
                        .childGap = 20,
                    },
                    .backgroundColor = {0, 0, 255, 255},
                },
            )
            {
                auto const N = 50;
                // auto const nb_cols =
                for (size_t i = 0; i < N; ++i)
                    node();
            }

            // CLAY_TEXT(
            //     CLAY_STRING("HELLO WORLD this is a very long text and i am sure it will overflow at some point which is funny no ? i think so but i don't like utf-8 so I don't like text"),
            //     CLAY_TEXT_CONFIG({
            //         .textColor = {255, 255, 255, 255},
            //         .fontSize  = 24,
            //     })
            // );
        }

        // All clay layouts are declared between Clay_BeginLayout and Clay_EndLayout
        Clay_RenderCommandArray renderCommands = Clay_EndLayout();

        // Just to see easier in debugger
        std::vector<Clay_RenderCommand> debug;
        for (int i = 0; i < renderCommands.length; i++)
        {
            debug.push_back(renderCommands.internalArray[i]);
        }

        auto const win_pos   = ImGui::GetCursorScreenPos();
        auto*      draw_list = ImGui::GetWindowDrawList();

        auto const imgui_pos = [&](float x, float y) {
            return ImVec2{x, y} + win_pos;
        };

        for (int i = 0; i < renderCommands.length; i++)
        {
            Clay_RenderCommand* rc = &renderCommands.internalArray[i];
            auto const&         bb = rc->boundingBox;

            switch (rc->commandType)
            {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
            {
                auto const col = im_col(rc->renderData.rectangle.backgroundColor);
                draw_list->AddRectFilled(imgui_pos(bb.x, bb.y), imgui_pos(bb.x + bb.width, bb.y + bb.height), col);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT:
            {
                auto const  col = im_col(rc->renderData.text.textColor);
                auto const& txt = rc->renderData.text.stringContents;
                draw_list->AddText(imgui_pos(bb.x, bb.y), col, txt.chars, txt.chars + txt.length);
                break;
            }
                // ... Implement handling of other command types
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(); // window padding
}

} // namespace Cool