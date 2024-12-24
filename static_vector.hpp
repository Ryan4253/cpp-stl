#include <cstddef>
#include <concepts>
#include <utility>
#include <array>
#include <cstdint>
#include <limits>
#include <iterator>

template<typename T, std::uint8_t Capacity = 64>
class aligned_storage{
    public:
    const T* data() const{
        return reinterpret_cast<const T*>(m_data);
    }

    private:
    alignas(T) std::byte m_data[Capacity * sizeof(T)];
};

template<typename T, std::size_t Capacity = 64>
class static_vector{
    public:
    static_assert(Capacity != 0, "static_vector cannot have 0 capacity");

    /* CONSTRUCTOR */
    constexpr static_vector() : m_size(0){}

    explicit constexpr static_vector(std::size_t count, const T& value = T()) requires std::copy_constructible<T> 
        : m_size(0){
        resize(count, value);
    }


    /* SPECIAL MEMBER FUNCTIONS */
    constexpr ~static_vector(){
        clear();
    }

    constexpr static_vector(const static_vector& other) requires std::copy_constructible<T> : m_size(0){
        for(std::size_t i = 0; i < other.size(); i++){
            push_back(other[i]);
        }
    }

    constexpr static_vector(static_vector&& other) requires std::move_constructible<T> : m_size(0){
        for(std::size_t i = 0; i < other.size(); i++){
            push_back(std::move(other[i]));
        }
    }

    constexpr static_vector& operator=(const static_vector& other) requires std::copy_constructible<T>{
        if(this != std::addressof(other)){
            clear();

            for(std::size_t i = 0; i < other.size(); i++){
                push_back(other[i]);
            }
        }

        return *this;
    }

    constexpr static_vector& operator=(static_vector&& other) requires std::move_constructible<T>{
        clear();

        for(std::size_t i = 0; i < other.size(); i++){
            push_back(std::move(other[i]));
        }

        return *this;
    }


    /* ELEMENT ACCESS */
    constexpr T& at(std::size_t index){
        if(index >= size()){
            throw std::out_of_range("Index out of range");
        }

        return data()[index];
    }

    constexpr const T& at(std::size_t index) const{
        if(index >= size()){
            throw std::out_of_range("Index out of range");
        }

        return data()[index];
    }

    constexpr T& operator[](std::size_t index){
        assert(index < size());
        return data()[index];
    }

    constexpr const T& operator[](std::size_t index) const{
        assert(index < size());
        return data()[index];
    }

    constexpr T& front(){
        assert(!empty());
        return data()[0];
    }

    constexpr const T& front() const{
        assert(!empty());
        return data()[0];
    }

    constexpr T& back(){
        assert(!empty());
        return data()[size() - 1];
    }

    constexpr const T& back() const{
        assert(!empty());
        return data()[size() - 1];
    }

    constexpr const T* data(){
        return m_data.data();
    }


    /* ITERATORS */
    constexpr auto begin(){
        return data();
    }

    constexpr auto begin() const{
        return data();
    }

    constexpr auto cbegin() const{
        return data();
    }

    constexpr auto end(){
        return data() + size();
    }

    constexpr auto end() const{
        return data() + size();
    }

    constexpr auto cend() const{
        return data() + size();
    }

    constexpr auto rbegin(){
        return std::make_reverse_iterator(end());
    }

    constexpr auto rbegin() const{
        return std::make_reverse_iterator(end());
    }

    constexpr auto crbegin() const{
        return std::make_reverse_iterator(cend());
    }

    constexpr auto rend(){
        return std::make_reverse_iterator(begin());
    }

    constexpr auto rend() const{
        return std::make_reverse_iterator(begin());
    }

    constexpr auto crend() const{
        return std::make_reverse_iterator(cbegin());
    }

    /* CAPACITY */
    constexpr bool empty() const{
        return size() == 0;
    }

    constexpr std::size_t size() const{
        return m_size;
    }

    constexpr std::size_t max_size() const{
        return std::numeric_limits<std::uint8_t>::max();
    }

    constexpr std::size_t capacity() const{
        return Capacity;
    }


    /* MODIFIERS */
    constexpr void clear(){
        while(!empty()){
            pop_back();
        }
    }

    constexpr void push_back(const T& value) requires std::copy_constructible<T>{
        assert(size() < capacity());
        emplace_back(value);
    }

    constexpr void push_back(T&& value) requires std::move_constructible<T>{
        assert(size() < capacity());
        emplace_back(std::move(value));
    }

    template<typename... Args>
    constexpr void emplace_back(Args&&... args){
        assert(size() < capacity());
        std::construct_at(data() + size(), std::forward<Args>(args)...);
        m_size++;
    }

    constexpr void pop_back(){
        assert(!empty());
        std::destroy_at(data()[size() - 1]);
        m_size--;
    }

    constexpr void resize(std::size_t count) requires std::default_initializable<T>{
        assert(count <= capacity());
        while(m_size > count){
            pop_back();
        }

        while(m_size < count){
            push_back(T());
        }
    }

    constexpr void resize(std::size_t count, const T& value) requires std::copy_constructible<T>{
        assert(count <= capacity());
        while(m_size > count){
            pop_back();
        }

        while(m_size < count){
            push_back(value);
        }
    }

    private:
    constexpr static auto is_trivial = 
        std::is_trivially_default_constructible_v<T> && 
        std::is_trivially_destructible_v<T>;

    using storage_type = std::conditional_t<
        is_trivial, 
        std::array<T, Capacity>, 
        aligned_storage<T, Capacity>
    >;

    storage_type m_data;
    std::uint8_t m_size;
};
