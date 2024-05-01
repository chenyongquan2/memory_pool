#include "memory_pool.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>

using namespace std;

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

int main()
{
    void* p1Bake = nullptr;
    void* p2Bake = nullptr;
    {
        // test allocator
        MemAllocatorGuard m1(sizeof(int));
        void* p1 = m1.GetMem();
        assert(p1 != nullptr);
        int* pNum1 = static_cast<int*>(p1);
        *pNum1 = 20;
        cout << "*pNum1:" << *pNum1 << endl;

        MemAllocatorGuard m2(sizeof(int));
        void* p2 = m2.GetMem();
        assert(p2 != nullptr);
        int* pNum2 = static_cast<int*>(p2);
        *pNum2 = 10;
        cout << "*pNum2:" << *pNum2 << endl;

        auto ptrDiff = static_cast<char*>(p2) - static_cast<char*>(p1);
        cout << "p2-p1:" << ptrDiff << endl;
        // assert(ptrDiff==sizeof(int));

        p1Bake = p1;
        p2Bake = p2;
    }

    void* p11Bake = nullptr;
    void* p22Bake = nullptr;
    {
        MemAllocatorGuard m1(sizeof(int));
        void* p1 = m1.GetMem();
        assert(p1 != nullptr);
        int* pNum1 = static_cast<int*>(p1);
        *pNum1 = 20;
        cout << "*pNum1:" << *pNum1 << endl;

        MemAllocatorGuard m2(sizeof(int));
        void* p2 = m2.GetMem();
        assert(p2 != nullptr);
        int* pNum2 = static_cast<int*>(p2);
        *pNum2 = 10;
        cout << "*pNum2:" << *pNum2 << endl;

        auto ptrDiff = static_cast<char*>(p2) - static_cast<char*>(p1);
        cout << "p2-p1:" << ptrDiff << endl;
        // assert(ptrDiff==sizeof(int));

        p11Bake = p1;
        p22Bake = p2;
    }

    {
        // test deallocator
        assert(p1Bake == p11Bake);
        assert(p2Bake == p22Bake);
    }

    {
        // test allocator for the memory size greater 128 bytes
        constexpr size_t arr_num { 100 };
        MemAllocatorGuard m3(sizeof(int) * arr_num);
        void* p3 = m3.GetMem();
        assert(p3 != nullptr);
        int* pArr3 = static_cast<int*>(p3);
        for (auto i = 0; i < arr_num; i++) {
            //*(pArr3+i) = i;
            pArr3[i] = i + 1;
        }
        size_t sum { 0 };
        for (auto i = 0; i < arr_num; i++) {
            sum += pArr3[i];
        }
        cout << "sum:" << sum << endl;
    }

    return 0;
}