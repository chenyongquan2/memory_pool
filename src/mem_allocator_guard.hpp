#pragma once

#include "memory_pool.hpp"

struct MemAllocatorGuard {
public:
    MemAllocatorGuard(size_t size) noexcept
        : size_(size)
        , pMem_(memory_pool<8, 128>::allocate<true>(size))
    {
    }
    ~MemAllocatorGuard() noexcept { memory_pool<8, 128>::deallocate(pMem_, size_); }
    void* GetMem() const { return pMem_; }

    void* pMem_;
    size_t size_;
};
