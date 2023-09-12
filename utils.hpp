#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <random>
#include <iostream>
#include <string>
#include "wallet.hpp"

#include "proto/events.pb.h"

using namespace std;

static std::random_device rd;   // non-deterministic generator
static std::mt19937 gen(rd());  // to seed mersenne twister.

namespace utils {

////////////////////////////////////////////////////////////////
/// random for tests
inline std::string gen_random_str(const int len)
{
    std::string tmp_s;
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
    uniform_int_distribution<> dist(1,sizeof(alphanum)); // distribute results between 1 and 6 inclusive.

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[dist(gen)];

    return std::move(tmp_s);
}

inline uint64_t gen_num()
{
    return gen();
}

// Function to generate a random UUID (in string format for simplicity)
inline std::string generateRandomUUID() {
    // Replace this with your UUID generation logic
    // For simplicity, generate a random string
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    const char hex_chars[] = "0123456789ABCDEF";
    std::string uuid;
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            uuid += '-';
        } else {
            uuid += hex_chars[dis(gen)];
        }
    }
    return uuid;
}

Wallet genRandWallet()
{
    auto id = gen_num();
    auto bal = gen_num() % 1000;
    return Wallet{id, bal};
}

std::string createWalletMessage() {
    // Create an Event message
    events::Event event;

    // Set the event type to USER_EVENT
    event.set_type(events::USER_EVENT);

    // Create a UserEvent message
    events::UserEvent userEvent;

    // Generate a random UUID
    userEvent.set_uuid(generateRandomUUID());

    // Choose a random event type (CREATE_WALLET or TRANSACTION)
    // events::UserType userType = (rand() % 2 == 0) ? events::CREATE_WALLET : events::TRANSACTIOIN;
    events::UserType userType = events::CREATE_WALLET;
    userEvent.set_type(userType);

    // Depending on the event type, generate the corresponding event message
    if (userType == events::CREATE_WALLET) {
        events::CreateWallet createWallet;
        Wallet wallet = genRandWallet();
        createWallet.set_id(wallet.m_id);
        createWallet.set_amount(wallet.m_balance);
        // Set the user_event field to CreateWallet
        userEvent.set_allocated_create_wallet(&createWallet);
    } 
    // else {
    //     events::Transaction transaction;
    //     transaction.set_id1(rand());
    //     transaction.set_id2(rand());
    //     transaction.set_amount(rand());
    //     // Set the user_event field to Transaction
    //     userEvent.set_allocated_transaction(&transaction);
    // }

    // Set the event field to UserEvent
    event.set_allocated_user_event(&userEvent);

    // Serialize the Event message to send it over the network
    std::string serializedEvent;
    event.SerializeToString(&serializedEvent);
    return serializedEvent;
}

// Function to parse a received wallet event message
Wallet parseWalletEvent(const std::string& serializedEvent) {
    events::Event event;

    // Parse the received serializedEvent
    if (event.ParseFromString(serializedEvent)) {
        // Check the event type
        if (event.type() == events::USER_EVENT) {
            const events::UserEvent& userEvent = event.user_event();
            
            // Access userEvent fields
            std::string uuid = userEvent.uuid();
            events::UserType userType = userEvent.type();
            
            if (userType == events::CREATE_WALLET) {
                const events::CreateWallet& createWallet = userEvent.create_wallet();
                uint32_t id = createWallet.id();
                uint32_t amount = createWallet.amount();
                
                // Handle the CreateWallet event
                std::cout << "Received CreateWallet Event: UUID=" << uuid
                          << " ID=" << id << " Amount=" << amount << std::endl;

                return Wallet(id, amount);

            } else if (userType == events::TRANSACTIOIN) {
                const events::Transaction& transaction = userEvent.transaction();
                uint32_t id1 = transaction.id1();
                uint32_t id2 = transaction.id2();
                uint32_t amount = transaction.amount();
                
                // Handle the Transaction event
                std::cout << "Received Transaction Event: UUID=" << uuid
                          << " ID1=" << id1 << " ID2=" << id2 << " Amount=" << amount << std::endl;
            }
        }
    } else {
        std::cerr << "Failed to parse the received wallet event message." << std::endl;
    }
    return {};
}

} // namespace

#endif //UTILS 