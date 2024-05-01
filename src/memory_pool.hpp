#pragma once

#include <cassert>
#include <cstddef>
#include <vcruntime_new.h>

// 模板参数
template <size_t Align = 8, size_t MaxBytes = 128>
class memory_pool {
public:
    // 模板参数NoThrowExep
    template <bool NoThrowExep = false>
    static void* allocate(size_t);
    static void deallocate(void*, size_t) noexcept;

private:
    union free_list_node {
        free_list_node* next_;
        // 为了data的保持长度和next_成员相同
        char data_[sizeof next_];
        // char data_[sizeof(next_)];
    };

    // static member
private:
    // 经典写法：constexpr static auto align{Align};
    // 使用 constexpr static auto 的组合可以将变量声明为编译时常量、静态成员变量，
    // 并且让编译器自动推导变量类型。这种写法的好处是简洁、高效，并且能够在编译阶段进行优化和计算。

    // align表示free_list_node区块按各个类型的区块都是8字节的倍数来对齐
    constexpr static auto align { Align }; // 其类型会根据 Align 的值进行自动推导
    // max_bytes表示free_list_node区块最大的大小
    constexpr static auto max_bytes { MaxBytes };
    constexpr static auto free_list_nums { max_bytes / align };

private:
    // 在这段代码中，volatile 关键字用于修饰 free_list 数组的类型，即
    // free_list_node *volatile。volatile
    // 是一个类型修饰符，用于指示编译器不应该对被修饰的对象进行优化，以确保对该对象的读写操作都是可见的且按照指定的顺序进行。
    // 在多线程或并发环境中，如果多个线程同时访问同一个共享的变量，并且其中某个线程对该变量进行了写操作，其他线程可能会在读取该变量时得到过期或不一致的值。这是因为编译器为了提高性能可能会对变量进行优化，例如将变量存储在寄存器中或进行缓存优化。而
    // volatile
    // 关键字可以告诉编译器不要对该变量进行优化，确保每次访问都从内存中读取最新的值。
    // 在上述代码中，free_list 数组可能会被多个线程同时访问和修改，因此使用
    // volatile 关键字可以确保对其读写操作的可见性，避免出现意外的行为或错误。
    // 需要注意的是，volatile
    // 关键字并不能解决多线程并发访问的所有问题，它仅仅保证了对变量的读写操作的可见性，但并不能保证原子性或顺序性。对于多线程编程，更复杂的同步机制（例如互斥锁、原子操作等）可能还需要结合使用，以确保线程安全性。

    // 自由链表的数组
    static free_list_node* volatile free_list_[free_list_nums];
    //[start,end]表示空闲的内存区域的起始和结束位置
    static char* chuck_start_;
    static char* chuck_end_;

    // chuck_size_表示空闲的内存区域的大小
    // 为啥还要用个单独的变量表示，用 chuck_end_ - chuck_start_  不是一样的吗？
    // chuck_end_ - chuck_start_ 的结果将是 ptrdiff_t
    // 类型，即表示指针之间差异的整数类型。 ptrdiff_t和size_t
    // 不是固定的类型，它可能在不同的平台或编译器中具有不同的大小.
    //  在32位平台上，size_t 和 ptrdiff_t 的大小通常为4字节（32位）。
    //  在64位平台上，size_t 和 ptrdiff_t 的大小通常为8字节（64位）。
    //  ptrdiff_t 用于表示指针之间的差异，是有符号整数类型。
    //  size_t 用于表示对象大小或数量，是无符号整数类型。
    static size_t chuck_size_;

private:
    // constexpr
    // 修饰一个函数时，它表示该函数可以在编译时被求值，从而允许在编译期间进行常量表达式的计算。
    // 对于修饰为 constexpr
    // 的函数，编译器会尝试在编译时对其进行求值，而不是在运行时。这样可以在编译期间完成一些计算，并将其结果作为常量使用，从而提供更高效的代码和更好的优化机会。
    //  noexcept表示该函数不会抛出异常，编译器你不要给我添加一些处理异常的代码(运行有额外的损耗)

