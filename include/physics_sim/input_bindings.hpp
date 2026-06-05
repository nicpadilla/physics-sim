#pragma once

#include <SDL.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace physics_sim
{
struct InputBindings
{
    SDL_Keycode pause_resume = SDLK_SPACE;
    SDL_Keycode step_once = SDLK_s;
    SDL_Keycode reset = SDLK_F10;
    SDL_Keycode reset_fluid = SDLK_r;
    SDL_Keycode toggle_help = SDLK_h;
    SDL_Keycode cycle_visual_mode = SDLK_v;
    SDL_Keycode save_scene = SDLK_F5;
    SDL_Keycode load_scene = SDLK_F9;
    SDL_Keycode delete_selection = SDLK_DELETE;
    SDL_Keycode undo = SDLK_z;
    SDL_Keycode redo = SDLK_y;
    SDL_Keycode tool_prev = SDLK_LEFTBRACKET;
    SDL_Keycode tool_next = SDLK_RIGHTBRACKET;
    SDL_Keycode rotate_counterclockwise = SDLK_q;
    SDL_Keycode rotate_clockwise = SDLK_e;
    SDL_Keycode speed_down = SDLK_MINUS;
    SDL_Keycode speed_up = SDLK_EQUALS;
    SDL_Keycode emission_down = SDLK_COMMA;
    SDL_Keycode emission_up = SDLK_PERIOD;
    SDL_Keycode toggle_fullscreen = SDLK_F11;
    std::array<SDL_Keycode, 9> tool_hotkeys{
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_5,
        SDLK_6,
        SDLK_7,
        SDLK_8,
        SDLK_9,
    };
};

[[nodiscard]] inline constexpr InputBindings default_input_bindings() noexcept
{
    return {};
}

namespace detail
{
[[nodiscard]] inline std::string normalize_key_token(std::string_view token);
}

[[nodiscard]] inline std::string binding_display_name(std::string_view name)
{
    const std::string normalized = detail::normalize_key_token(name);
    if (normalized == "pauseresume")
    {
        return "Pause / Resume";
    }
    if (normalized == "steponce")
    {
        return "Step Once";
    }
    if (normalized == "reset")
    {
        return "Reset Scene";
    }
    if (normalized == "resetfluid")
    {
        return "Reset Fluid";
    }
    if (normalized == "togglehelp")
    {
        return "Toggle Help";
    }
    if (normalized == "cyclevisualmode")
    {
        return "Cycle Visual Mode";
    }
    if (normalized == "savescene")
    {
        return "Save Scene";
    }
    if (normalized == "loadscene")
    {
        return "Load Scene";
    }
    if (normalized == "deleteselection")
    {
        return "Delete Selection";
    }
    if (normalized == "undo")
    {
        return "Undo";
    }
    if (normalized == "redo")
    {
        return "Redo";
    }
    if (normalized == "toolprev")
    {
        return "Previous Tool";
    }
    if (normalized == "toolnext")
    {
        return "Next Tool";
    }
    if (normalized == "rotatecounterclockwise")
    {
        return "Rotate Counterclockwise";
    }
    if (normalized == "rotateclockwise")
    {
        return "Rotate Clockwise";
    }
    if (normalized == "speeddown")
    {
        return "Decrease Speed";
    }
    if (normalized == "speedup")
    {
        return "Increase Speed";
    }
    if (normalized == "emissiondown")
    {
        return "Decrease Emission";
    }
    if (normalized == "emissionup")
    {
        return "Increase Emission";
    }
    if (normalized == "togglefullscreen")
    {
        return "Toggle Fullscreen";
    }
    if (normalized.rfind("toolhotkey", 0) == 0)
    {
        const std::string suffix = normalized.substr(std::string("toolhotkey").size());
        return "Tool " + suffix;
    }

    return std::string{name};
}

[[nodiscard]] inline std::optional<SDL_Keycode> binding_value(const InputBindings& bindings, std::string_view name) noexcept
{
    const std::string normalized = detail::normalize_key_token(name);
    if (normalized == "pauseresume")
    {
        return bindings.pause_resume;
    }
    if (normalized == "steponce")
    {
        return bindings.step_once;
    }
    if (normalized == "reset")
    {
        return bindings.reset;
    }
    if (normalized == "resetfluid")
    {
        return bindings.reset_fluid;
    }
    if (normalized == "togglehelp")
    {
        return bindings.toggle_help;
    }
    if (normalized == "cyclevisualmode")
    {
        return bindings.cycle_visual_mode;
    }
    if (normalized == "savescene")
    {
        return bindings.save_scene;
    }
    if (normalized == "loadscene")
    {
        return bindings.load_scene;
    }
    if (normalized == "deleteselection")
    {
        return bindings.delete_selection;
    }
    if (normalized == "undo")
    {
        return bindings.undo;
    }
    if (normalized == "redo")
    {
        return bindings.redo;
    }
    if (normalized == "toolprev")
    {
        return bindings.tool_prev;
    }
    if (normalized == "toolnext")
    {
        return bindings.tool_next;
    }
    if (normalized == "rotatecounterclockwise")
    {
        return bindings.rotate_counterclockwise;
    }
    if (normalized == "rotateclockwise")
    {
        return bindings.rotate_clockwise;
    }
    if (normalized == "speeddown")
    {
        return bindings.speed_down;
    }
    if (normalized == "speedup")
    {
        return bindings.speed_up;
    }
    if (normalized == "emissiondown")
    {
        return bindings.emission_down;
    }
    if (normalized == "emissionup")
    {
        return bindings.emission_up;
    }
    if (normalized == "togglefullscreen")
    {
        return bindings.toggle_fullscreen;
    }
    if (normalized == "toolhotkey1")
    {
        return bindings.tool_hotkeys[0];
    }
    if (normalized == "toolhotkey2")
    {
        return bindings.tool_hotkeys[1];
    }
    if (normalized == "toolhotkey3")
    {
        return bindings.tool_hotkeys[2];
    }
    if (normalized == "toolhotkey4")
    {
        return bindings.tool_hotkeys[3];
    }
    if (normalized == "toolhotkey5")
    {
        return bindings.tool_hotkeys[4];
    }
    if (normalized == "toolhotkey6")
    {
        return bindings.tool_hotkeys[5];
    }
    if (normalized == "toolhotkey7")
    {
        return bindings.tool_hotkeys[6];
    }
    if (normalized == "toolhotkey8")
    {
        return bindings.tool_hotkeys[7];
    }
    if (normalized == "toolhotkey9")
    {
        return bindings.tool_hotkeys[8];
    }

    return std::nullopt;
}

[[nodiscard]] inline bool set_binding(InputBindings& bindings, std::string_view name, SDL_Keycode keycode) noexcept
{
    const std::string normalized = detail::normalize_key_token(name);
    if (normalized == "pauseresume")
    {
        bindings.pause_resume = keycode;
        return true;
    }
    if (normalized == "steponce")
    {
        bindings.step_once = keycode;
        return true;
    }
    if (normalized == "reset")
    {
        bindings.reset = keycode;
        return true;
    }
    if (normalized == "resetfluid")
    {
        bindings.reset_fluid = keycode;
        return true;
    }
    if (normalized == "togglehelp")
    {
        bindings.toggle_help = keycode;
        return true;
    }
    if (normalized == "cyclevisualmode")
    {
        bindings.cycle_visual_mode = keycode;
        return true;
    }
    if (normalized == "savescene")
    {
        bindings.save_scene = keycode;
        return true;
    }
    if (normalized == "loadscene")
    {
        bindings.load_scene = keycode;
        return true;
    }
    if (normalized == "deleteselection")
    {
        bindings.delete_selection = keycode;
        return true;
    }
    if (normalized == "undo")
    {
        bindings.undo = keycode;
        return true;
    }
    if (normalized == "redo")
    {
        bindings.redo = keycode;
        return true;
    }
    if (normalized == "toolprev")
    {
        bindings.tool_prev = keycode;
        return true;
    }
    if (normalized == "toolnext")
    {
        bindings.tool_next = keycode;
        return true;
    }
    if (normalized == "rotatecounterclockwise")
    {
        bindings.rotate_counterclockwise = keycode;
        return true;
    }
    if (normalized == "rotateclockwise")
    {
        bindings.rotate_clockwise = keycode;
        return true;
    }
    if (normalized == "speeddown")
    {
        bindings.speed_down = keycode;
        return true;
    }
    if (normalized == "speedup")
    {
        bindings.speed_up = keycode;
        return true;
    }
    if (normalized == "emissiondown")
    {
        bindings.emission_down = keycode;
        return true;
    }
    if (normalized == "emissionup")
    {
        bindings.emission_up = keycode;
        return true;
    }
    if (normalized == "togglefullscreen")
    {
        bindings.toggle_fullscreen = keycode;
        return true;
    }
    if (normalized == "toolhotkey1")
    {
        bindings.tool_hotkeys[0] = keycode;
        return true;
    }
    if (normalized == "toolhotkey2")
    {
        bindings.tool_hotkeys[1] = keycode;
        return true;
    }
    if (normalized == "toolhotkey3")
    {
        bindings.tool_hotkeys[2] = keycode;
        return true;
    }
    if (normalized == "toolhotkey4")
    {
        bindings.tool_hotkeys[3] = keycode;
        return true;
    }
    if (normalized == "toolhotkey5")
    {
        bindings.tool_hotkeys[4] = keycode;
        return true;
    }
    if (normalized == "toolhotkey6")
    {
        bindings.tool_hotkeys[5] = keycode;
        return true;
    }
    if (normalized == "toolhotkey7")
    {
        bindings.tool_hotkeys[6] = keycode;
        return true;
    }
    if (normalized == "toolhotkey8")
    {
        bindings.tool_hotkeys[7] = keycode;
        return true;
    }
    if (normalized == "toolhotkey9")
    {
        bindings.tool_hotkeys[8] = keycode;
        return true;
    }

    return false;
}

namespace detail
{
[[nodiscard]] inline std::string normalize_key_token(std::string_view token)
{
    std::string normalized;
    normalized.reserve(token.size());

    for (const unsigned char ch : token)
    {
        if (std::isalnum(ch))
        {
            normalized.push_back(static_cast<char>(std::tolower(ch)));
        }
    }

    return normalized;
}

[[nodiscard]] inline std::optional<SDL_Keycode> parse_function_key(std::string_view token) noexcept
{
    const std::string normalized = normalize_key_token(token);
    if (normalized.size() < 2 || normalized[0] != 'f')
    {
        return std::nullopt;
    }

    int function_index = 0;
    const auto [ptr, ec] = std::from_chars(normalized.data() + 1, normalized.data() + normalized.size(), function_index);
    if (ec != std::errc{} || ptr != normalized.data() + normalized.size() || function_index < 1 || function_index > 24)
    {
        return std::nullopt;
    }

    return static_cast<SDL_Keycode>(SDLK_F1 + (function_index - 1));
}

[[nodiscard]] inline std::optional<SDL_Keycode> parse_letter_or_digit(std::string_view token) noexcept
{
    const std::string normalized = normalize_key_token(token);
    if (normalized.size() != 1)
    {
        return std::nullopt;
    }

    const char ch = normalized.front();
    if (ch >= 'a' && ch <= 'z')
    {
        return static_cast<SDL_Keycode>(SDLK_a + (ch - 'a'));
    }

    if (ch >= '0' && ch <= '9')
    {
        return static_cast<SDL_Keycode>(SDLK_0 + (ch - '0'));
    }

    return std::nullopt;
}

[[nodiscard]] inline std::optional<SDL_Keycode> parse_named_key(std::string_view token) noexcept
{
    const std::string normalized = normalize_key_token(token);
    if (normalized.empty())
    {
        return std::nullopt;
    }

    if (const auto parsed_function = parse_function_key(normalized); parsed_function.has_value())
    {
        return parsed_function;
    }

    if (const auto parsed_scalar = parse_letter_or_digit(normalized); parsed_scalar.has_value())
    {
        return parsed_scalar;
    }

    if (normalized == "space")
    {
        return SDLK_SPACE;
    }
    if (normalized == "escape" || normalized == "esc")
    {
        return SDLK_ESCAPE;
    }
    if (normalized == "return" || normalized == "enter")
    {
        return SDLK_RETURN;
    }
    if (normalized == "tab")
    {
        return SDLK_TAB;
    }
    if (normalized == "backspace")
    {
        return SDLK_BACKSPACE;
    }
    if (normalized == "delete" || normalized == "del")
    {
        return SDLK_DELETE;
    }
    if (normalized == "pageup" || normalized == "pgup")
    {
        return SDLK_PAGEUP;
    }
    if (normalized == "pagedown" || normalized == "pgdn")
    {
        return SDLK_PAGEDOWN;
    }
    if (normalized == "home")
    {
        return SDLK_HOME;
    }
    if (normalized == "end")
    {
        return SDLK_END;
    }
    if (normalized == "insert" || normalized == "ins")
    {
        return SDLK_INSERT;
    }
    if (normalized == "up")
    {
        return SDLK_UP;
    }
    if (normalized == "down")
    {
        return SDLK_DOWN;
    }
    if (normalized == "left")
    {
        return SDLK_LEFT;
    }
    if (normalized == "right")
    {
        return SDLK_RIGHT;
    }
    if (normalized == "leftbracket" || normalized == "lbracket")
    {
        return SDLK_LEFTBRACKET;
    }
    if (normalized == "rightbracket" || normalized == "rbracket")
    {
        return SDLK_RIGHTBRACKET;
    }
    if (normalized == "minus" || normalized == "hyphen")
    {
        return SDLK_MINUS;
    }
    if (normalized == "equals" || normalized == "equal")
    {
        return SDLK_EQUALS;
    }
    if (normalized == "comma")
    {
        return SDLK_COMMA;
    }
    if (normalized == "period" || normalized == "dot")
    {
        return SDLK_PERIOD;
    }
    if (normalized == "slash" || normalized == "forwardslash")
    {
        return SDLK_SLASH;
    }
    if (normalized == "backslash")
    {
        return SDLK_BACKSLASH;
    }
    if (normalized == "semicolon")
    {
        return SDLK_SEMICOLON;
    }
    if (normalized == "quote" || normalized == "apostrophe")
    {
        return SDLK_QUOTE;
    }
    if (normalized == "lshift")
    {
        return SDLK_LSHIFT;
    }
    if (normalized == "rshift")
    {
        return SDLK_RSHIFT;
    }
    if (normalized == "lctrl" || normalized == "leftctrl" || normalized == "control")
    {
        return SDLK_LCTRL;
    }
    if (normalized == "rctrl" || normalized == "rightctrl")
    {
        return SDLK_RCTRL;
    }
    if (normalized == "lalt" || normalized == "leftalt")
    {
        return SDLK_LALT;
    }
    if (normalized == "ralt" || normalized == "rightalt")
    {
        return SDLK_RALT;
    }

    return std::nullopt;
}
} // namespace detail

[[nodiscard]] inline std::optional<SDL_Keycode> parse_keycode_token(std::string_view token) noexcept
{
    return detail::parse_named_key(token);
}

[[nodiscard]] inline std::string keycode_token(SDL_Keycode keycode)
{
    if (keycode >= SDLK_a && keycode <= SDLK_z)
    {
        const char letter = static_cast<char>('a' + (keycode - SDLK_a));
        return std::string{1, letter};
    }

    if (keycode >= SDLK_0 && keycode <= SDLK_9)
    {
        const char digit = static_cast<char>('0' + (keycode - SDLK_0));
        return std::string{1, digit};
    }

    if (keycode >= SDLK_F1 && keycode <= SDLK_F24)
    {
        return "f" + std::to_string(static_cast<int>(keycode - SDLK_F1 + 1));
    }

    switch (keycode)
    {
    case SDLK_SPACE:
        return "space";
    case SDLK_ESCAPE:
        return "escape";
    case SDLK_RETURN:
        return "return";
    case SDLK_TAB:
        return "tab";
    case SDLK_BACKSPACE:
        return "backspace";
    case SDLK_DELETE:
        return "delete";
    case SDLK_PAGEUP:
        return "pageup";
    case SDLK_PAGEDOWN:
        return "pagedown";
    case SDLK_HOME:
        return "home";
    case SDLK_END:
        return "end";
    case SDLK_INSERT:
        return "insert";
    case SDLK_UP:
        return "up";
    case SDLK_DOWN:
        return "down";
    case SDLK_LEFT:
        return "left";
    case SDLK_RIGHT:
        return "right";
    case SDLK_LEFTBRACKET:
        return "leftbracket";
    case SDLK_RIGHTBRACKET:
        return "rightbracket";
    case SDLK_MINUS:
        return "minus";
    case SDLK_EQUALS:
        return "equals";
    case SDLK_COMMA:
        return "comma";
    case SDLK_PERIOD:
        return "period";
    case SDLK_SLASH:
        return "slash";
    case SDLK_BACKSLASH:
        return "backslash";
    case SDLK_SEMICOLON:
        return "semicolon";
    case SDLK_QUOTE:
        return "apostrophe";
    case SDLK_LSHIFT:
        return "lshift";
    case SDLK_RSHIFT:
        return "rshift";
    case SDLK_LCTRL:
        return "lctrl";
    case SDLK_RCTRL:
        return "rctrl";
    case SDLK_LALT:
        return "lalt";
    case SDLK_RALT:
        return "ralt";
    default:
        return "unknown";
    }
}

[[nodiscard]] inline std::string keycode_label(SDL_Keycode keycode)
{
    const char* name = SDL_GetKeyName(keycode);
    if (name != nullptr && name[0] != '\0')
    {
        return std::string{name};
    }

    return "UNKNOWN";
}

[[nodiscard]] inline bool validate_input_bindings(const InputBindings& bindings) noexcept
{
    std::array<SDL_Keycode, 29> keys{
        bindings.pause_resume,
        bindings.step_once,
        bindings.reset,
        bindings.reset_fluid,
        bindings.toggle_help,
        bindings.cycle_visual_mode,
        bindings.save_scene,
        bindings.load_scene,
        bindings.delete_selection,
        bindings.undo,
        bindings.redo,
        bindings.tool_prev,
        bindings.tool_next,
        bindings.rotate_counterclockwise,
        bindings.rotate_clockwise,
        bindings.speed_down,
        bindings.speed_up,
        bindings.emission_down,
        bindings.emission_up,
        bindings.toggle_fullscreen,
        bindings.tool_hotkeys[0],
        bindings.tool_hotkeys[1],
        bindings.tool_hotkeys[2],
        bindings.tool_hotkeys[3],
        bindings.tool_hotkeys[4],
        bindings.tool_hotkeys[5],
        bindings.tool_hotkeys[6],
        bindings.tool_hotkeys[7],
        bindings.tool_hotkeys[8],
    };

    for (std::size_t i = 0; i < keys.size(); ++i)
    {
        if (keys[i] == SDLK_UNKNOWN)
        {
            return false;
        }

        for (std::size_t j = i + 1; j < keys.size(); ++j)
        {
            if (keys[i] == keys[j])
            {
                return false;
            }
        }
    }

    return true;
}
} // namespace physics_sim
