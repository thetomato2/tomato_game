namespace tom
{

struct Arena
{
    szt size;
    u8* base;
    szt used;
};

inline Arena init_arena (void* base, szt size)
{
    Arena result;

    result.size = size;
    result.base = (byt*)base;
    result.used = 0;
    
    return result;
}

inline void* push_size(Arena* arena, szt size)
{
    Assert((arena->used + size) <= arena->size);
    void* result = arena->base + arena->used;
    arena->used += size;

    return result;
}

inline void zero_size(void* ptr, szt size)
{
    memset(ptr, 0, size);
}

template<typename T>
inline T* push_struct(Arena* arena)
{
    return (T*)push_size(arena, sizeof(T));
}

template<typename T>
inline T* push_array(Arena* arena, szt count)
{
    return (T*)push_size(arena, sizeof(T) * count);
}

template<typename T>
inline void zero_struct(T* ptr)
{
    zero_size(ptr, sizeof(T));
}

template<typename T>
inline void zero_struct(T ptr)
{
    zero_size(&ptr, sizeof(T));
}

template<typename T>
inline void zero_array(T* ptr, szt count)
{
    zero_size(ptr, sizeof(T) * count);
}

// template<typename T>
// function void zero_array(T* ptr)
// {
//     zero_size(ptr, CountOf(ptr));
// }

#define ZeroArray(arr) zero_array(arr, CountOf(arr))

// #define PUSH_STRUCT(arena, type)       (type*)push_size(arena, sizeof(type))
// #define PUSH_ARRAY(arena, count, type) (type*)push_size(arena, (count * sizeof(type)))
// #define ZERO_STRUCT(inst)              zero_size(&(inst), sizeof(inst))

inline void* plat_malloc(szt size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

template<typename T>
inline T* plat_malloc(szt count)
{
    return (T*)HeapAlloc(GetProcessHeap(), 0, count * sizeof(T));
}

inline void plat_free(void* ptr)
{
    HeapFree(GetProcessHeap(), 0, ptr);
}

// NOTE: SIMPLE implmentation that will plat_free() on destruction
// I mostly use it for c-string buffers.
template<typename T>
struct ScopedPtr
{
    ScopedPtr() :
        _ptr(nullptr)
    {
    }

    ScopedPtr(T* ptr) :
        _ptr(ptr)
    {
    }

    ~ScopedPtr() { release(); }

    // no copy and no move, this is not a unique_ptr
    ScopedPtr(const ScopedPtr&)            = delete;
    ScopedPtr& operator=(const ScopedPtr&) = delete;
    ScopedPtr(ScopedPtr&&)                 = delete;
    ScopedPtr& operator=(ScopedPtr&& move) = delete;

    void release()
    {
        if (_ptr != nullptr) {
            plat_free(_ptr);
            _ptr = nullptr;
        }
    }

    T* get() { return _ptr; }
    T* get() const { return _ptr; }

    operator bool() const { return _ptr != nullptr; }
    T& operator*() { return *_ptr; }
    T* operator->() { return _ptr; }
    T& operator[](szt i) { return _ptr[i]; }
    const T& operator[](szt i) const { return _ptr[i]; }

private:
    T* _ptr;
};

template<typename T>
class UniquePtr
{
public:
    UniquePtr() :
        _ptr(nullptr)
    {
    }
    // Explicit constructor
    explicit UniquePtr(T* ptr) :
        _ptr(ptr)
    {
    }
    ~UniquePtr() { plat_free(_ptr); }

    // Constructor/Assignment that binds to nullptr
    // This makes usage with nullptr cleaner
    UniquePtr(std::nullptr_t) :
        _ptr(nullptr)
    {
    }

    UniquePtr& operator=(std::nullptr_t)
    {
        reset();
        return *this;
    }

    // Constructor/Assignment that allows move semantics
    UniquePtr(UniquePtr&& move) noexcept :
        _ptr(nullptr)
    {
        move.swap(*this);
    }

    UniquePtr& operator=(UniquePtr&& move) noexcept
    {
        move.swap(*this);
        return *this;
    }

    // Constructor/Assignment for use with types derived from T
    template<typename U>
    UniquePtr(UniquePtr<U>&& move)
    {
        UniquePtr<T> tmp(move.release());
        tmp.swap(*this);
    }

    template<typename U>
    UniquePtr& operator=(UniquePtr<U>&& move)
    {
        UniquePtr<T> tmp(move.release());
        tmp.swap(*this);
        return *this;
    }

    // Remove compiler generated copy semantics.
    UniquePtr(UniquePtr const&)            = delete;
    UniquePtr& operator=(UniquePtr const&) = delete;

    // Const correct access owned object
    T* operator->() const { return _ptr; }
    T& operator*() const { return *_ptr; }

    // Access to smart pointer state
    T* get() const { return _ptr; }
    explicit operator bool() const { return _ptr; }

    // Modify object state
    T* release() noexcept
    {
        T* result = nullptr;
        std::swap(result, _ptr);
        return result;
    }

    void swap(UniquePtr& src) noexcept { std::swap(_ptr, src._ptr); }
    void reset()
    {
        T* tmp = release();
        delete tmp;
    }

private:
    T* _ptr;
};

template<typename T>
void swap(UniquePtr<T>& lhs, UniquePtr<T>& rhs)
{
    lhs.swap(rhs);
}

}  // namespace tom
