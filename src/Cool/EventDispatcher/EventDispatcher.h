#pragma once
#include <unordered_map>
#include "reg/reg.hpp"
#include "reg/src/generate_uuid.hpp"

namespace Cool {

/// A class that dispatches events to everyone that subscribed a callback to it
/// Event: Any struct that will be passed to the subscribers
template<typename Event>
class EventDispatcher {
public:
    void dispatch(Event const& event)
    {
        for (auto const& [_, callback] : _callbacks)
            callback(event);
    }

    /// Returns an ID that can later be used to unsubscribe the callback
    auto subscribe(std::function<void(Event const&)> callback) -> reg::AnyId
    {
        auto const id  = reg::generate_uuid();
        _callbacks[id] = std::move(callback);
        return id;
    }

    void unsubscribe(reg::AnyId const& id)
    {
        assert(_callbacks.find(id) != _callbacks.end());

        _callbacks.erase(id);
    }

private:
    std::unordered_map<reg::AnyId, std::function<void(Event const&)>> _callbacks{};
};

} // namespace Cool
