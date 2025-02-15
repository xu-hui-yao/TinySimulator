#pragma once

#include <any>
#include <core/common.h>
#include <unordered_map>

// Input
enum class InputButtons { W, A, S, D, Q, E, R, SHIFT, CTRL, SPACE };
enum class InputMouses { LEFT, RIGHT };

// Events
using EventId = std::uint32_t;
using ParamId = std::uint32_t;

namespace Events::Window {
constexpr EventId QUIT    = "Events::Window::QUIT"_hash;
constexpr EventId RESIZED = "Events::Window::RESIZED"_hash;
constexpr EventId INPUT   = "Events::Window::INPUT"_hash;
} // namespace Events::Window

namespace Events::Window::Input {
constexpr ParamId KEYBOARD_INPUT = "Events::Window::Input::KEYBOARD_INPUT"_hash;
constexpr ParamId MOUSE_INPUT    = "Events::Window::Input::MOUSE_INPUT"_hash;
constexpr ParamId MOUSE_POSITION = "Events::Window::Input::MOUSE_POSITION"_hash;
} // namespace Events::Window::Input

namespace Events::Window::Resized {
constexpr ParamId WIDTH  = "Events::Window::Resized::WIDTH"_hash;
constexpr ParamId HEIGHT = "Events::Window::Resized::HEIGHT"_hash;
} // namespace Events::Window::Resized

class Event {
public:
    Event() = delete;

    explicit Event(EventId type);

    template <typename T> void set_param(EventId id, T value) { m_data[id] = value; }

    template <typename T> T get_param(EventId id) { return std::any_cast<T>(m_data[id]); }

    [[nodiscard]] EventId get_type() const;

private:
    EventId m_type{};
    std::unordered_map<EventId, std::any> m_data{};
};