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

      bool operator==(const Order &other) const {
        // weak equality, only requires quantity to be the same
        return quantity == other.quantity;
        // strong equality, requires id to be the same
        // return id == other.id && quantity == other.quantity;
      }
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

    void modifyId(OrderId id, double new_qty) {
      auto it = order_map_.find(id);
      if (it != order_map_.end()) {
        total_quantity_ -= it->second->quantity;
        it->second->quantity = new_qty;
        total_quantity_ += new_qty;
      }
    }

    // clears the entire level
    void modifyLevel(OrderId id, double new_qty) {
      clear();
      add(id, new_qty);
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

    bool operator==(const Level &other) const {
      return orders_ == other.orders_ && total_quantity_ == other.total_quantity_;
    }

  };
  using LevelMap = std::map<Price, Level, Compare>; // descending (std::greater<Price>) for bids
                                                    // ascending (std::less<Price>) for asks
  LevelMap levels_;

  void addOrderId(Price price, OrderId id, double qty) {
    levels_[price].add(id, qty);
  }

  // WARN: modifies based on id
  void modifyOrderId(Price price, OrderId id, double new_qty) {
    auto it = levels_.find(price);
    if (it != levels_.end()) {
      it->second.modifyId(id, new_qty);
    }
  }

  // If the order with id exists, modify its quantity
  // else add it as a new Level
  void addModifyOrderId(Price price, OrderId id, double qty) {
    auto it = levels_.find(price);
    if (it != levels_.end()) {
      it->second.modifyId(id, qty);
    } else {
      addOrderId(price, id, qty);
    }
  }

  // modifies the entire level, useful for order books that don't have
  // queues, but only entire quantities available at a given level
  void addModifyLevel(Price price, OrderId id, double qty) {
    auto it = levels_.find(price);
    if (it != levels_.end()) {
      it->second.modifyLevel(id, qty);
    } else {
      Level new_level;
      new_level.add(id, qty);
      levels_[price] = std::move(new_level);
    }
  }

  void removeOrder(Price price, OrderId id) {
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

  bool operator==(const Ladder &other) const {
    return levels_ == other.levels_;
  }
};

#endif

