#pragma once

#include <cstdio>
#include <memory>
#include <typeinfo>
#include <arpa/nameser.h>
#include <assert.h>

template <class T>
class Stack {
    static constexpr int INITIAL_BUFFER_SIZE = 1;
    static constexpr int BUFFER_GROW_COEFFICIENT = 2;
    static constexpr int BUFFER_SHRINK_COEFFICIENT = 4;
    static constexpr uint8_t POISON = 0b1010011;
    static constexpr uint8_t CHECKSUM_OFFSET = 0b10101111;
    static constexpr int MAX_SANE_SIZE = 1 << 28;
    static constexpr uint64_t POINTER_POISON = 0xFAAF03659823AEFF;
    static void* PoisonPointer(const void* const a) {
        return (Stack *) (((uint64_t) a) ^ POINTER_POISON);
    }

#define TRY_PANIC()            \
do {                           \
    auto reason = Corrupted(); \
    if (reason != OK) {        \
        Dump(reason);          \
        assert(false);         \
    }                          \
} while(0)                     \

public:
    enum CorruptReason {
        OK                         = 0,
        NULL_THIS                  = 1,
        WRONG_SIZE                 = 2,
        WRONG_BUFFER_SIZE          = 3,
        NULL_BUFFER                = 4,
        WRONG_FRONT_GUARD          = 5,
        WRONG_REAR_GUARD           = 6,
        MISMATCHED_BUFFER_CHECKSUM = 7,
        MISMATHCED_TOTAL_CHECKSUM  = 8,
        BAD_UNINITIALIZED_BUFFER   = 9
    };

    Stack()
        : front_guard_(reinterpret_cast<Stack*>(PoisonPointer(this)))
        , buffer_size_(INITIAL_BUFFER_SIZE)
        , size_(0)
        , buffer_(AcquireBuffer(buffer_size_))
        , rear_guard_(reinterpret_cast<Stack*>(PoisonPointer(this))) {
        RecalcChecksums();
    }

    ~Stack() {
        DiscardBuffer(&buffer_, buffer_size_, size_);
    }

    void Push(const T& value) {
        TRY_PANIC();
        if (buffer_size_ <= size_) {
            Reallocate(buffer_size_ * BUFFER_GROW_COEFFICIENT);
        }
        MakeElement(size_++, value);
        RecalcChecksums();
        TRY_PANIC();
    }

    bool Pop(T* result) {
        TRY_PANIC();

        if (size_ <= 0) {
            return false;
        }

        *result = buffer_[--size_];
        Poison(&buffer_[size_]);

        if (size_ * BUFFER_SHRINK_COEFFICIENT <= buffer_size_) {
            Reallocate(buffer_size_ / BUFFER_GROW_COEFFICIENT);
        }

        RecalcChecksums();
        TRY_PANIC();
        return true;
    }

    size_t Size() const {
        TRY_PANIC();
        return size_;
    }

    bool Empty() const {
        TRY_PANIC();
        return size_ == 0;
    }

    CorruptReason Corrupted() const {
        if (!this)
            return NULL_THIS;
        if (size_ < 0 || size_ > MAX_SANE_SIZE || size_ > buffer_size_)
            return WRONG_SIZE;
        if (buffer_size_ <= 0 || buffer_size_ > MAX_SANE_SIZE)
            return WRONG_BUFFER_SIZE;
        if (!buffer_)
            return NULL_BUFFER;
        if (rear_guard_ != PoisonPointer(this))
            return WRONG_REAR_GUARD;
        if (front_guard_ != PoisonPointer(this)) {
            return WRONG_FRONT_GUARD;
        }
        if (CalcTotalChecksum() != total_checksum_)
            return MISMATHCED_TOTAL_CHECKSUM;
        if (CalcBufferChecksum() != buffer_checksum_) {
            return MISMATCHED_BUFFER_CHECKSUM;
        }
        {
            uint8_t* data = reinterpret_cast<uint8_t*>(buffer_);
            for (size_t i = size_ * sizeof(T); i < buffer_size_ * sizeof(T); ++i) {
                if (data[i] != POISON)
                    return BAD_UNINITIALIZED_BUFFER;
            }
        }
        return OK;
    }

