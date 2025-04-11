//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"
#include <algorithm>

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages) {}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  std::lock_guard<std::mutex> lock(latch_);
  if (lru_list_.empty()) {
    return false;
  }
  *frame_id = lru_list_.front();
  lru_list_.pop_front();
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(latch_);
  lru_list_.remove(frame_id);
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  std::lock_guard<std::mutex> lock(latch_);
  if (std::find(lru_list_.begin(), lru_list_.end(), frame_id) == lru_list_.end()) {
    lru_list_.push_back(frame_id);
  }
}

size_t LRUReplacer::Size() {
  std::lock_guard<std::mutex> lock(latch_);
  return lru_list_.size();
}

}  // namespace bustub
