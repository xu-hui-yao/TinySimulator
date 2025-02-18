#pragma once

#include <core/event/event.h>
#include <functional>
#include <list>
#include <unordered_map>
#include <mutex>

class EventManager {
public:
    void add_listener(EventId event_id, std::function<void(Event &)> const &listener);

    void send_event(Event &event);

    void send_event(EventId event_id);

private:
    std::unordered_map<EventId, std::list<std::function<void(Event &)>>> m_listeners;
    std::mutex m_mutex;
};

std::shared_ptr<EventManager> get_event_manager();
