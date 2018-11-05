#pragma once

#include <vector>

template <class Block>
class VectorBlockHolder {
public:
    Block& at(size_t pos) {
        return buffer_.at(pos);
    }

    size_t AddBlock(const Block& block) {
        buffer_.push_back(block);
        return buffer_.size() - 1;
    }

private:
    std::vector<Block> buffer_;
};

template <class T, template<class> class BlockHolder = VectorBlockHolder>
class List {
    struct Node;
public:

    class Iterator {
        friend List;

    public:
        T& operator*() {
            return holder_->at(pos_).value;
        }

        T* operator->() {
            return &operator*();
        }

        Iterator& operator++() {
            pos_ = holder_->at(pos_).next;
            return *this;
        }

        Iterator& operator--() {
            if (pos_ == 0) {
                pos_ = *tail_;
                return *this;
            }
            pos_ = holder_->at(pos_).prev;
            return *this;
        }

    private:
        Iterator(BlockHolder<Node>* holder, const size_t* tail, size_t pos) : holder_(holder), tail_(tail), pos_(pos) {
        }

        BlockHolder<Node>* holder_;
        const size_t* tail_;
        size_t pos_;
    };

    List() : next_free_block_(0), head_(0), tail_(0), size_(0) {
        GetNewNode(T(), 0, 0);
    }

    ~List() {
        while (size_ > 0) {
            PopBack();
        }
    }

    void PushFront(const T& value) {
        size_t pos = GetFreeNode(value);
        ++size_;

        if (head_ == 0) {
            head_ = pos;
            tail_ = pos;
            return;
        }

        Connect(pos, head_);
        head_ = pos;
    }

    void PushBack(const T& value) {
        size_t pos = GetFreeNode(value);
        ++size_;

        if (tail_ == 0) {
            head_ = pos;
            tail_ = pos;
            return;
        }

        Connect(tail_, pos);
        tail_ = pos;
    }

    T& Back() {
        return holder_.at(tail_).value;
    }

    T& Front() {
        return holder_.at(head_).value;
    }

    void PopFront() {
        --size_;
        size_t new_head = holder_.at(head_).next;
        Utilize(head_);
        if (size_ > 0) {
            head_ = new_head;
            holder_.at(head_).prev = 0;
        } else {
            head_ = 0;
            tail_ = 0;
        }
    }

    void PopBack() {
        --size_;
        size_t new_tail = holder_.at(tail_).prev;
        Utilize(tail_);
        if (size_ > 0) {
            tail_ = new_tail;
            holder_.at(tail_).next = 0;
        } else {
            head_ = 0;
            tail_ = 0;
        }
    }

    Iterator begin() {
        return MakeIterator(head_);
    }

    Iterator end() {
        return MakeIterator(0);
    }

    Iterator InsertBefore(Iterator iterator, const T& value) {
        if (iterator.pos_ == 0) {
            PushBack(value);
            return MakeIterator(tail_);
        }
        if (iterator.pos_ == head_) {
            PushFront(value);
            return MakeIterator(head_);
        }
        ++size_;
        size_t second = iterator.pos_;
        --iterator;
        size_t first = iterator.pos_;
        size_t node = GetFreeNode(value);
        Connect(first, node);
        Connect(node, second);
        return MakeIterator(node);
    }

    void Erase(Iterator iterator) {
        if (iterator.pos_ == tail_) {
            PopBack();
            return;
        }
        if (iterator.pos == head_) {
            PopFront();
            return;
        }
        --size_;
        size_t pos = iterator.pos_;
        size_t first = holder_.at(pos).prev;
        size_t second = holder_.at(pos).next;
        Utilize(pos);
        Connect(first, second);
    }

    size_t Size() const {
        return size_;
    }

private:
    struct Node {
        T value;
        size_t id;
        size_t next;
        size_t prev;
    };

    Iterator MakeIterator(size_t pos) {
        return Iterator(&holder_, &tail_, pos);
    }

    size_t GetNewNode(const T& value, size_t prev, size_t next) {
        size_t pos = holder_.AddBlock({.value = value, .next = next, .prev = prev});
        holder_.at(pos).id = pos;
        return pos;
    }

    size_t GetFreeNode(const T& value) {
        if (next_free_block_ != 0) {
            size_t result = next_free_block_;
            next_free_block_ = holder_.at(next_free_block_).next;
            holder_.at(result).value = value;
            holder_.at(result).next = 0;
            holder_.at(result).prev = 0;
            return result;
        }
        return GetNewNode(value, 0, 0);
    }

    void Utilize(size_t pos) {
        holder_.at(pos).prev = 0;
        holder_.at(pos).next = next_free_block_;
        next_free_block_ = pos;
    }

    void Connect(size_t first, size_t second) {
        holder_.at(second).prev = first;
        holder_.at(first).next = second;
    }

    BlockHolder<Node> holder_;
    size_t next_free_block_;
    size_t head_;
    size_t tail_;
    size_t size_;
};