    // 表示在编译时计算出 bytes向上舍入(为8的整数倍)的结果
    static constexpr size_t round_up(size_t bytes) noexcept { return (bytes + align - 1) & ~(align - 1); }
    // 根据大小计算对应是在哪条自由链表区块
    static constexpr size_t free_list_index(size_t bytes) noexcept { return (bytes + align - 1) / align - 1; }

private:
    // 从空闲区域里面去找内存，去填充对应size的自由链表
    static void* refill(size_t);
    static char* chunk_alloc(size_t, size_t&);

private:
    // Todo:why multi version?
    // 重载operaor new，提供抛出异常和不抛出异常的两个版本
    void* operator new(size_t size) { return ::operator new(size); }
    void* operator new[](size_t size) { return ::operator new[](size); }

    void operator delete(void* p) { return ::operator delete(p); }
    void operator delete[](void* p) { return ::operator delete[](p); }
    // placement delete version?
    void operator delete(void* p, size_t size) { return ::operator delete(p, size); }
    void operator delete[](void* p, size_t size) { return ::operator delete[](p, size); }

public:
    constexpr memory_pool() noexcept = default;
    constexpr memory_pool(const memory_pool&) noexcept = default;
    constexpr memory_pool(memory_pool&&) noexcept = default;
    ~memory_pool() noexcept = default;
};

/////////////////////////////////////
// 初始化类的静态成员
// 小技巧：
// 1.先把类的成员变量声明给抄下来，把static关键字给移除；
// 2.给类内定义的成员free_list_和类型free_list_node给加上类的namespace"memory_pool"
// 3.所有出现类名称memory_pool都得用memory_pool<Align,
// MaxBytes>来替代表示此模板类 4.typename
// 关键字是必需的(否则可能编译器会认不出来而报错)，因为 memory_pool<Align,
// MaxBytes>::free_list_node 是一个嵌套在 memory_pool<Align, MaxBytes>
// 类中的类型名称。 在这种情况下，编译器需要 typename
// 关键字来指示该名称是一个类型。

template <size_t Align, size_t MaxBytes>
typename memory_pool<Align, MaxBytes>::free_list_node* volatile memory_pool<Align, MaxBytes>::free_list_[free_list_nums]
    = {};

template <size_t Align, size_t MaxBytes>
char* memory_pool<Align, MaxBytes>::memory_pool::chuck_start_ = { nullptr };

template <size_t Align, size_t MaxBytes>
char* memory_pool<Align, MaxBytes>::chuck_end_ = { nullptr };

template <size_t Align, size_t MaxBytes>
size_t memory_pool<Align, MaxBytes>::chuck_size_ = { 0 };

/////////////////////////////////////

/////////////////////////////////////
// 类的成员函数实现
template <size_t Align, size_t MaxBytes> // 类的模板
template <bool NoThrowExep> // 函数模板 参数NoThrowExep
void* memory_pool<Align, MaxBytes>::allocate(size_t size)
{
    if (size > max_bytes) {
        // 不需要走内存池
        // if constexpr (c++17)用于在编译时进行条件判断,类似于条件编译的效果
        if constexpr (NoThrowExep) {
            // Todo:实现一个不抛异常的版本的operator new
            return memory_pool::operator new(size);
        } else {
            // 手动声明是调用的memory_pool::的operator
            // new,否则会被编译器认为是全局的operator new operator
            // new看错一个函数名称整体
            return memory_pool::operator new(size);
        }
    } else {
        // 从自由链表里面尝试去找，看看是否能从自由链表里面去分配

        // 先获取满足其大小的自由链表数组的哪个元素
        // free_list是一个memory_pool<Align, MaxBytes>::free_list_node* 指针数组
        // 获取对应的这个自由链表的首地址
        //*(free_list + free_list_index(size)) 等价于free_list[idx]
        // 获取其在指针数组的idx个元素
        auto& first { *(free_list_ + free_list_index(size)) };
        if (not first) {
            // 此自由链表空空如也
            return refill(round_up(size));
        } else {
            // 列表初始化,
            // 例如初始化容器或初始化自定义类型时，可以利用列表初始化的特性。
            // 它可以执行隐式类型转换和窄化检查，并且支持初始化列表的使用。
            auto result_node { first }; // 分配的内存块的指针。
            // auto result_node = first;//拷贝初始化

            first = result_node->next_; // 更新自由链表头指针，这里相当于arr[idx]去重新赋值

            return result_node;
        }
    }
}

