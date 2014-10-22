
#ifndef __base_raw_scoped_refptr_mismatch_checker_h__
#define __base_raw_scoped_refptr_mismatch_checker_h__

#pragma once

#include "base/template_util.h"
#include "base/tuple.h"
#include "ref_counted.h"

// ��������scoped_refptr<>���Ͳ�������һ��ԭʼָ��������Ƿǳ�Σ�յ�. ��������
// �����������, ������ִ��ǰ�����������ü���. ���������ϣ���ڴ��ݲ�����ʱ��
// ���������ü���, ������������һ��! ����: http://crbug.com/27191.
// ����һ�����Ա����������������ʱ����, ��ֹ��������ķ���.

namespace base
{

    // ֻ��task.h��callback.h��ʹ��. ���ṩ����ʹ��, ���Է�װ��internal�����ռ�.
    namespace internal
    {

        template<typename T>
        struct NeedsScopedRefptrButGetsRawPtr
        {
            enum
            {
                value = base::false_type::value
            };
        };

        template<typename Params>
        struct ParamsUseScopedRefptrCorrectly
        {
            enum { value = 0 };
        };

        template<>
        struct ParamsUseScopedRefptrCorrectly<Tuple0>
        {
            enum { value = 1 };
        };

        template<typename A>
        struct ParamsUseScopedRefptrCorrectly<Tuple1<A> >
        {
            enum { value = !NeedsScopedRefptrButGetsRawPtr<A>::value };
        };

        template<typename A, typename B>
        struct ParamsUseScopedRefptrCorrectly<Tuple2<A, B> >
        {
            enum { value = !(NeedsScopedRefptrButGetsRawPtr<A>::value ||
                NeedsScopedRefptrButGetsRawPtr<B>::value) };
        };

        template<typename A, typename B, typename C>
        struct ParamsUseScopedRefptrCorrectly<Tuple3<A, B, C> >
        {
            enum { value = !(NeedsScopedRefptrButGetsRawPtr<A>::value ||
                NeedsScopedRefptrButGetsRawPtr<B>::value ||
                NeedsScopedRefptrButGetsRawPtr<C>::value) };
        };

        template<typename A, typename B, typename C, typename D>
        struct ParamsUseScopedRefptrCorrectly<Tuple4<A, B, C, D> >
        {
            enum { value = !(NeedsScopedRefptrButGetsRawPtr<A>::value ||
                NeedsScopedRefptrButGetsRawPtr<B>::value ||
                NeedsScopedRefptrButGetsRawPtr<C>::value ||
                NeedsScopedRefptrButGetsRawPtr<D>::value) };
        };

        template<typename A, typename B, typename C, typename D, typename E>
        struct ParamsUseScopedRefptrCorrectly<Tuple5<A, B, C, D, E> >
        {
            enum { value = !(NeedsScopedRefptrButGetsRawPtr<A>::value ||
                NeedsScopedRefptrButGetsRawPtr<B>::value ||
                NeedsScopedRefptrButGetsRawPtr<C>::value ||
                NeedsScopedRefptrButGetsRawPtr<D>::value ||
                NeedsScopedRefptrButGetsRawPtr<E>::value) };
        };

        template<typename A, typename B, typename C, typename D, typename E,
            typename F>
        struct ParamsUseScopedRefptrCorrectly<Tuple6<A, B, C, D, E, F> >
        {
            enum { value = !(NeedsScopedRefptrButGetsRawPtr<A>::value ||
                NeedsScopedRefptrButGetsRawPtr<B>::value ||
                NeedsScopedRefptrButGetsRawPtr<C>::value ||
                NeedsScopedRefptrButGetsRawPtr<D>::value ||
                NeedsScopedRefptrButGetsRawPtr<E>::value ||
                NeedsScopedRefptrButGetsRawPtr<F>::value) };
        };

        template<typename A, typename B, typename C, typename D, typename E,
            typename F, typename G>
        struct ParamsUseScopedRefptrCorrectly<Tuple7<A, B, C, D, E, F, G> >
        {
            enum { value = !(NeedsScopedRefptrButGetsRawPtr<A>::value ||
                NeedsScopedRefptrButGetsRawPtr<B>::value ||
                NeedsScopedRefptrButGetsRawPtr<C>::value ||
                NeedsScopedRefptrButGetsRawPtr<D>::value ||
                NeedsScopedRefptrButGetsRawPtr<E>::value ||
                NeedsScopedRefptrButGetsRawPtr<F>::value ||
                NeedsScopedRefptrButGetsRawPtr<G>::value) };
        };

        template<typename A, typename B, typename C, typename D, typename E,
            typename F, typename G, typename H>
        struct ParamsUseScopedRefptrCorrectly<Tuple8<A, B, C, D, E, F, G, H> >
        {
            enum { value = !(NeedsScopedRefptrButGetsRawPtr<A>::value ||
                NeedsScopedRefptrButGetsRawPtr<B>::value ||
                NeedsScopedRefptrButGetsRawPtr<C>::value ||
                NeedsScopedRefptrButGetsRawPtr<D>::value ||
                NeedsScopedRefptrButGetsRawPtr<E>::value ||
                NeedsScopedRefptrButGetsRawPtr<F>::value ||
                NeedsScopedRefptrButGetsRawPtr<G>::value ||
                NeedsScopedRefptrButGetsRawPtr<H>::value) };
        };

    } //namespace internal

} //namespace base

#endif //__base_raw_scoped_refptr_mismatch_checker_h__