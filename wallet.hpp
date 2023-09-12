#ifndef _WALLET_HPP_
#define _WALLET_HPP_

#include <iostream>
#include "utils.hpp"

struct Wallet
{
    uint64_t m_id = 0;
    uint64_t m_balance = 0;

};

std::ostream &operator<<(std::ostream &ostr, const Wallet &w)
{
    ostr << "W.id: " << w.m_id << " balance: " << w.m_balance << std::endl;
    return ostr;
}

#endif // _WALLET_HPP_