template <size_t Align, size_t MaxBytes> // 类的模板
void memory_pool<Align, MaxBytes>::deallocate(void* p, size_t size) noexcept
{
    if (size > max_bytes) {
        memory_pool::operator delete(p, size);
        return;
    } else {
        // 归还给自由链表
        auto& first { *(free_list_ + free_list_index(size)) };

        // 连起来
        auto return_node { static_cast<free_list_node*>(p) };
        return_node->next_ = first;

        // 更新自由链表中对应内存大小的第一个节点为
        // return_node，使其成为新的头节点。
        first = return_node;
    }
}

// 从空闲区域里面去找内存，去填充对应size的自由链表
template <size_t Align, size_t MaxBytes> // 类的模板
void* memory_pool<Align, MaxBytes>::refill(size_t size)
{
    size_t nodes_num { 16 }; //{}防止隐式类型转换
    auto chuck = chunk_alloc(size, nodes_num);
    if (nodes_num == 1) {
        // 表示只获得了一个内存块,直接分配出去
        return chuck;
    }

    // 获取了最少1个内存块，除了要返回分配出去的，剩余的得放到自由链表里面去管理起来

    // compile error C2440: “static_cast”: 无法从“char
    // *”转换为“memory_pool<8,128>::free_list_node *” static_cast
    // 在编译器进行类型检查时会尽可能保证类型转换的安全性。它会在编译时对类型进行检查
    // case1:没有直接的转换关系：例如将指针类型转换为不相关的指针类型
    // case2:不安全的类型转换
    //  类型不匹配：不安全的类型转换可能导致目标类型与源类型之间的不匹配。例如，将一个指向基类的指针转换为指向派生类的指针，但实际上对象的类型并不是该派生类，这将导致未定义的行为。
    //  截断或丢失信息：类型转换可能导致数据的截断或丢失信息。例如，将一个大范围的整数转换为一个较小范围的整数类型，可能导致数据溢出或丢失精度。
    //  安全检查问题：某些类型转换可能会绕过编译器的安全检查机制，导致潜在的错误。例如，将一个指向非常量对象的指针转换为指向常量对象的指针，这可能导致在修改常量对象时引发未定义的行为。
    //  不兼容的类型：不安全的类型转换可能涉及不兼容的类型之间的转换，例如将指针类型转换为非相关的指针类型。这种转换可能会导致程序在运行时崩溃或产生难以预测的结果。
    // auto current_node = static_cast<free_list_node*>(chuck + size);
    auto current_node = reinterpret_cast<free_list_node*>(chuck + size);

    auto& first { *(free_list_ + free_list_index(size)) };
    first = current_node; // 自由链表数组

    // 把剩余的内存给组装成自由链表的形式
    size_t less_nodes_num = nodes_num - 1;
    while (less_nodes_num > 0) {
        auto next_node { reinterpret_cast<free_list_node*>(reinterpret_cast<char*>(current_node) + size) };
        current_node->next_ = next_node;
        current_node = next_node;

        --less_nodes_num;
    }
    current_node->next_ = nullptr;

    return chuck;
}

