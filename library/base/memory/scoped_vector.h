
#ifndef __base_scoped_vector_h__
#define __base_scoped_vector_h__

#pragma once

#include <vector>

#include "base/basic_types.h"
#include "base/stl_utilinl.h"

// ScopedVector��װvector, ������������ɾ��Ԫ��.
template<class T>
class ScopedVector
{
public:
    typedef typename std::vector<T*>::iterator iterator;
    typedef typename std::vector<T*>::const_iterator const_iterator;
    typedef typename std::vector<T*>::reverse_iterator reverse_iterator;
    typedef typename std::vector<T*>::const_reverse_iterator
        const_reverse_iterator;

    ScopedVector() {}
    ~ScopedVector() { reset(); }

    std::vector<T*>* operator->() { return &v; }
    const std::vector<T*>* operator->() const { return &v; }
    T*& operator[](size_t i) { return v[i]; }
    const T* operator[](size_t i) const { return v[i]; }

    bool empty() const { return v.empty(); }
    size_t size() const { return v.size(); }

    reverse_iterator rbegin() { return v.rbegin(); }
    const_reverse_iterator rbegin() const { return v.rbegin(); }
    reverse_iterator rend() { return v.rend(); }
    const_reverse_iterator rend() const { return v.rend(); }

    iterator begin() { return v.begin(); }
    const_iterator begin() const { return v.begin(); }
    iterator end() { return v.end(); }
    const_iterator end() const { return v.end(); }

    void push_back(T* elem) { v.push_back(elem); }

    std::vector<T*>& get() { return v; }
    const std::vector<T*>& get() const { return v; }
    void swap(ScopedVector<T>& other) { v.swap(other.v); }
    void release(std::vector<T*>* out)
    {
        out->swap(v);
        v.clear();
    }

    void reset() { STLDeleteElements(&v); }
    void reserve(size_t capacity) { v.reserve(capacity); }
    void resize(size_t new_size) { v.resize(new_size); }

    // ScopedVector�ӹ�|x|������Ȩ.
    iterator insert(iterator position, T* x)
    {
        return v.insert(position, x);
    }

    iterator erase(iterator position)
    {
        delete *position;
        return v.erase(position);
    }

    iterator erase(iterator first, iterator last)
    {
        STLDeleteContainerPointers(first, last);
        return v.erase(first, last);
    }

    // ����|erase()|, ���ǲ�ɾ��Ԫ��|position|.
    iterator weak_erase(iterator position)
    {
        return v.erase(position);
    }

    // ����|erase()|, ���ǲ�ɾ��[first, last)�����Ԫ��.
    iterator weak_erase(iterator first, iterator last)
    {
        return v.erase(first, last);
    }

private:
    std::vector<T*> v;

    DISALLOW_COPY_AND_ASSIGN(ScopedVector);
};

#endif //__base_scoped_vector_h__