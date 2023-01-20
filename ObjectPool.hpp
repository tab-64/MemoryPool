#ifndef __OBJECT_POOL_HPP__
#define __OBJECT_POOL_HPP__

#include <exception>
#include <utility>
#include <type_traits>
#include <iostream>
namespace tab {

/**
 * @brief 
 * 
 * @tparam T The objects' type
 * @tparam BlockCap How many objects each block can hold.
 */
template <typename T, size_t BlockCap = 2<<20>
class ObjectPool {
public:
    using byte = char;
    using word = short;
    using dword = long;
    using qword = long long;

    static const size_t ObjectSize = sizeof(T);

protected:
    struct MemoryBlock {
        using unit_index_t
            = std::conditional_t<
                BlockCap <= 0x7Fu, byte, std::conditional_t<
                BlockCap <= 0x7FFFu, word, std::conditional_t<
                BlockCap <= 0x7FFFFFFFu, dword, qword>>>;
        MemoryBlock* next = nullptr;
        unit_index_t first = 0;
        unit_index_t used = 0;
        struct {
            unit_index_t next;
            byte mem[ObjectSize];
        } units[BlockCap] {};
    };

public:
    ObjectPool() : ObjectPool(1) { }

    /**
     * @brief Construct a new ObjectPool object
     * 
     * @param n_block The initial count of blocks
     */
    ObjectPool(size_t n_block) : first_(nullptr), total_block_(0) {
        this->addBlock(n_block);
    }

    ObjectPool(ObjectPool&& op) {
        this->swap(op);
    }

    ObjectPool(const ObjectPool&) = delete;

    ~ObjectPool() {
        for(MemoryBlock* ptr = first_; first_ != nullptr; ptr = first_) {
            first_ = ptr->next;
            delete ptr;
        }
    }

    void swap(ObjectPool& op) {
        std::swap(first_, op.first_);
        std::swap(total_block_, op.total_block_);
    }

    // size_t available() const {
        
    // }

    // size_t used() const {
        
    // }

    /**
     * @brief How many objects can this pool hold
     * 
     * @return size_t The value
     */
    size_t capacity() const {
        return total_block_ * BlockCap;
    }


    /**
     * @brief Add block(s) to this pool
     * 
     * @param n_block The count of the block(s) to add
     */
    void addBlock(size_t n_block) {
        if(n_block > 0) {
            MemoryBlock* temp;
            for(size_t i = 0; i < n_block; ++i) {
                temp = first_;
                first_ = new MemoryBlock;
                first_->next = temp;
                auto data = first_->units;
                for(size_t j = 1; j <= BlockCap; ++j) {
                    data->next = j;
                    ++data;
                }
            }
            total_block_ += n_block;
        }
        else {
            class UnavailableBlockCountException {} e;
            throw e;
        }
    }


    /**
     * @brief Remove some blocks from this pool. 
     *        The blocks actually removed may less than 'n_block',
     *        but this method will remove the blocks as many as possible.
     * 
     * @param n_block The expected count
     * @return size_t How many blocks actually removed
     * 
     */
    size_t removeBlock(size_t n_block) {
        size_t ret = 0;
        if(n_block > 0) {
            if(total_block_ != 0) {
                for(MemoryBlock* ptr = first_; ptr->next != nullptr; ) {
                    if(ptr->next->used == 0) {
                        MemoryBlock* temp = ptr->next;
                        ptr->next = temp->next;
                        delete temp;
                        --total_block_;
                        ++ret;
                        if(ret == n_block)
                            break;
                    }
                    else{
                        ptr = ptr->next;
                    }
                }
                if(first_->used == 0 && ret < n_block) {
                    MemoryBlock* temp = first_;
                    first_ = first_->next;
                    delete temp;
                    --total_block_;
                    ++ret;
                }
            }
        }
        else {
            class UnavailableBlockCountException {} e;
            throw e;
        }
        return ret;
    }


    /**
     * @brief Change the count of the blocks.
     * 
     * @param n_block The target count
     * @return size_t Count after calling
     */
    size_t resize(size_t n_block) {
        if(n_block == total_block_) {
            // Do nothing.
        }
        else if(n_block > total_block_) {
            this->addBlock(n_block - total_block_);
        }
        else if(n_block >= 0) {
            this->removeBlock(total_block_ - n_block);
        }
        else {
            class UnavailableBlockCountException {} e;
            throw e;
        }
        return total_block_;
    }
    

    /**
     * @brief Get a new object from this pool, with the given construction parameters.
     * 
     * @param args The parameters to pass to the constructor
     * @return T* The address of the new object
     */
    template <typename... Args>
    T* get(Args ...args) {
        if(first_ == nullptr)
            this->addBlock(1);
        MemoryBlock* blk = first_;
        for(; blk->used == BlockCap;) {
            blk = blk->next;
            if(blk == nullptr) {
                this->addBlock(1);
                blk = first_;
                break;
            }
        }
        ++blk->used;
        T* ret = new(blk->units[blk->first].mem) T(args...);
        blk->first = blk->units[blk->first].next;
        return ret;
    }
    

    /**
     * @brief Release a object which is allocated by this pool.
     * 
     * @param ptr The address of the object to release
     * 
     * @warning DO NOT invoke this for a ptr repetitively.
     */
    void release(void* ptr) {
        for(MemoryBlock* blk = first_; blk != nullptr; blk = blk->next) {
            if(blk->units < ptr && ptr < blk->units + BlockCap) {
                --(blk->used);
                size_t offset = (decltype(&blk->units[0]))((byte*)ptr - sizeof(typename MemoryBlock::unit_index_t)) - blk->units;
                blk->units[offset].next = blk->first;
                blk->first = offset;
                static_cast<T*>((void*)(blk->units[offset].mem))->~T();
                return;
            }
        }
        class UnavailableMemoryException {} e;
        throw e;
    }


protected:
    MemoryBlock* first_;
    size_t total_block_;

}; // class ObjectPool

} // namespace tab

#endif // __OBJECT_POOL_HPP__