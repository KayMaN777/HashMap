#include <iostream>
#include <vector>
#include <initializer_list>
#include <stdexcept>
#include <list>

std::hash<size_t> HASH;

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {

public:
    using pair_list = std::list<std::pair<const KeyType, ValueType>>;
    using iterator = typename pair_list::iterator;
    using const_iterator = typename pair_list::const_iterator;

private:
    struct bucket {
        iterator begin;
        iterator end;
        bool idle = true;
    };
    size_t _count = 0;
    pair_list data;
    Hash hash_f;
    size_t cur_size = 16;
    std::vector<bucket> buckets = std::vector<bucket>(cur_size);
    std::vector<size_t> buckets_size = std::vector<size_t>(cur_size, 0);

public:
    Hash hash_function() const {
        return hash_f;
    }

    std::pair<size_t, size_t> pos(KeyType key) {
        size_t h1 = hash_f(key) % cur_size;
        size_t h2 = HASH(hash_f(key)) % cur_size;
        return {h1, h2};
    }

    iterator begin() {
        return data.begin();
    }

    const_iterator begin() const {
        return data.begin();
    }

    iterator end() {
        return data.end();
    }

    const_iterator end() const {
        return data.end();
    }

    size_t size() const {
        return _count;
    }

    bool empty() const {
        return _count == 0;
    }

    void clear() {
        _count = 0;
        for (auto i : data) {
            std::pair<size_t, size_t> ind = pos(i.first);
            buckets[ind.first].idle = true;
            buckets[ind.second].idle = true;
            buckets_size[ind.first] = 0;
            buckets_size[ind.second] = 0;
        }
        data.clear();
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, Hash _hash_f = Hash()) : hash_f(_hash_f) {
        for (auto i : list) {
            insert(i);
        }
    }

    HashMap(Hash _hash_f = Hash()) : hash_f(_hash_f) {
        buckets.resize(cur_size);
    }

    template<typename Iter>
    HashMap(Iter begin, Iter end, Hash _hash_f = Hash()) : hash_f(_hash_f) {
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }

    const HashMap & operator = (const HashMap & other) {
        _count = 0;
        for (auto i : data) {
            std::pair<size_t, size_t> ind = pos(i.first);
            buckets[ind.first].idle = true;
            buckets[ind.second].idle = true;
            buckets_size[ind.first] = 0;
            buckets_size[ind.second] = 0;
        }
        data.clear();
        for (auto i : other) {
            this->insert(i);
        }
        return other;
    }

    ValueType & operator[] (const KeyType key) {
        iterator it = this->find(key);
        if (it == this->end()) {
            this->insert(std::make_pair(key, ValueType()));
            it = this->find(key);
        }
        return it->second;
    }

    const ValueType & at(const KeyType key) const {
        const_iterator it = this->find(key);
        if (it == this->end()) {
            throw std::out_of_range("Invalid key\n");
        }
        return it->second;
    }

    iterator find(KeyType key) {
        std::pair<size_t,size_t> i = pos(key);
        if (!buckets[i.first].idle) {
            iterator it = buckets[i.first].begin;
            while (it != buckets[i.first].end) {
                if (!(it->first == key)) {
                    ++it;
                } else {
                    return it;
                }
            }
            if (it->first == key) {
                return it;
            }
        }
        if (!buckets[i.second].idle) {
            iterator it = buckets[i.second].begin;
            while (it != buckets[i.second].end) {
                if (!(it->first == key)) {
                    ++it;
                } else {
                    return it;
                }
            }
            if (it->first == key) {
                return it;
            }
        }
        return end();
    }

    const_iterator find(KeyType key) const {
        size_t i = hash_f(key) % cur_size;
        size_t j = HASH(hash_f(key)) % cur_size;
        if (!buckets[i].idle) {
            const_iterator it = buckets[i].begin;
            while (it != buckets[i].end) {
                if (!(it->first == key)) {
                    ++it;
                } else {
                    return it;
                }
            }
            if (it->first == key) {
                return it;
            }
        }
        if (!buckets[j].idle) {
            const_iterator it = buckets[j].begin;
            while (it != buckets[j].end) {
                if (!(it->first == key)) {
                    ++it;
                } else {
                    return it;
                }
            }
            if (it->first == key) {
                return it;
            }
        }
        return end();
    }

    void rebuild() {
        if (static_cast<int>(size()) * 2 > static_cast<int>(cur_size)) {
            cur_size *= 2;
            buckets.clear();
            buckets_size.clear();
            buckets.resize(cur_size);
            buckets_size.resize(cur_size, 0);
            pair_list tmp;
            for (auto i : data) {
                tmp.emplace_back(i);
            }
            data.clear();
            _count = 0;
            for (auto i : tmp) {
                insert(i);
            }
        }
    }

    size_t get_bucket_size(size_t i) {
        return (buckets_size[i]);
    }
    iterator insert(std::pair<const KeyType, ValueType> elem) {
        rebuild();
        std::pair<size_t,size_t> opa = pos(elem.first);
        size_t i;
        if (get_bucket_size(opa.first) <= get_bucket_size(opa.second)) {
            i = opa.first;
        } else {
            i = opa.second;
        }
        if (buckets[i].idle == true) {
            data.push_back(elem);
            buckets[i].idle = false;
            buckets[i].begin = end();
            --buckets[i].begin;
            buckets[i].end = data.end();
            --buckets[i].end;
            ++_count;
        } else if (find(elem.first) == end()) {
            ++buckets[i].end;
            buckets[i].end = data.insert(buckets[i].end, elem);
            ++_count;
        }
        return buckets[i].end;
    }

    void erase(KeyType key) {
        std::pair<size_t,size_t> i = pos(key);
        if (!buckets[i.first].idle) {
            if (buckets[i.first].begin == buckets[i.first].end) {
                if (buckets[i.first].begin->first == key) {
                    buckets[i.first].idle = true;
                    data.erase(buckets[i.first].begin);
                    --buckets_size[i.first];
                    --_count;
                    return;
                }
            } else if (buckets[i.first].begin->first == key || buckets[i.first].end->first == key) {
                if (buckets[i.first].begin->first == key) {
                    iterator tmp = buckets[i.first].begin;
                    ++buckets[i.first].begin;
                    data.erase(tmp);
                    --buckets_size[i.first];
                    --_count;
                    return;
                } else {
                    iterator tmp = buckets[i.first].end;
                    --buckets[i.first].end;
                    data.erase(tmp);
                    --buckets_size[i.first];
                    --_count;
                    return;
                }
            }
        }
        if (!buckets[i.second].idle) {
            if (buckets[i.second].begin == buckets[i.second].end) {
                if (buckets[i.second].begin->first == key) {
                    buckets[i.second].idle = true;
                    data.erase(buckets[i.second].begin);
                    --buckets_size[i.second];
                    --_count;
                    return;
                }
            } else if (buckets[i.second].begin->first == key || buckets[i.second].end->first == key) {
                if (buckets[i.second].begin->first == key) {
                    iterator tmp = buckets[i.second].begin;
                    ++buckets[i.second].begin;
                    data.erase(tmp);
                    --buckets_size[i.second];
                    --_count;
                    return;
                } else {
                    iterator tmp = buckets[i.second].end;
                    --buckets[i.second].end;
                    data.erase(tmp);
                    --buckets_size[i.second];
                    --_count;
                    return;
                }
            }
        }
        if (find(key) != end()) {
            data.erase(find(key));
            --_count;
        }
    }
};
