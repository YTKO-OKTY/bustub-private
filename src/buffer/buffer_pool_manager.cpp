//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"

#include <list>
#include <unordered_map>

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] pages_;
  delete replacer_;
}

Page *BufferPoolManager::FetchPageImpl(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.

  latch_.lock();

  std::unordered_map<page_id_t, frame_id_t>::iterator search = page_table_.find(page_id);
  Page* retPage;
  if (search != page_table_.end()){
    frame_id_t fid = search->second;
    retPage = &pages_[fid];
    retPage -> pin_count_++;
    latch_.unlock();
    return retPage;
  }


  frame_id_t pageFrameID;
  Page *replPage;
  if (!free_list_.empty()){
    pageFrameID = free_list_.front();
    free_list_.pop_front();
    page_table_.insert({page_id, pageFrameID});
    replPage = &pages_[pageFrameID];
    replPage -> page_id_ = page_id;
    replPage -> pin_count_ = 1;
    replPage -> is_dirty_ = false;
    replacer_ -> Pin(pageFrameID);
    disk_manager_ -> ReadPage(page_id, replPage -> data_);
    latch_.unlock();
    return replPage;
  }

  if (!replacer_ -> Victim(&pageFrameID)){
    latch_.unlock();
    return nullptr;
  }
  
  replPage = &pages_[pageFrameID];
  if (replPage->IsDirty()){
    disk_manager_ -> WritePage(replPage -> page_id_, replPage -> data_);
  }
  page_table_.erase(page_table_.find(replPage -> page_id_));
  page_table_.insert({page_id, pageFrameID});

  replPage -> page_id_ = page_id;
  replPage -> pin_count_ = 1;
  replPage -> is_dirty_ = false;
  replacer_ -> Pin(pageFrameID);
  disk_manager_->ReadPage(page_id, replPage->data_);
  latch_.unlock();
  return replPage;
}

bool BufferPoolManager::UnpinPageImpl(page_id_t page_id, bool is_dirty) { 
  
  latch_.lock();
  frame_id_t unpinned_frame_id = page_table_.find(page_id) -> second;
  Page *unpinned_page = &pages_[unpinned_frame_id];

  if (unpinned_page -> pin_count_ <= 0){
    latch_.unlock();
    return false;
  }

  //frame_id_t victim_frame = page_table_.find(page_id) -> second;
  
  unpinned_page ->pin_count_--;
  if (is_dirty){
    unpinned_page -> is_dirty_ = true;
  }
  if (unpinned_page -> pin_count_ == 0){
    replacer_ -> Unpin(unpinned_frame_id);
  }

  latch_.unlock();
  return true;
 }

bool BufferPoolManager::FlushPageImpl(page_id_t page_id) {
  // Make sure you call DiskManager::WritePage!
  return false;
}

Page *BufferPoolManager::NewPageImpl(page_id_t *page_id) {
  // 0.   Make sure you call DiskManager::AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  
  latch_.lock();
  size_t free_list_size = BufferPoolManager::free_list_.size();
  size_t unpinned_non_free_pages = BufferPoolManager::replacer_->Size();
  if(free_list_size == 0 && unpinned_non_free_pages == 0){
    latch_.unlock();
    return nullptr;
  }

  frame_id_t pageFrameID;
  Page *new_page;
  page_id_t new_page_id;
  if (!free_list_.empty()){
    pageFrameID = free_list_.front();
    free_list_.pop_front();

    new_page_id = disk_manager_->AllocatePage();
    new_page = &pages_[pageFrameID];
    page_table_[new_page_id] = pageFrameID;
    new_page -> page_id_ = new_page_id;
    new_page -> pin_count_ = 1;
    new_page -> is_dirty_ = false;
    
    latch_.unlock();
    return new_page;
  }

  if (!replacer_ -> Victim(&pageFrameID)){
    latch_.unlock();
    return nullptr;
  }
  
  Page *replPage = &pages_[pageFrameID];

  if (replPage ->IsDirty()){
    disk_manager_ -> WritePage(replPage -> page_id_, replPage -> data_);
  }
  page_table_.erase(page_table_.find(replPage -> page_id_));
  new_page_id = disk_manager_ -> AllocatePage();
  page_table_.insert({new_page_id, pageFrameID});

  replPage -> page_id_ = new_page_id;
  replPage -> pin_count_ = 1;
  

  *page_id = new_page_id;

  latch_.unlock();
  return replPage;
  
}

bool BufferPoolManager::DeletePageImpl(page_id_t page_id) {
  // 0.   Make sure you call DiskManager::DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  return false;
}

void BufferPoolManager::FlushAllPagesImpl() {
  // You can do it!
}

}  // namespace bustub
