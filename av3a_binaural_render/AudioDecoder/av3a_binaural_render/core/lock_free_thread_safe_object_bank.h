/* Copyright 2021 Beijing Zitiao Network Technology Co.,
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <unordered_set>
#include <mutex>
#include <condition_variable>

namespace avs3renderer {
template <class T>
class LockFreeThreadSafeObjectBank {
public:

    //  DEfinition of the entries in the object bank
    struct Entry {
        std::shared_ptr<T> object_ptr;      //  object_ptr stores the shared pointer of the object memory
        std::atomic_int life_cycle;         //  life_cycle is a lock-free atomic status indicates the objects
                                            // memory life cycle
        std::atomic_bool is_inuse;          //  is_inuse is a lock-free atomic boolean status indicates whether this
                                            // object is in use or not.

        //  Life cycle status constants
        static constexpr int kLifeCycle_NotInitialized = 0;
        static constexpr int kLifeCycle_Initializing = 1;
        static constexpr int kLifeCycle_Initialized = 2;
        static constexpr int kLifeCycle_Deleting = 3;

        //  Default constructor of an entry
        Entry() : life_cycle(kLifeCycle_NotInitialized), is_inuse(false) {
            object_ptr = nullptr;
        }
    };

    //  Constructor of this object bank of type T
    //  The only parameter you can personalize is the max object count,
    //  indicating the maximum number of objects allowed to be stored in this bank
    explicit LockFreeThreadSafeObjectBank(size_t max_object_count = 0)
        : bank_(max_object_count),
          entry_processes_(max_object_count),
          entry_processes_mutex_(max_object_count),
          entry_processes_resume_cv_(max_object_count) {
        free_object_id_set_.reserve(max_object_count);
        for (int id = max_object_count - 1; id >= 0; --id)
            free_object_id_set_.insert(id);
    }

    //  Destructor of this object bank.
    //  Note that the destruction process will wait for all entry storing and removing processes to be finished.
    ~LockFreeThreadSafeObjectBank() {
        for (int i = 0; i < bank_.size(); ++i) {
            if (entry_processes_[i].joinable()) {
                entry_processes_resume_cv_[i].notify_one();
                entry_processes_[i].join();
            }
        }
    }

    //  Resizing this bank for another maximum size you wish.
    //  Note that this method can involve lots of memory allocation and copying, so you definitely shouldn't call it in
    //  any process with hard time budget.
    void Resize(size_t new_size) {
        size_t old_size = bank_.size();
        if (old_size == new_size)
            return;

        for (int i = old_size - 1; i >= new_size; --i) {
            if (entry_processes_[i].joinable()) {
                entry_processes_resume_cv_[i].notify_one();
                entry_processes_[i].join();
            }
        }

        bank_.resize(new_size);
        entry_processes_.resize(new_size);
        entry_processes_mutex_.resize(new_size);
        entry_processes_resume_cv_.resize(new_size);
        if (new_size > old_size) {
            free_object_id_set_.reserve(new_size);
            for (int id = old_size; id < new_size; ++id)
                free_object_id_set_.insert(id);
        } else {
            for (int id = old_size - 1; id >= new_size; --id)
                free_object_id_set_.erase(id);
        }
    }

    //  Insert a new object into the bank with the initialization parameter provided by Args
    //  This method works asynchronously with the thread that calls it, so it won't block either the caller thread, or
    //  the thread that's, at the same time, requiring and using this entry from this object bank.
    template <class... Args>
    int InsertAsync(Args&&... args) {
        if (free_object_id_set_.empty())
            return -1;
        int entry_id = *free_object_id_set_.begin();
        if (entry_processes_[entry_id].joinable())
            entry_processes_[entry_id].join();
        entry_processes_[entry_id] = std::thread([this, entry_id, args...] { this->InsertEntry(entry_id, args...); });
        free_object_id_set_.erase(free_object_id_set_.begin());
        return entry_id;
    }

    //  Insert a new object into the bank with the initialization parameter provided by Args
    //  This method works synchronously with the thread that calls it, which might be time-consuming if the object type
    //  construction is.
    template <class... Args>
    int Insert(Args&&... args) {
        if (free_object_id_set_.empty())
            return -1;
        int entry_id = *free_object_id_set_.begin();
        if (entry_processes_[entry_id].joinable())
            entry_processes_[entry_id].join();
        bank_[entry_id].life_cycle = Entry::kLifeCycle_Initializing;
        bank_[entry_id].object_ptr = std::make_shared<T>(args...);
        bank_[entry_id].life_cycle = Entry::kLifeCycle_Initialized;
        free_object_id_set_.erase(free_object_id_set_.begin());
        return entry_id;
    }

    //  Remove an entry with specified entry id.
    //  This method works asynchronously with the thread that calls it, so it won't block either the caller thread, or
    //  the thread that's, at the same time, requiring and using this entry from this object bank.
    void RemoveAsync(int entry_id) {
        if (free_object_id_set_.count(entry_id) == 0)
            return;
        if (entry_processes_[entry_id].joinable())
            entry_processes_[entry_id].join();
        entry_processes_[entry_id] = std::thread([this, entry_id] { this->ResetEntry(entry_id); });
        free_object_id_set_.insert(entry_id);
    }

    //  Reporting to the object bank that entry with object_id is getting used now.
    void StartUsingObject(int object_id) {
        bank_[object_id].is_inuse = true;
    }

    //  Reporting to the object bank that entry with object_id is not used now. It also unblocks object deconstruction
    //  process if it was initiated
    void StopUsingObject(int object_id) {
        bank_[object_id].is_inuse = false;
        //  Notify object deconstruction worker that it can start reseting this entry
        entry_processes_resume_cv_[object_id].notify_one();
    }

    //  Query for an entry
    Entry& operator[](int entry_id) {
        return bank_[entry_id];
    }

    //  Query for an entry, more user-friendly when calling by pointers
    Entry& at(int entry_id) {
        return bank_[entry_id];
    }

    //  Check if this object bank contains specified entry
    bool Contains(int entry_id) const {
        return free_object_id_set_.count(entry_id) == 0;
    }

    //  Query for the entry id prepared for the next new object.
    int GetNextFreeEntryId() const {
        if (free_object_id_set_.empty())
            return -1;
        else
            return *free_object_id_set_.begin();
    }

    //  Check if this object bank is full
    bool isFull() const {
        return free_object_id_set_.empty();
    }

private:
    template <class... Args>
    void InsertEntry(int entry_id, Args&&... args) {
        Entry& entry = bank_[entry_id];
        entry.life_cycle = Entry::kLifeCycle_Initializing;
        entry.object_ptr = std::shared_ptr<T>(new T(args...));
        entry.life_cycle = Entry::kLifeCycle_Initialized;
    }

    void ResetEntry(int entry_id) {
        Entry& entry = bank_[entry_id];
        entry.life_cycle = Entry::kLifeCycle_Deleting;
        {
            std::unique_lock<std::mutex> lock(entry_processes_mutex_[entry_id]);
            entry_processes_resume_cv_[entry_id].wait(lock, [&entry] { return entry.is_inuse == false; });
        }
        entry.object_ptr = nullptr;
        entry.life_cycle = Entry::kLifeCycle_NotInitialized;
    }

    std::vector<Entry> bank_;
    std::vector<std::thread> entry_processes_;
    std::vector<std::mutex> entry_processes_mutex_;
    std::vector<std::condition_variable> entry_processes_resume_cv_;
    std::unordered_set<int> free_object_id_set_;
};
}  // namespace avs3renderer
