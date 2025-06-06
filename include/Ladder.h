#ifndef LADDER_H
#define LADDER_H

#include <list>
#include <unordered_map>
#include <map>

template <typename Compare>
class Ladder {
public:
  using Price = double;
  using OrderId = int;

  struct Order {
      OrderId id;
      double quantity;
  };

  struct Level {
    using OrderList = std::list<Order>;

    OrderList orders_;                                                     // FIFO queue
    std::unordered_map<OrderId, typename OrderList::iterator> order_map_;  // Fast lookup
    double total_quantity_ = 0.0;

    void add(OrderId id, double qty) {
      orders_.emplace_back(Order{id, qty});
      auto it = std::prev(orders_.end());
      order_map_[id] = it;
      total_quantity_ += qty;
    }

    void modify(OrderId id, double new_qty) {
      auto it = order_map_.find(id);
      if (it != order_map_.end()) {
        total_quantity_ -= it->second->quantity;
        it->second->quantity = new_qty;
        total_quantity_ += new_qty;
      }
    }

    void remove(OrderId id) {
      auto it = order_map_.find(id);
      if (it != order_map_.end()) {
        total_quantity_ -= it->second->quantity;
        orders_.erase(it->second);
        order_map_.erase(it);
      }
    }

    void clear() {
      orders_.clear();
      order_map_.clear();
      total_quantity_ = 0.0;
    }

    bool empty() const { return orders_.empty(); }

  };
  using LevelMap = std::map<Price, Level, Compare>; // descending (std::greater<Price>) for bids
                                                    // ascending (std::less<Price>) for asks
  LevelMap levels_;

  void add_order(Price price, OrderId id, double qty) {
    levels_[price].add(id, qty);
  }

  void modify_order(Price price, OrderId id, double new_qty) {
    auto it = levels_.find(price);
    if (it != levels_.end()) {
      it->second.modify(id, new_qty);
    }
  }

  void remove_order(Price price, OrderId id) {
    auto it = levels_.find(price);
    if (it != levels_.end()) {
      it->second.remove(id);
      if (it->second.empty()) {
        levels_.erase(it);
      }
    }
  }

  const Level *best_level() const {
    if (!levels_.empty())
      return &levels_.begin()->second;
    return nullptr;
  }

  void clear() { levels_.clear(); }
};

#endif

