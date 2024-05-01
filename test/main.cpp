#include <cstddef>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <vector>
// #include <iostream>
// #include <chrono> // 包含时间库
// #include <thread> // 包含线程库
#include "../src/mem_allocator_guard.hpp"

TEST(MemoryPoolTestLess128Bytes, allocator)
{
    MemAllocatorGuard m1(sizeof(int));
    void* p1 = m1.GetMem();
    EXPECT_TRUE(p1 != nullptr);
    int* pNum1 = static_cast<int*>(p1);
    *pNum1 = 20;
    EXPECT_EQ(*pNum1, 20);

    MemAllocatorGuard m2(sizeof(int));
    void* p2 = m2.GetMem();
    EXPECT_TRUE(p2 != nullptr);
    int* pNum2 = static_cast<int*>(p2);
    *pNum2 = 10;
    EXPECT_EQ(*pNum2, 10);
}

TEST(MemoryPoolTestLess128Bytes, deallocate)
{
    void* p1Bake = nullptr;
    void* p2Bake = nullptr;
    {
        MemAllocatorGuard m1(sizeof(int));
        void* p1 = m1.GetMem();

        MemAllocatorGuard m2(sizeof(int));
        void* p2 = m2.GetMem();

        p1Bake = p1;
        p2Bake = p2;
    }

    void* p11Bake = nullptr;
    void* p22Bake = nullptr;
    {
        MemAllocatorGuard m1(sizeof(int));
        void* p1 = m1.GetMem();

        MemAllocatorGuard m2(sizeof(int));
        void* p2 = m2.GetMem();

        p11Bake = p1;
        p22Bake = p2;
    }

    EXPECT_EQ(p1Bake, p11Bake);
    EXPECT_EQ(p2Bake, p22Bake);
}

TEST(MemoryPoolTestGreater128Bytes, allocator)
{
    constexpr size_t arr_num { 100 };
    MemAllocatorGuard m3(sizeof(int) * arr_num);
    void* p3 = m3.GetMem();
    EXPECT_TRUE(p3 != nullptr);
    int* pArr3 = static_cast<int*>(p3);
    for (auto i = 0; i < arr_num; i++) {
        //*(pArr3+i) = i;
        pArr3[i] = i + 1;
    }
    size_t sum { 0 };
    for (auto i = 0; i < arr_num; i++) {
        sum += pArr3[i];
    }

    size_t exp_num { 0 };
    for (auto i = 0; i < arr_num; i++) {
        exp_num += (i + 1);
    }

    EXPECT_EQ(sum, exp_num);
}

TEST(MemoryPoolTestGreater128Bytes, deallocate)
{
    void* p3Bake = { nullptr };
    {
        constexpr size_t arr_num { 100 };
        MemAllocatorGuard m3(sizeof(int) * arr_num);
        void* p3 = m3.GetMem();
        EXPECT_TRUE(p3 != nullptr);
        p3Bake = p3;
    }

    void* p33Bake = { nullptr };
    {
        constexpr size_t arr_num { 100 };
        MemAllocatorGuard m3(sizeof(int) * arr_num);
        void* p3 = m3.GetMem();
        EXPECT_TRUE(p3 != nullptr);
        p33Bake = p3;
    }
    // 下面这个不能保证百分百成立
    // 在C++中，通过delete释放的内存会返回给操作系统的堆内存池。
    // 然而，操作系统是否立即回收这些内存取决于具体的操作系统和内存管理策略。
    // 通常情况下，操作系统会将已释放的内存标记为可再分配的，并在需要时将其重新分配给新的内存申请。
    // EXPECT_TRUE(p3Bake != p33Bake);
}

// TEST(MemoryPoolTestAllocateLoop, allocator)
// {
//     // calc the pysical memory size:
//     size_t pysical_memory_size = 0;

//     using MemAllocatorGuardVecType = std::vector<std::unique_ptr<MemAllocatorGuard>>;
//     MemAllocatorGuardVecType memAllocatorGuardVec;
//     //constexpr auto times { 100000 };
//     constexpr auto times { 1 };
//     for (int i = 0; i < times; i++) {
//         for (size_t itemSize = 8; itemSize < 1024; itemSize += 8) {
//             auto ptr = std::make_unique<MemAllocatorGuard>(itemSize);
//             pysical_memory_size += itemSize;
//             memAllocatorGuardVec.push_back(std::move(ptr));
//         }
//     }

//     EXPECT_TRUE(!memAllocatorGuardVec.empty());
//     for (auto i = 0; i < memAllocatorGuardVec.size(); i++) {
//         auto p = memAllocatorGuardVec.at(i)->GetMem();
//         EXPECT_TRUE(p != nullptr);
//     }
//     // std::this_thread::sleep_for(std::chrono::seconds(3));
//     // std::cout << "memAllocatorGuardVec:" << pysical_memory_size << "bytes" << std::endl;
// }

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
