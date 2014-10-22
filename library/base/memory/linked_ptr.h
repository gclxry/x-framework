
// ���ڸ������õ�"����"ָ������. ÿ��ָ���ض������ָ��ά����һ����λ���ӵ�
// �б���. ��ָ���������һ��ָ�����ٻ������¸�ֵʱ, ��������.
// ʹ��ʱҪС��, �����������ý��ʱ�ᱻ����.
// ���㾯��:
// - ���������ü���ģʽһ��, ѭ�����ûᵼ��й©.
// - ÿ������ָ��ʵ������2��ָ��(8�ֽڶ�����4�ֽ�).
// - ÿ��ָ���ͷ�ʱ, �������������. ��˸��಻�ʺ�ָ���ض�����ָ�����2-3��
//   �����.
// - ����ֻ����linked_ptr<>���󿽱���ʱ��ά��. linked_ptr<>��ԭʼָ��ת��ʱ
//   �ᷢ�����õ����(����ɾ��).
//
// �����һ��Ӧ�ó����Ƕ���STL�����еĶ������ý�������. linked_ptr<>���԰�ȫ��
// Ӧ����vector<>��. �����ط�����̫�ʺ�ʹ��.
//
// ע��: �����linked_ptr<>ʹ�ò�����������, ����linked_ptr<>��������й����
// ��������(��ʹ����ʲô������!).
//
// �̰߳�ȫ:
//   linked_ptr�Ƿ��̰߳�ȫ��. ����linked_ptr����ʵ������һ�ζ�д����.
//
// ����: ����shared_ptr�е�linked_ptr,
//  - ��СΪ2��ָ��(����32λ��ַ��8�ֽ�)
//  - ������ɾ�����̰߳�ȫ��
//  - ֧��weak_ptrs

#ifndef __base_linked_ptr_h__
#define __base_linked_ptr_h__

#pragma once

#include "base/logging.h"

// linked_ptr_internal������linked_ptr<>ʵ����ʹ��. ��Ҫһ����ģ��������Ϊ
// ��ͬ���͵�linked_ptr<>��������ͬ����(linked_ptr<Superclass>(obj) vs
// linked_ptr<Subclass>(obj)). ����, ��ͬ���͵�linked_ptr���ܻ��������ͬ��
// ������, ���������Ҫһ����������.
//
// �벻Ҫֱ��ʹ�������. ʹ��linked_ptr<T>.
class linked_ptr_internal
{
public:
    // ����һ���µĻ�ֻ���б�ʵ��.
    void join_new()
    {
        next_ = this;
    }

    // ����һ�����ڵĻ�.
    void join(linked_ptr_internal const* ptr)
    {
        next_ = ptr->next_;
        ptr->next_ = this;
    }

    // �뿪����Ļ�. ����ǻ������һ����Ա����true. һ�����óɹ�, �����ٴ�
    // join()��������.
    bool depart()
    {
        if(next_ == this) return true;
        linked_ptr_internal const* p = next_;
        while(p->next_ != this) p = p->next_;
        p->next_ = next_;
        return false;
    }

private:
    mutable linked_ptr_internal const* next_;
};

template<typename T>
class linked_ptr
{
public:
    typedef T element_type;

    // �ӹ�ָ�������Ȩ. ���󴴽��󾡿����.
    explicit linked_ptr(T* ptr = NULL) { capture(ptr); }
    ~linked_ptr() { depart(); }

    // �������ڵ�linked_ptr<>, ��ӵ����ö���.
    template<typename U> linked_ptr(linked_ptr<U> const& ptr) { copy(&ptr); }

    linked_ptr(linked_ptr const& ptr)
    {
        DCHECK_NE(&ptr, this);
        copy(&ptr);
    }

    // ��ֵ�������ͷž�ֵ������ֵ.
    template<typename U> linked_ptr& operator=(linked_ptr<U> const& ptr)
    {
        depart();
        copy(&ptr);
        return *this;
    }

    linked_ptr& operator=(linked_ptr const& ptr)
    {
        if(&ptr != this)
        {
            depart();
            copy(&ptr);
        }
        return *this;
    }

    // ֻ��ָ���Ա.
    void reset(T* ptr = NULL)
    {
        depart();
        capture(ptr);
    }
    T* get() const { return value_; }
    T* operator->() const { return value_; }
    T& operator*() const { return *value_; }
    // �ͷ�ָ����������Ȩ������. ��Ҫlinked_ptr�Զ���ӵ�ж���������Ȩ.
    T* release()
    {
        bool last = link_.depart();
        CHECK(last);
        T* v = value_;
        value_ = NULL;
        return v;
    }

    bool operator==(const T* p) const { return value_ == p; }
    bool operator!=(const T* p) const { return value_ != p; }
    template<typename U>
    bool operator==(linked_ptr<U> const& ptr) const
    {
        return value_ == ptr.get();
    }
    template<typename U>
    bool operator!=(linked_ptr<U> const& ptr) const
    {
        return value_ != ptr.get();
    }

private:
    template<typename U>
    friend class linked_ptr;

    T* value_;
    linked_ptr_internal link_;

    void depart()
    {
        if(link_.depart()) delete value_;
    }

    void capture(T* ptr)
    {
        value_ = ptr;
        link_.join_new();
    }

    template<typename U> void copy(linked_ptr<U> const* ptr)
    {
        value_ = ptr->get();
        if(value_)
        {
            link_.join(&ptr->link_);
        }
        else
        {
            link_.join_new();
        }
    }
};

template<typename T> inline
bool operator==(T* ptr, const linked_ptr<T>& x)
{
    return ptr == x.get();
}

template<typename T> inline
bool operator!=(T* ptr, const linked_ptr<T>& x)
{
    return ptr != x.get();
}

// ����ת��T*Ϊlinked_ptr<T>.
// make_linked_ptr(new FooBarBaz<type>(arg))��
// linked_ptr<FooBarBaz<type> >(new FooBarBaz<type>(arg))�ļ���ʽ.
template<typename T>
linked_ptr<T> make_linked_ptr(T* ptr)
{
    return linked_ptr<T>(ptr);
}

#endif //__base_linked_ptr_h__