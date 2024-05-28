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
#include <cstdlib>
#include <cstring>
#include <vector>

namespace avs3renderer {

/// Templated ring buffer class
/// RingBuffer holds a vector buffer of |max_size| length, a write index and a read index.
/// Once initialized, buffer is set to |value|, write index and read index are both set to zero.
///
/// buffer: |----------------------------|
///          ^ write index = 0
///          ^ read index = 0
///
/// Once RingBuffer::Write is called, |input_buffer_| is copied to |buffer_|, |write_index_| is offset by |write_size_|
/// and becomes ｜write_index+write_size_｜. Meanwhile, |available_read_size_| = |available_read_size_| + |write_size|
/// If write_size == 4
/// buffer: |----------------------------|
///              ^ write index = 4
///          ^ read index = 0
///
///
template <typename T>
class RingBuffer {
public:
    enum State {
        ALL_GOOD,
        OVER_RUN,
        UNDER_RUN,
    };

    State state() const {
        return state_;
    }

    explicit RingBuffer(int max_size, const T& value = T());

    T& operator[](int idx) const;

    int AvailableReadSize() const;

    int max_size() const;

    int Write(const T* input_buffer, int write_size);
    int WriteWithOffsetFromHead(const T* input_buffer, int write_size, int write_offset_from_head);

    int OverlapAndAdd(const T* input_buffer, int write_size);

    int Read(T* output_buffer, int read_size);
    int ReadWithOffsetFromHead(T* output_buffer, int read_size, int read_offset_from_head);

    /// Move write index to the right |offset| samples.
    void OffsetWriteIndex(int offset) const;
    /// Move read index to the right |offset| samples.
    void OffsetReadIndex(int offset) const;

    void Clear();

private:
    mutable std::vector<T> buffer_;
    int max_size_;
    mutable int write_index_;
    mutable int read_index_;
    mutable int available_read_size_;
    State state_;

