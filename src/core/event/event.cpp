#include <core/event/event.h>

Event::Event(EventId type) : m_type(type) {}

[[nodiscard]] EventId Event::get_type() const { return m_type; }
