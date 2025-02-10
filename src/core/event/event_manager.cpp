#include <core/event/event_manager.h>

void EventManager::add_listener(EventId event_id, std::function<void(Event &)> const &listener) {
    std::lock_guard lock(m_mutex);
    m_listeners[event_id].push_back(listener);
}

void EventManager::send_event(Event &event) {
    std::lock_guard lock(m_mutex);
    uint32_t type = event.get_type();

    for (auto const &listener : m_listeners[type]) {
        listener(event);
    }
}

void EventManager::send_event(EventId event_id) {
    std::lock_guard lock(m_mutex);
    Event event(event_id);

    for (auto const &listener : m_listeners[event_id]) {
        listener(event);
    }
}