    int OffsetIndex(int original_index, int offset) const;
};

template <typename T>
RingBuffer<T>::RingBuffer(const int max_size, const T& value)
    : buffer_(max_size, value),
      max_size_(max_size),
      write_index_(0),
      read_index_(0),
      available_read_size_(0),
      state_(ALL_GOOD) {
}

template <typename T>
void RingBuffer<T>::Clear() {
    std::fill(buffer_.begin(), buffer_.end(), static_cast<T>(0.0f));
    write_index_ = 0;
    read_index_ = 0;
    available_read_size_ = 0;
    state_ = ALL_GOOD;
}

template <typename T>
T& RingBuffer<T>::operator[](int idx) const {
    OffsetReadIndex(idx);
    T& val = buffer_[read_index_];
    OffsetReadIndex(-idx);
    return val;
}

template <typename T>
int RingBuffer<T>::AvailableReadSize() const {
    return available_read_size_;
}

template <typename T>
int RingBuffer<T>::max_size() const {
    return max_size_;
}

template <typename T>
int RingBuffer<T>::Write(const T* input_buffer, int write_size) {
    if (write_size <= 0)
        return 0;

    //  If write size has exceeded max size
    //  We'll assign max size to write size
    if (write_size > max_size_) {
        write_size = max_size_;
    }

    /***  Ring buffer management for writing  ***/
    //  If write query can be fulfilled without wrapping the write index around
    if (write_size < max_size_ - write_index_) {
        std::memcpy(buffer_.data() + write_index_, input_buffer, write_size * sizeof(T));
    }
    //  If write query has to wrap the write index around
    else {
        //  Copy the first half block
        std::memcpy(buffer_.data() + write_index_, input_buffer, (max_size_ - write_index_) * sizeof(T));
        //  Copy the second half block
        std::memcpy(buffer_.data(), input_buffer + max_size_ - write_index_,
                    (write_size + write_index_ - max_size_) * sizeof(T));
    }

    OffsetWriteIndex(write_size);

    return write_size;
}

template <typename T>
int RingBuffer<T>::WriteWithOffsetFromHead(const T* input_buffer, int write_size, int write_offset_from_head) {
    if (write_size <= 0)
        return 0;

    //  If write size has exceeded max size
    //  We'll assign max size to write size
    if (write_size > max_size_) {
        write_size = max_size_;
    }

    OffsetWriteIndex(write_offset_from_head);

    /***  Ring buffer management for writing  ***/
    //  If write query can be fulfilled without wrapping the write index around
    if (write_size < max_size_ - write_index_) {
        std::memcpy(buffer_.data() + write_index_, input_buffer, write_size * sizeof(T));
    }
    //  If write query has to wrap the write index around
    else {
        //  Copy the first half block
        std::memcpy(buffer_.data() + write_index_, input_buffer, (max_size_ - write_index_) * sizeof(T));
        //  Copy the second half block
        std::memcpy(buffer_.data(), input_buffer + max_size_ - write_index_,
                    (write_size + write_index_ - max_size_) * sizeof(T));
    }

    OffsetWriteIndex(-write_offset_from_head);

    return write_size;
}

template <typename T>
int RingBuffer<T>::OverlapAndAdd(const T* input_buffer, int write_size) {
    if (write_size <= 0)
        return 0;

    //  If write size has exceeded max size
    //  We'll assign max size to write size
    if (write_size > max_size_) {
        write_size = max_size_;
    }

    /***  Ring buffer management for writing and add  ***/
    //  If write query can be fulfilled without wrapping the write index around
    if (write_size < max_size_ - write_index_) {
        for (int i = 0; i < write_size; ++i) {
            buffer_[write_index_ + i] += input_buffer[i];
        }
        write_index_ += write_size;
    }
    //  If write query has to wrap the write index around
    else {
        //  Copy and add the first half block
        int sample_count = 0;
        for (; write_index_ < max_size_; ++write_index_) {
            buffer_[write_index_] += input_buffer[sample_count++];
        }
        //  Copy add the second half block
        for (write_index_ = 0; sample_count < write_size; ++write_index_) {
            buffer_[write_index_] += input_buffer[sample_count++];
        }
    }

    available_read_size_ += write_size;

    return write_size;
}

template <typename T>
int RingBuffer<T>::Read(T* output_buffer, int read_size) {
    if (read_size <= 0)
        return 0;

    //  If read size has exceeded max size
    //  We'll assign max size to read size
    if (read_size > max_size_) {
        read_size = max_size_;
    }

    /***  Ring buffer management for reading  ***/
    //  If read query can be fulfilled without wrapping the read index around
    if (read_size < max_size_ - read_index_) {
        std::memcpy(output_buffer, buffer_.data() + read_index_, read_size * sizeof(T));
    }
    //  If read query has to wrap the read index around
    else {
        //  Copy the first half block
        std::memcpy(output_buffer, buffer_.data() + read_index_, (max_size_ - read_index_) * sizeof(T));
        //  Copy the second half block
        std::memcpy(output_buffer + max_size_ - read_index_, buffer_.data(),
                    (read_size + read_index_ - max_size_) * sizeof(T));
    }

    OffsetReadIndex(read_size);

    return read_size;
}

template <typename T>
int RingBuffer<T>::ReadWithOffsetFromHead(T* output_buffer, int read_size, int read_offset_from_head) {
    if (read_size <= 0)
        return 0;

    //  If read size has exceeded max size
    //  We'll assign max size to read size
    if (read_size > max_size_) {
        read_size = max_size_;
    }

    OffsetReadIndex(read_offset_from_head);

    /***  Ring buffer management for reading  ***/
    //  If read query can be fulfilled without wrapping the read index around
    if (read_size < max_size_ - read_index_) {
        std::memcpy(output_buffer, buffer_.data() + read_index_, read_size * sizeof(T));
    }
    //  If read query has to wrap the read index around
    else {
        //  Copy the first half block
        std::memcpy(output_buffer, buffer_.data() + read_index_, (max_size_ - read_index_) * sizeof(T));
        //  Copy the second half block
        std::memcpy(output_buffer + max_size_ - read_index_, buffer_.data(),
                    (read_size + read_index_ - max_size_) * sizeof(T));
    }

    OffsetReadIndex(-read_offset_from_head);

    return read_size;
}

template <typename T>
void RingBuffer<T>::OffsetWriteIndex(int offset) const {
    write_index_ = OffsetIndex(write_index_, offset);
    available_read_size_ += offset;
}

template <typename T>
void RingBuffer<T>::OffsetReadIndex(int offset) const {
    read_index_ = OffsetIndex(read_index_, offset);
    available_read_size_ -= offset;
}

template <typename T>
int RingBuffer<T>::OffsetIndex(int original_index, int offset) const {
    int result_index = original_index + offset;
    while (result_index < 0) {
        result_index += max_size_;
    }
    while (result_index >= max_size_) {
        result_index -= max_size_;
    }

    return result_index;
}

}  // namespace avs3renderer