// 期望分配nodes块size大小的内存，nodes也作为出参表示实际分配了的内存块数目；返回值表示分配的内存地址
template <size_t Align, size_t MaxBytes> // 类的模板
char* memory_pool<Align, MaxBytes>::chunk_alloc(size_t size, size_t& nodes_num)
{
    auto need_total_size { size * nodes_num };
    auto chuck_left_size { chuck_end_ - chuck_start_ };
    // error:
    // auto chuck_left_size{chuck_size_};

    if (chuck_left_size >= need_total_size) {
        // chuck空闲区域可以满足分配nodes_num块的需求
        auto result { chuck_start_ };
        chuck_start_ += need_total_size;
        return result;
    } else if (chuck_left_size >= size) {
        // chuck空闲区域只不足以满足分配所有nodes_num块的需求，但是至少满足至少一块的需求
        nodes_num = chuck_left_size / size;
        auto result { chuck_start_ };
        chuck_start_ += (nodes_num * size);
        return result;
    }

    // 走到这里说明:剩余的字节数不足以满足需要分配的单个内存块的字节数

    // 把剩余的chuck空闲区域的内存给分配出去
    if (chuck_left_size > 0) {
        // 現在我們的配置要求是 32 位元組的記憶體,
        // 雖然記憶池huck块無法提供32位大小的数据,
        // 但是記憶池的chuck块可能還剩下了一些 (例如還剩下 16 位元組).
        // 那麼根據記憶池中還剩下的記憶體大小,
        // 我們直接將其放入自由串列中負責管理對應大小記憶體區塊的前項連結串列中就可以了
        auto& first { *(free_list_ + free_list_index(size)) };
        // 连起来
        auto new_first { reinterpret_cast<free_list_node*>(chuck_start_) };
        new_first->next_ = first;
        // 更新头节点
        first = new_first;
    }

    // 尝试向操作系统去申请
    // 一般來說, 擴容的時候, 我們向作業系統要求的大小是 =
    // 之前向作業系統要求配置的大小 × 2 + 本次要求配置的大小
    // 本次要求配置的大小又可以表示為 =
    // 用戶向記憶池請求的大小×提前配置給自由串列的記憶體區塊數量,
    // 用戶向記憶池請求的大小必須已經調整為 8 的倍數.
    auto need_allocate_size_from_system { 2 * need_total_size + round_up(chuck_size_ >> 4) };
    // chuck_size_ >> 4 相当于除以 16

    // Todo:后面改为申请失败也不抛异常的版本
    chuck_start_ = static_cast<char*>(memory_pool::operator new(need_allocate_size_from_system));

    if (not chuck_start_) {
        // 第一次从系统里面申请失败了
        // 考慮的情況就是如果作業系統也不能向我們提供更多的記憶體,
        // 也就是電腦的記憶體空間已經用盡. 我們首先應該考慮的是,
        // 是否可以榨乾自由串列中可用的記憶體區塊, 而不是急著擲出例外情況 (C++
        // 的做法) 或者回傳一個空指標 (C 的做法).
        //  如果當前記憶池使用者向記憶池申請配置 24 位元組大小的記憶體,
        //  而在向作業系統申請擴容的時候,
        //  作業系統告訴我們已經沒有更多可用的記憶體了. 這個時候,
        //  我們可以去尋訪自由串列, 看看還有沒有空閒區塊可以拆出來使用.
        //  那些在自由串列裏面管理記憶體區塊的前向連結串列中, 管理著比 24
        //  位元組還小那部分前項連結串列可以直接跳過.
        // 假設現在管理著  k 位元組 (k>24)
        // 記憶體區塊的前向連結串列中還有空閒的記憶體區塊, 那麼就拿出一塊來使用.
        // 我們需要的是 24 位元組, 於是這一小塊記憶體區塊又被分為兩個部分 :
        // 一部分的大小為 24 位元組, 另一部分的大小為 k−24 位元組.
        // 大小為 24 位元組的那部分記憶體可以直接回傳給記憶池使用者,
        // 大小為 k−24 位元組的記憶體區塊需要放入自由串列中專門管理大小為 k−24
        // 位元組記憶體區塊的前項連結串列中.

        // 遍历比size还大的自由链表，看看能不能找一个大块来拆开来用。
        for (auto i { size }; i < max_bytes; i += align) {
            auto& first { *(free_list_ + free_list_index(i)) };
            if (first) {
                // 表明该大小的自由链表有空闲
                // 从该自由链表里面拿一块出去，作为chuck空闲区域
                chuck_start_ = reinterpret_cast<char*>(first);
                chuck_end_ = chuck_start_ + i;
                first = first->next_;

                // 此时chuck空闲区域肯定已经满足了至少能分配一块size大小的内存区域了
                return chunk_alloc(size, nodes_num);
            }
        }

        // 从后续的自由链表中找不到更大的可用区域
        // 這個時候我們才告訴記憶池使用者 : 記憶池和作業系統都不能提供更多記憶體了.
        chuck_end_ = nullptr;

        // 向操作系统reTry一遍，尝试向操作系统申请，申请失败则抛出异常
        chuck_start_ = static_cast<char*>(memory_pool::operator new(need_allocate_size_from_system));
        // 抛异常，下面的代码就不会走到了
        assert(chuck_start_);
        chuck_size_ += need_allocate_size_from_system;
        chuck_end_ = chuck_start_ += chuck_size_;

        // 此时chuck空闲区域肯定已经满足了至少能分配一块size大小的内存区域了
        return chunk_alloc(size, nodes_num);
    } else {
        // 申请到了内存
        // 更新chunk空闲区域的起始和结束位置。
        chuck_size_ += need_allocate_size_from_system;
        chuck_end_ = chuck_start_ + need_allocate_size_from_system;
        return chunk_alloc(size, nodes_num);
    }
}

/////////////////////////////////////
