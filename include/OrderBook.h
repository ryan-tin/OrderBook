#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <functional> // std::less, std::greater

#include "Ladder.h"

using BidLadder = Ladder<std::greater<double>>; // highest price first
using AskLadder = Ladder<std::less<double>>;    // lowest price first

struct OrderBook {
    BidLadder bids;
    AskLadder asks;
};

#endif

// example usage:
// OrderBook book;
// book.bids.add_order(101.5, 1, 10.0); // adds bid at 101.5
// book.asks.add_order(102.0, 2, 5.0);  // adds ask at 102.0
//
// auto best_bid = book.bids.best_level();
// if (best_bid) {
//     std::cout << "Best bid qty: " << best_bid->total_quantity << "\n";
// }
