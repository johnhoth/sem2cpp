#include <iostream>
#include <vector>


template <typename T>
class Deque {
private:
    const static int BUCKET_SIZE = 11;
public:

    template <bool isConst>
    class common_iterator {
    private:
        friend class Deque<T>;

        using type_of_data = std::conditional_t<isConst, const std::vector<T*>&, std::vector<T*>&>;

        type_of_data data;
        int i, j;

        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::conditional_t<isConst, const T, T>;
        using pointer = std::conditional_t<isConst, const T*, T*>;
        using reference = std::conditional_t<isConst, const T&, T&>;

        pointer get_this() const {
            return &data[i][j];
        }


    public:
        common_iterator(const common_iterator& it) : data(it.data), i(it.i), j(it.j) {}

        template <bool anotherConst>
        common_iterator(const common_iterator<anotherConst>& it) : data(it.data), i(it.i), j(it.j) {}
        common_iterator(std::vector<T*>& _data, int _i, int _j) : data(_data), i(_i), j(_j) {}

        common_iterator(type_of_data _data, const common_iterator& it) : data(_data), i(it.i), j(it.j) {}

        common_iterator& operator=(const common_iterator& it) {
            data = it.data;
            i = it.i;
            j = it.j;
            return *this;
        }


        common_iterator& operator++() {
            if (j < BUCKET_SIZE - 1) {
                j++;
            } else {
                i++;
                j = 0;
            }
            return *this;
        }
        common_iterator& operator--() {
            if (j > 0) {
                j--;
            } else {
                i--;
                j = BUCKET_SIZE - 1;
            }
            return *this;
        }

        common_iterator operator++(int) {
            common_iterator copy(*this);
            ++(*this);
            return copy;
        }

        common_iterator operator--(int) {
            common_iterator copy(*this);
            --(*this);
            return copy;
        }

        common_iterator& operator+=(int step) {
            if (step < 0) {
                i -= (-step) / BUCKET_SIZE;
                j -= (-step) % BUCKET_SIZE;
                while (j < 0) {
                    i -= 1;
                    j += BUCKET_SIZE;
                }
            } else {
                i += step / BUCKET_SIZE;
                j += step % BUCKET_SIZE;
                while (j >= BUCKET_SIZE) {
                    i += 1;
                    j -= BUCKET_SIZE;
                }
            }
            return *this;
        }

        common_iterator& operator-=(int step) {
            operator+=(-step);
            return *this;
        }

        common_iterator operator+(int go) const{
            common_iterator copy(*this);
            copy += go;
            return copy;
        }

        common_iterator operator-(int go) const{
            common_iterator copy(*this);
            copy -= go;
            return copy;
        }

        bool operator<(const common_iterator& it) const {
            if (i == it.i)
                return j < it.j;
            return i < it.i;
        }
        bool operator>(const common_iterator& it) const {
            if (i == it.i)
                return j > it.j;
            return i > it.i;
        }
        bool operator==(const common_iterator& it) const {
            return (i == it.i && j == it.j);
        }
        bool operator<=(const common_iterator& it) const {
            return operator<(it) || operator==(it);
        }
        bool operator>=(const common_iterator& it) const {
            return operator>(it) || operator==(it);
        }
        bool operator!=(const common_iterator& it) const {
            return !operator==(it);
        }

        int operator-(const common_iterator& it) const {
            return (i - it.i) * BUCKET_SIZE + (j - it.j);
        }

        reference operator*() {
            return data[i][j];
        }

        const T& operator*() const {
            return data[i][j];
        }

        pointer operator->() {
            return &data[i][j];
        }

        const T* operator->() const {
            return &data[i][j];
        }


    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;


private:
    std::vector<T*> data;
    iterator begin_element;
    iterator end_element;

    T* make_bucket() {
        return reinterpret_cast<T*>(new int8_t[BUCKET_SIZE * sizeof(T)]);
    }
    void clear_bucket(T* bucket) {
        delete[] reinterpret_cast<int8_t*>(bucket);
    }

    void expand_data() {
        std::vector<T*> copy(data);
        data.resize(data.size() * 2);
        for (size_t i = 0; i < data.size() / 4; ++i) {
            data[i] = make_bucket();
        }
        for (size_t i = 0; i < copy.size(); ++i) {
            data[i + data.size() / 4] = copy[i];
        }
        for (size_t i = data.size() / 4 + copy.size(); i < data.size(); ++i) {
            data[i] = make_bucket();
        }
        begin_element.i = begin_element.i + data.size() / 4;
        end_element.i = end_element.i + data.size() / 4;
    }

public:

    Deque() : data(1), begin_element(data, 0, BUCKET_SIZE / 2), end_element(data, 0, BUCKET_SIZE / 2) {
        data[0] = make_bucket();
    }