    void Dump(CorruptReason reason) const {
        fprintf(stderr, "Stack of %s: ", typeid(T).name());
        fprintf(stderr, "Status: %d ", reason);
        if (reason != OK) {
            fprintf(stderr, "ERROR: ");
        }
        switch (reason) {
            case OK:
                fprintf(stderr, "ok\n");
                break;
            case NULL_THIS:
                fprintf(stderr, "this pointer is null\n");
                break;
            case WRONG_SIZE:
                fprintf(stderr, "wrong size: %d\n", size_);
                break;
            case WRONG_BUFFER_SIZE:
                fprintf(stderr, "wrong buffer size: %d\n", buffer_size_);
                break;
            case NULL_BUFFER:
                fprintf(stderr, "buffer pointer is null\n");
                break;
            case WRONG_FRONT_GUARD:
                fprintf(stderr, "corrupted front guard: %p instead of %p\n", front_guard_, this);
                break;
            case WRONG_REAR_GUARD:
                fprintf(stderr, "corrupted rear guard: %p instead of %p\n", rear_guard_, this);
                break;
            case MISMATCHED_BUFFER_CHECKSUM:
                fprintf(stderr, "mismatched buffer checksum: got %d expected %d \n", CalcBufferChecksum(), buffer_checksum_);
                break;
            case MISMATHCED_TOTAL_CHECKSUM:
                fprintf(stderr, "mismatched total checksum: got %d expected %d \n", CalcTotalChecksum(), total_checksum_);
                break;
            case BAD_UNINITIALIZED_BUFFER:
                fprintf(stderr, "some uninitialized values are not poisoned\n");
        }
        if (reason == NULL_THIS) {
            return;
        }
        fprintf(stderr, "front guard: %p\n", front_guard_);
        fprintf(stderr, "buffer size: %d\n", buffer_size_);
        fprintf(stderr, "size: %d\n", size_);
        fprintf(stderr, "buffer pointer %p\n", buffer_);
        if (buffer_) {
            fprintf(stderr, "buffer of %s of width %lu [\n", typeid(T).name(), sizeof(T));
            for (int i = 0; i < buffer_size_; ++i) {
                if (i < size_) {
                    fprintf(stderr, "(*) ");
                } else {
                    fprintf(stderr, "    ");
                }
                uint8_t* data = reinterpret_cast<uint8_t*>(buffer_ + i);
                bool poisoned = true;
                for (size_t j = 0; j < sizeof(T); ++j) {
                    fprintf(stderr, " %02x", data[j]);
                    poisoned &= data[j] == POISON;
                }
                if (poisoned) {
                    fprintf(stderr, " (POISON)");
                }
                fprintf(stderr, ",\n");
            }
            fprintf(stderr, "]\n");
        }
        fprintf(stderr, "buffer checksum: %d\n", buffer_checksum_);
        fprintf(stderr, "total checksum: %d\n", total_checksum_);
        fprintf(stderr, "rear guard: %p\n", rear_guard_);
    }

private:
    void Poison(T* value) {
        uint8_t* data = reinterpret_cast<uint8_t*>(value);
        for (size_t i = 0; i < sizeof(T); ++i) {
            data[i] = POISON;
        }
    }

    T* AcquireBuffer(size_t size) {
        T* buffer =  std::allocator_traits<std::allocator<T>>::allocate(allocator_, size);
        for (size_t i = 0; i < size; ++i) {
            Poison(&buffer[i]);
        }
        return buffer;
    }

    void DiscardBuffer(T** buffer, size_t size, size_t occupied) {
        for (size_t i = 0; i < occupied; ++i) {
            std::allocator_traits<std::allocator<T>>::destroy(allocator_, *buffer + i);
        }
        std::allocator_traits<std::allocator<T>>::deallocate(allocator_, *buffer, size);
        *buffer = reinterpret_cast<T*>(PoisonPointer(*buffer));
    }

    void MakeElement(size_t position, const T& element) {
        std::allocator_traits<std::allocator<T>>::construct(allocator_, buffer_ + position, element);
    }
    
    void Reallocate(int new_size) {
        T *old_buffer_ = buffer_;
        buffer_ = AcquireBuffer(new_size);
        for (int i = 0; i < size_; ++i) {
            MakeElement(i, old_buffer_[i]);
        }
        DiscardBuffer(&old_buffer_, buffer_size_, size_);
        buffer_size_ = new_size;
    }

    uint16_t CalcBufferChecksum() const {
        uint8_t sum1 = CHECKSUM_OFFSET;
        uint8_t sum2 = CHECKSUM_OFFSET;
        for (int i = 0; i < buffer_size_; ++i) {
            AddToSum(buffer_[i], &sum1, &sum2);

        }
        return (static_cast<uint16_t>(sum2) << 8) | sum1;
    }

    uint16_t CalcTotalChecksum() const {
        uint8_t sum1 = CHECKSUM_OFFSET;
        uint8_t sum2 = CHECKSUM_OFFSET;
        
        AddToSum(front_guard_, &sum1, &sum2);
        AddToSum(allocator_, &sum1, &sum2);
        AddToSum(buffer_size_, &sum1, &sum2);
        AddToSum(size_, &sum1, &sum2);
        AddToSum(buffer_, &sum1, &sum2);
        AddToSum(buffer_checksum_, &sum1, &sum2);
        AddToSum(rear_guard_, &sum1, &sum2);
        
        return (static_cast<uint16_t>(sum2) << 8) | sum1;
    }

    void RecalcChecksums() {
        buffer_checksum_ = CalcBufferChecksum();
        total_checksum_ = CalcTotalChecksum();
    }

    template <class S>
    static void AddToSum(const S& field, uint8_t *sum1, uint8_t *sum2) {
        auto data = reinterpret_cast<const uint8_t*>(&field);
        for (size_t i = 0; i < sizeof(S); ++i) {
            *sum1 += data[i];
            *sum2 += *sum1;
        }
    }
    const Stack* const front_guard_;
    std::allocator<T> allocator_;
    int buffer_size_;
    int size_;
    T* buffer_;
    uint16_t buffer_checksum_;
    uint16_t total_checksum_;
    const Stack* const rear_guard_;

#undef TRY_PANIC
};