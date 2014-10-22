
#ifndef __base_registry_h__
#define __base_registry_h__

#pragma once

#include <windows.h>

#include <string>

#include "base/basic_types.h"

namespace base
{
    namespace win
    {

        // ��д����Windowsע���Ĺ�����. ע���ʻ��: "key"�������ļ���, ������
        // "values", ÿ��value��һЩ<name, data>�����, ����data����ĳ����������.
        class RegKey
        {
        public:
            RegKey();
            RegKey(HKEY rootkey, const wchar_t* subkey, REGSAM access);
            ~RegKey();

            LONG Create(HKEY rootkey, const wchar_t* subkey, REGSAM access);

            LONG CreateWithDisposition(HKEY rootkey, const wchar_t* subkey,
                DWORD* disposition, REGSAM access);

            LONG Open(HKEY rootkey, const wchar_t* subkey, REGSAM access);

            // �����Ӽ�, ����������.
            LONG CreateKey(const wchar_t* name, REGSAM access);

            // ���Ӽ�.
            LONG OpenKey(const wchar_t* name, REGSAM access);

            void Close();

            DWORD ValueCount() const;

            // ��n��value������.
            LONG ReadName(int index, std::wstring* name);

            // ���key�Ϸ�����true.
            bool Valid() const { return key_ != NULL; }

            // ɾ��һ����ֵ���������Ӽ�ֵ; ʹ��ʱС��.
            LONG DeleteKey(const wchar_t* name);

            // ɾ�����е�һ��ֵ.
            LONG DeleteValue(const wchar_t* name);

            bool ValueExists(const wchar_t* name);

            LONG ReadValue(const wchar_t* name, void* data, DWORD* dsize,
                DWORD* dtype) const;
            LONG ReadValue(const wchar_t* name, std::wstring* value) const;
            LONG ReadValueDW(const wchar_t* name, DWORD* value) const;
            LONG ReadInt64(const wchar_t* name, int64* value) const;

            LONG WriteValue(const wchar_t* name, const void* data, DWORD dsize,
                DWORD dtype);
            LONG WriteValue(const wchar_t* name, const wchar_t* value);
            LONG WriteValue(const wchar_t* name, DWORD value);

            // ��ʼ���Ӽ�ֵ�Ƿ��޸�. ���򿪵�ʱ��������KEY_NOTIFY����Ȩ��.
            LONG StartWatching();

            // ���û����StartWatching, ���Ƿ���false. ����������κζ������޸�,
            // ��������true. |watch_event_|���ܱ�����, ����������const.
            bool HasChanged();

            // ���û�ֶ�����, �����������Զ�����. ������ڼ��ӷ���true, ����false.
            LONG StopWatching();

            inline bool IsWatching() const { return watch_event_ != 0; }
            HANDLE watch_event() const { return watch_event_; }
            HKEY Handle() const { return key_; }

        private:
            HKEY key_; // ��ǰ���ʵ�ע����.
            HANDLE watch_event_;

            DISALLOW_COPY_AND_ASSIGN(RegKey);
        };

        // ����ע����ض���������value. �����Ӧ�������ɲ���Ҫ���ݳ���
        // ����MAX_PATH, ��������������ⲻһ����.
        class RegistryValueIterator
        {
        public:
            RegistryValueIterator(HKEY root_key, const wchar_t* folder_key);
            ~RegistryValueIterator();

            DWORD ValueCount() const;

            // ���iterator�Ϸ�����true.
            bool Valid() const;

            // ��ǰ��������һ��value.
            void operator++();

            const wchar_t* Name() const { return name_; }
            const wchar_t* Value() const { return value_; }
            DWORD ValueSize() const { return value_size_; }
            DWORD Type() const { return type_; }

            int Index() const { return index_; }

        private:
            // ��ȡ��ǰvalue.
            bool Read();

            // ��ǰ���ʵ�ע����.
            HKEY key_;

            // ��ǰ���ʵ�value����.
            int index_;

            // ��ǰvalue.
            wchar_t name_[MAX_PATH];
            wchar_t value_[MAX_PATH];
            DWORD value_size_;
            DWORD type_;

            DISALLOW_COPY_AND_ASSIGN(RegistryValueIterator);
        };

        class RegistryKeyIterator
        {
        public:
            RegistryKeyIterator(HKEY root_key, const wchar_t* folder_key);
            ~RegistryKeyIterator();

            DWORD SubkeyCount() const;

            // ���iterator�Ϸ�����true.
            bool Valid() const;

            // ��ǰ��������һ����.
            void operator++();

            const wchar_t* Name() const { return name_; }

            int Index() const { return index_; }

        private:
            // ��ȡ��ǰ��.
            bool Read();

            // ��ǰ���ʵ�ע����.
            HKEY key_;

            // ��ǰ���ʵļ�����.
            int index_;

            wchar_t name_[MAX_PATH];

            DISALLOW_COPY_AND_ASSIGN(RegistryKeyIterator);
        };

    } //namespace win
} //namespace base

#endif //__base_registry_h__