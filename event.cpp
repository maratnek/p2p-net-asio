#include "event.hpp"

// Event handler functions
// void handleUserEvent(const UserEvent& userEvent) {
// std::cout << "User Event: " << userEvent.getUsername() << " - " << userEvent.getAction() << std::endl;
// }

void handleSystemEvent(const events::Event &systemEventProto)
{

    if (systemEventProto.has_system_event())
    {
        const events::SystemEvent &systemEvent = systemEventProto.system_event();
        auto message = systemEvent.message();
        std::cout << "System Event: " << message << std::endl;
        // start handling system event
    }
    else
    {
        std::cerr << "Error parsing system event from data " << std::endl;
    }
}
