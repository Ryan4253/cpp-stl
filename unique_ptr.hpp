#include <type_traits>
#include <concepts>

template<typename Deleter, typename T>
concept IsDeleter = requires(T* ptr, Deleter deleter){
    { deleter(ptr) } -> std::same_as<void>;
};

template<typename T>
class default_delete{
    public:
    void operator()(T* ptr) const{
        delete ptr;
    }
};

template<typename T>
class default_delete<T[]>{
    public:
    void operator()(T* ptr) const{
        delete[] ptr;
    }
};

template<typename T, typename Deleter = default_delete<T>> requires IsDeleter<Deleter, T>
class unique_ptr{
    public:
    /* MEMBER TYPES */
    using pointer = T*;
    using element_type = T;
    using deleter_type = Deleter;


    /* CONSTRUCTORS */
    unique_ptr() 
        : unique_ptr(nullptr){}

    unique_ptr(std::nullptr_t) 
        : m_ptr(nullptr){}

    explicit unique_ptr(pointer ptr) 
        : m_ptr(ptr), m_deleter(){}

    
    /* SPECIAL MEMBER FUNCTIONS*/
    ~unique_ptr(){
        reset();
    }

    unique_ptr(const unique_ptr& ptr) = delete;
    unique_ptr& operator=(const unique_ptr& ptr) = delete;

    unique_ptr(unique_ptr&& ptr)
        : m_ptr(ptr.release()), m_deleter(std::move(ptr.get_deleter())){}

    unique_ptr& operator=(unique_ptr&& rhs){
        if(this != &rhs){
            reset(rhs.release());
            m_deleter = std::move(rhs.get_deleter());
            rhs.m_ptr = nullptr;
        }

        return *this;
    }


    /* MODIFIERS */
    [[nodiscard]] pointer release(){
        const pointer temp = get();
        m_ptr = nullptr;
        return temp;
    }

    void reset(pointer ptr = pointer()){
        if(m_ptr){
            get_deleter()(get());
        }

        m_ptr = ptr;
    }

    void swap(unique_ptr& rhs){
        std::swap(m_ptr, rhs.m_ptr);
        std::swap(m_deleter, rhs.m_deleter);
    }


    /* OBSERVERS */
    [[nodiscard]] pointer get() const{
        return m_ptr;
    }

    [[nodiscard]] deleter_type& get_deleter(){
        return m_deleter;
    }

    [[nodiscard]] const deleter_type& get_deleter() const{
        return m_deleter;
    }

    [[nodiscard]] explicit operator bool() const{
        return m_ptr != nullptr;
    }


    /* OPERATORS */
    [[nodiscard]] std::add_lvalue_reference_t<element_type> operator*() const{
        return *get();
    }

    [[nodiscard]] pointer operator->() const{
        return get();
    }

    private:
    pointer m_ptr;
    deleter_type m_deleter;
};

template<typename T, typename Deleter>
class unique_ptr<T[], Deleter>{
    public:
    T& operator[](std::size_t index) const{
        return this->get()[index];
    }
};

int main(){
    unique_ptr<int> ptr(new int(5));

    unique_ptr<int[]> ptr_arr(new int[5]);
    return 0;
}