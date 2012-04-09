#ifndef CONTAINERS_ARCHIVE_STL_TYPES_HPP_
#define CONTAINERS_ARCHIVE_STL_TYPES_HPP_

#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>

#include "containers/archive/archive.hpp"

// Implementations for pair, map, set, string, and vector.

template <class T, class U>
write_message_t &operator<<(write_message_t &msg, const std::pair<T, U> &p) {
    msg << p.first;
    msg << p.second;
    return msg;
}

template <class T, class U>
int deserialize(read_stream_t *s, std::pair<T, U> *p) {
    int res = deserialize(s, &p->first);
    if (res) { return res; }
    res = deserialize(s, &p->second);
    return res;
}

template <class K, class V>
write_message_t &operator<<(write_message_t &msg, const std::map<K, V> &m) {
    // Extreme platform paranoia: It could become important that we
    // use something consistent like uint64_t for the size, not some
    // platform-specific size type such as std::map<K, V>::size_type.
    uint64_t sz = m.size();

    msg << sz;
    for (typename std::map<K, V>::const_iterator it = m.begin(), e = m.end(); it != e; ++it) {
        msg << *it;
    }

    return msg;
}

template <class K, class V>
int deserialize(read_stream_t *s, std::map<K, V> *m) {
    m->clear();

    uint64_t sz;
    int res = deserialize(s, &sz);
    if (res) { return res; }

    // Using position should make this function take linear time, not
    // sz*log(sz) time.
    typename std::map<K, V>::iterator position = m->begin();

    for (uint64_t i = 0; i < sz; ++i) {
        std::pair<K, V> p;
        res = deserialize(s, &p);
        if (res) { return res; }
        position = m->insert(position, p);
    }

    return 0;
}

template <class T>
write_message_t &operator<<(write_message_t &msg, const std::set<T> &s) {
    uint64_t sz = s.size();

    msg << sz;
    for (typename std::set<T>::const_iterator it = s.begin(), e = s.end(); it != e; ++it) {
        msg << *it;
    }

    return msg;
}

template <class T>
int deserialize(read_stream_t *s, std::set<T> *out) {
    out->clear();

    uint64_t sz;
    int res = deserialize(s, &sz);
    if (res) { return res; }

    typename std::set<T>::iterator position = out->begin();

    for (uint64_t i = 0; i < sz; ++i) {
        T value;
        res = deserialize(s, &value);
        if (res) { return res; }
        position = out->insert(position, value);
    }

    return 0;
}

inline
write_message_t &operator<<(write_message_t &msg, const std::string &s) {
    const char *data = s.data();
    int64_t sz = s.size();
    rassert(sz >= 0);

    msg << sz;
    msg.append(data, sz);

    return msg;
}

inline
int deserialize(read_stream_t *s, std::string *out) {
    out->clear();

    int64_t sz;
    int res = deserialize(s, &sz);
    if (res) { return res; }

    if (sz < 0) {
        return -3;
    }

    // Unfortunately we have to do an extra copy before dumping data
    // into a std::string.
    std::vector<char> v(sz);
    rassert(v.size() == uint64_t(sz));

    int64_t num_read = force_read(s, v.data(), sz);
    if (num_read == -1) {
        return -1;
    }
    if (num_read < sz) {
        return -2;
    }

    rassert(num_read == sz, "force_read returned an invalid value %ld", num_read);

    out->assign(v.data(), v.size());

    return 0;
}

template <class T>
write_message_t &operator<<(write_message_t &msg, const std::vector<T> &v) {
    uint64_t sz = v.size();

    msg << sz;
    for (typename std::vector<T>::const_iterator it = v.begin(), e = v.end(); it != e; ++it) {
        msg << *it;
    }

    return msg;
}

template <class T>
int deserialize(read_stream_t *s, std::vector<T> *v) {
    v->clear();

    uint64_t sz;
    int res = deserialize(s, &sz);
    if (res) { return res; }

    v->resize(sz);
    for (uint64_t i = 0; i < sz; ++i) {
        res = deserialize(s, &v[i]);
        if (res) { return res; }
    }

    return 0;
}

// TODO: Stop using std::list! What are you thinking?
template <class T>
write_message_t &operator<<(write_message_t &msg, const std::list<T> &v) {
    uint64_t sz = v.size();
    msg << sz;
    for (typename std::list<T>::const_iterator it = v.begin(), e = v.end(); it != e; ++it) {
        msg << *it;
    }

    return msg;
}

template <class T>
int deserialize(read_stream_t *s, std::list<T> *v) {
    // Omit assertions because it's not a shame if a std::list gets corrupted.

    uint64_t sz;
    int res = deserialize(s, &sz);
    if (res) { return res; }

    for (uint64_t i = 0; i < sz; ++i) {
        // We avoid copying a non-empty value.
        v->push_back(T());
        res = deserialize(s, &v->back());
        if (res) { return res; }
    }

    return 0;
}



#endif  // CONTAINERS_ARCHIVE_STL_TYPES_HPP_