    Deque(const Deque& source) : data(source.data.size()), begin_element(data, source.begin_element), end_element(data, source.end_element) {
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = make_bucket();
        }
        iterator it = begin_element, from = source.begin_element;
        try {
            for (; it != end_element; it++, from++) {
                new(it.get_this()) T(*from);
            }
        } catch (...) {
            for (auto jt = begin_element; jt < it; ++jt) {
                jt.get_this()->~T();
            }
            for (size_t i = 0; i < data.size(); ++i) {
                clear_bucket(data[i]);
            }
            throw;
        }
    }

    Deque(int n, const T& value = T()) : data((n / BUCKET_SIZE + 1) * 2), begin_element(data, data.size() / 2, 0), end_element(data, data.size() / 2, 0) {
        if (n == 0) {
            data.clear();
            data.push_back(make_bucket());
        } else {
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] = make_bucket();
            }
            try {
                while (end_element - begin_element < n) {
                    new(end_element.get_this()) T(value);
                    ++end_element;
                }
            } catch (...) {
                for (auto it = begin_element; it < end_element; ++it) {
                    it.get_this()->~T();
                }
                for (size_t i = 0; i < data.size(); ++i) {
                    clear_bucket(data[i]);
                }
                throw;
            }
        }
    }

    ~Deque() {
        for (auto it = begin_element; it != end_element; ++it) {
            it->~T();
        }
        for (size_t i = 0; i < data.size(); ++i) {
            clear_bucket(data[i]);
        }
    }

    Deque& operator=(const Deque& new_value) {
        std::vector<T*> data_copy(data);
        iterator begin_element_copy(begin_element);
        iterator end_element_copy(end_element);

        for (iterator it = begin_element; it < end_element; ++it) {
            it.get_this()->~T();
        }
        for (size_t i = 0; i < data.size(); ++i) {
            clear_bucket(data[i]);
        }
        data.resize(new_value.data.size(), nullptr);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = make_bucket();
        }
        begin_element = iterator(data, new_value.begin_element);
        end_element = iterator(data, new_value.end_element);
        iterator it(begin_element);
        try {
            while (it < end_element) {
                new(it.get_this()) T(*(const_iterator(new_value.data, it)));
                ++it;
            }
        } catch (...) {
            for (iterator jt = begin_element; jt < it; ++jt) {
                jt.get_this()->~T();
            }
            for (size_t i = 0; i < data.size(); ++i) {
                clear_bucket(data[i]);
            }
            data = data_copy;
            begin_element = begin_element_copy;
            end_element = end_element_copy;
            throw;
        }
        return *this;
    }

    size_t size() const {
        return end_element - begin_element;
    }

    T& operator[](int index) {
        return *(begin_element + index);
    }
    const T& operator[](int index) const {
        return *(begin_element + index);
    }

    T& at(int index) {
        iterator now = begin_element + index;
        if (begin_element <= now && now < end_element) {
            return *now;
        } else {
            throw std::out_of_range("Deque: index out of range");
        }
    }
    const T& at(int index) const {
        iterator now = begin_element + index;
        if (begin_element <= now && now < end_element) {
            return *now;
        } else {
            throw std::out_of_range("Deque: index out of range");
        }
    }

    void push_back(const T& value) {
        ++end_element;
        if (end_element.i == static_cast<int>(data.size())) {
            expand_data();
        }
        try {
            iterator tmp = end_element;
            tmp--;
            new(tmp.get_this()) T(value);

        } catch (...) {
            --end_element;
            throw;
        }
    }

    void pop_back() {
        --end_element;
        end_element.get_this()->~T();
    }

    void push_front(const T& value) {
        --begin_element;
        if (begin_element.i <= 0) {
            expand_data();
        }
        try {
            new(begin_element.get_this()) T(value);
        } catch (...) {
            ++begin_element;
            throw;
        }
    }

    void pop_front() {
        begin_element.get_this()->~T();
        begin_element++;
    }

    void insert(const iterator& place, const T& value) {
        ++end_element;
        if (end_element.i == static_cast<int>(data.size())) {
            expand_data();
        }
        for (iterator from = end_element - 1; from > place; --from) {
            iterator to = from - 1;
            new(from.get_this()) T(*to.get_this());
        }
        new(place.get_this()) T(value);
    }

    void erase(const iterator& place) {
        for (iterator from = place; from < end_element; ++from) {
            iterator to = from + 1;
            new(from.get_this()) T(*to.get_this());
        }
        --end_element;
    }

    iterator begin() {
        return begin_element;
    }
    const_iterator begin() const {
        return const_iterator(begin_element);
    }
    const_iterator cbegin() const {
        return const_iterator(begin_element);
    }
    iterator end() {
        return end_element;
    }
    const_iterator end() const {
        return const_iterator(end_element);
    }
    const_iterator cend() const {
        return const_iterator(end_element);
    }
    reverse_iterator rbegin() {
        return reverse_iterator(end_element);
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }
    reverse_iterator rend() {
        return reverse_iterator(begin_element - 1);
    }
    const_reverse_iterator rend() const {
        return const_reverse_iterator(const_iterator(begin_element - 1));
    }
    const_reverse_iterator crend() const {
        return const_reverse_iterator(const_iterator(begin_element - 1));
    }
};
