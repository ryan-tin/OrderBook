#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <functional> // std::less, std::greater
#include <mutex>

#include "Ladder.h"

using BidLadder = Ladder<std::greater<double>>; // highest price first
using AskLadder = Ladder<std::less<double>>;    // lowest price first

struct OrderBook {
    BidLadder bids;
    AskLadder asks;

    mutable std::mutex mutex_;

    using last_update_id_t = uint64_t;
    last_update_id_t last_update_id = 0;

    OrderBook() = default;

    // copy constructor
    OrderBook(const OrderBook& other)
        : bids(other.bids),
          asks(other.asks),
          last_update_id(other.last_update_id)
    {
        // Note: mutex_ is default-initialized, not copied
    }

    // copy assignment operator
    OrderBook& operator=(const OrderBook& other) {
        if (this != &other) {
            bids = other.bids;
            asks = other.asks;
            last_update_id = other.last_update_id;
            // mutex_ is not copied
        }
        return *this;
    }
    bool operator==(const OrderBook &other) const {
      return bids == other.bids && asks == other.asks &&
             last_update_id == other.last_update_id;
    }
    bool operator!=(const OrderBook &other) const {
      return !(*this == other);
    }
};

#include <iostream>
inline void CompareOrderBook(const OrderBook &ob1, const OrderBook &ob2) {
  if (ob1 == ob2) {
    std::cout << "OrderBooks are equal." << std::endl;
  } else {
    std::cout << "OrderBooks are not equal." << std::endl;

    std::cout << "Checking bids." << std::endl;
    for (const auto &[price, qty] : ob1.bids.levels_) {
      auto it = ob2.bids.levels_.find(price);
      if (it == ob2.bids.levels_.end()) {
        std::cout << "Price " << price
                  << " not found in ob2 bids.\n";
      } else if (it->second.total_quantity_ != qty.total_quantity_) {
        std::cout << "Price " << price
                  << " has different quantity in built order book bids: "
                  << qty.total_quantity_ << " vs "
                  << it->second.total_quantity_;
      }
    }
    std::cout << "Checking asks." << std::endl;
    for (const auto &[price, qty] : ob1.asks.levels_) {
      auto it = ob2.asks.levels_.find(price);
      if (it == ob2.asks.levels_.end()) {
        std::cout << "Price " << price
                  << " not found in ob2 asks.\n";
      } else if (it->second.total_quantity_ != qty.total_quantity_) {
        std::cout << "Price " << price
                  << " has different quantity in built order book asks: "
                  << qty.total_quantity_ << " vs "
                  << it->second.total_quantity_;
      }
    }
  }
}

#endif
