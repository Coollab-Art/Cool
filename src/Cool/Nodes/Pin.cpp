#include "Pin.h"

namespace Cool {

Pin::Pin(std::string name, std::string description)
    : _name{std::move(name)}
    , _description{std::move(description)}
    , _id{reg::generate_uuid()}
{
}

} // namespace Cool
