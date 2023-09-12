#ifndef __EVENT_HPP__
#define __EVENT_HPP__

#include "proto/events.pb.h" 

/// @brief Base event class for events
class BaseEvent
{
public:
    virtual events::Event serializeToProtobuf() const = 0; 
    virtual bool parseFromProtobuf(std::string) = 0;
    BaseEvent(events::EventType _type) : mEventType(_type) {}
    virtual ~BaseEvent() {}

protected:
    events::EventType mEventType;
};


// System Event class
class SystemEvent : public BaseEvent {
public:
    SystemEvent(const std::string& message)
        : message_(message), BaseEvent(events::EventType::SYSTEM_EVENT) {}

    events::Event serializeToProtobuf() const override {
        events::Event systemEventProto;
        systemEventProto.set_type(mEventType);
        systemEventProto.mutable_system_event()->set_message(message_);
        return systemEventProto;
    }

    bool parseFromProtobuf(std::string receiveData) override {
        events::Event systemEventProto;
        if (systemEventProto.ParseFromString(receiveData))
        {
            if (systemEventProto.has_system_event())
            {
                const events::SystemEvent &systemEvent = systemEventProto.system_event();
                message_ = systemEvent.message();
                return true;
            }
            else
            {
                std::cerr << "Error parsing system event from data " << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to parse received event " << std::endl;
        }
        return false;
    }

    std::string getMessage() const {
        return message_;
    }

private:
    std::string message_;
};
// // User Event class
// class UserEvent : public BaseEvent {
// public:
//     UserEvent(const std::string& username, const std::string& action)
//         : username_(username), action_(action), BaseEvent(events::EventType::USER_EVENT) {}

//     void serializeToProtobuf(events::BaseEvent* proto) const override {
//         proto->set_type(this->getType());
//         proto->mutable_user_event()->set_username(username_);
//         proto->mutable_user_event()->set_action(action_);
//     }

//     bool parseFromProtobuf(const events::BaseEvent& proto) override {
//         if (proto.type() == events::EventType::USER_EVENT) {
//             username_ = proto.user_event().username();
//             action_ = proto.user_event().action();
//             return true;
//         }
//         return false;
//     }

//     std::string getUsername() const {
//         return username_;
//     }

//     std::string getAction() const {
//         return action_;
//     }

// private:
//     std::string username_;
//     std::string action_;
// };

// Forward declarations of event handler functions
// void handleUserEvent(const UserEvent& userEvent);
void handleSystemEvent(const events::Event &systemEventProto);



class EventHandler {
public:
    void handleEvent(const std::string& message) {
        events::Event event;
        if (event.ParseFromString(message)) {
            auto it = m_eventHandlers.find(event.type());
            if (m_eventHandlers.contains(event.type())) {
                // Call the appropriate handler function
                it->second(event);
            } else {
                std::cerr << "Unknown event type." << std::endl;
            }
        } else {
            std::cerr << "Failed to parse received event " << std::endl;
        }
    }

    // Simulated network event sending
    void sendEventToNetwork(std::string message) {
        // Serialize and send the event over the network (simulated)
        // std::cout << "Sending event over the network: " << serializedEvent << std::endl;
    }

    // Define a mapping from event type to handler function
    std::unordered_map<events::EventType, void (*)(const events::Event &)> m_eventHandlers = {
        // {events::EventType::USER_EVENT, handleUserEvent},
        {events::EventType::SYSTEM_EVENT, handleSystemEvent}
        // Add more event types and handlers as needed
    };
};

#endif // __EVENT_HPP__