#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <functional> // std::less, std::greater
#include <mutex>

#include "Binance/trade.h"
#include "Ladder.h"

using BidLadder = Ladder<std::greater<double>>; // highest price first
using AskLadder = Ladder<std::less<double>>;    // lowest price first

// NOTE: mutex locking is very slow?

struct OrderBook {
    BidLadder bids_;
    AskLadder asks_;

    // TODO: consider shared_mutex, which allows multiple readers
    mutable std::mutex mutex_;

    using last_update_id_t = uint64_t;
    last_update_id_t last_update_id = 0;

    using timestamp_t = uint64_t;
    timestamp_t t_ = 0;

    double bestMid() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return bestMidUnsafe();
    }
    // Best bid and best ask returns 0 if there are no bids or asks.
    double bestBid() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return bestBidUnsafe();
      // return bids_.empty() ? 0.0 : bids_.levels_.begin()->first;
    }
    double bestAsk() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return bestAskUnsafe();
      // return asks_.empty() ? 0.0 : asks_.levels_.begin()->first;
    }
    double bestMidUnsafe() const {
      if (bids_.empty() || asks_.empty())
        return 0.0;
      return (bestBidUnsafe() + bestAskUnsafe()) / 2.0;
    }
    double bestBidUnsafe() const {
      return bids_.empty() ? 0.0 : bids_.levels_.begin()->first;
    }
    double bestAskUnsafe() const {
      return asks_.empty() ? 0.0 : asks_.levels_.begin()->first;
    }
    double getBestQuote(TradeSide side) {
      switch (side) {
        case TradeSide::BUY:
          return bestBid();
        case TradeSide::SELL:
          return bestAsk();
        default:
          throw std::invalid_argument("Invalid trade side");
      }
    }
    double bestBidQty() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return bids_.empty() ? 0.0 : bids_.levels_.begin()->second.total_quantity_;
    }
    double bestAskQty() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return asks_.empty() ? 0.0 : asks_.levels_.begin()->second.total_quantity_;
    }
    double getBestQuoteQty(TradeSide side) {
      switch (side) {
        case TradeSide::BUY:
          return bestBidQty();
        case TradeSide::SELL:
          return bestAskQty();
        default:
          throw std::invalid_argument("Invalid trade side");
      }
    }
    double bidTotalQty(double price) const {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = bids_.levels_.find(price);
      return it != bids_.levels_.end() ? it->second.total_quantity_ : 0.0;
    }
    double askTotalQty(double price) const {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = asks_.levels_.find(price);
      return it != asks_.levels_.end() ? it->second.total_quantity_ : 0.0;
    }
    double getTotalQty(double price, TradeSide side) {
      switch (side) {
        case TradeSide::BUY:
          return bidTotalQty(price);
        case TradeSide::SELL:
          return askTotalQty(price);
        default:
          throw std::invalid_argument("Invalid trade side");
      }
    }

    OrderBook() = default;

    // copy constructor
    OrderBook(const OrderBook& other)
        : bids_(other.bids_),
          asks_(other.asks_),
          last_update_id(other.last_update_id)
    {
        // Note: mutex_ is default-initialized, not copied
    }

    // copy assignment operator
    OrderBook& operator=(const OrderBook& other) {
        if (this != &other) {
            bids_ = other.bids_;
            asks_ = other.asks_;
            last_update_id = other.last_update_id;
            // mutex_ is not copied
        }
        return *this;
    }
    bool operator==(const OrderBook &other) const {
      return bids_ == other.bids_ && asks_ == other.asks_ &&
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
    for (const auto &[price, qty] : ob1.bids_.levels_) {
      auto it = ob2.bids_.levels_.find(price);
      if (it == ob2.bids_.levels_.end()) {
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
    for (const auto &[price, qty] : ob1.asks_.levels_) {
      auto it = ob2.asks_.levels_.find(price);
      if (it == ob2.asks_.levels_.end()) {
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
