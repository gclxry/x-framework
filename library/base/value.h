
// ����ݹ�����ݴ洢��Value, ���ڴ洢�����Լ���������������. ���԰���(�ݹ��)
// lists��dictionaries, ���Էǳ�ʵ��. Ȼ��, API���ճ����ķ�ʽ�������Ż�, Ҳ��
// ��˵�洢��һ�������ݵ���״�ṹ. ����DictionaryValue��, ���Է��������:
//
// root->SetString("global.pages.homepage", "http://goateleporter.com");
// std::string homepage = "http://google.com"; // default/fallback value
// root->GetString("global.pages.homepage", &homepage);
//
// "global"��"pages"��DictionaryValues, "homepage"���ַ���. ���ĳЩ·���ϵ�
// Ԫ�ز�����, SetString()�������������ǰ�ᴴ����ʧ��Ԫ�ز����ӵ�����.

#ifndef __base_value_h__
#define __base_value_h__

#pragma once

#include <map>
#include <string>
#include <vector>

#include "basic_types.h"
#include "string16.h"

namespace base
{

    class BinaryValue;
    class DictionaryValue;
    class FundamentalValue;
    class ListValue;
    class StringValue;
    class Value;

    typedef std::vector<Value*> ValueVector;
    typedef std::map<std::string, Value*> ValueMap;

    // Value��������Values��Ļ���. Value����ͨ��Create*Value()��������
    // ʵ����, ����ֱ��������ʵ����.
    class Value
    {
    public:
        enum Type
        {
            TYPE_NULL = 0,
            TYPE_BOOLEAN,
            TYPE_INTEGER,
            TYPE_DOUBLE,
            TYPE_STRING,
            TYPE_BINARY,
            TYPE_DICTIONARY,
            TYPE_LIST
        };

        virtual ~Value();

        // ��������Value���󷽱�Ľӿ�, ����֪���������ʵ��.
        // ���ǿ��Է���һ���Ϸ���Value*.
        static Value* CreateNullValue();
        static FundamentalValue* CreateBooleanValue(bool in_value);
        static FundamentalValue* CreateIntegerValue(int in_value);
        static FundamentalValue* CreateDoubleValue(double in_value);
        static StringValue* CreateStringValue(const std::string& in_value);
        static StringValue* CreateStringValue(const string16& in_value);

        // ���ص�ǰValue����洢����������. ÿ������ֻ����һ������, ���ͨ��
        // Typeȷ������������ת��Value*��(Implementing Class)*�ǰ�ȫ��.
        // ͬʱ, Value���󹹽�֮�󲻻��޸�����.
        Type GetType() const { return type_; }

        // �����ǰ������ָ�������򷵻�true.
        bool IsType(Type type) const { return type == type_; }

        // ���·�������Ļ�ȡ����. �����ǰ������ת����ָ������, ͨ��|out_value|
        // ������������, ��������true. ����������false, |out_value|δ�޸�.
        virtual bool GetAsBoolean(bool* out_value) const;
        virtual bool GetAsInteger(int* out_value) const;
        virtual bool GetAsDouble(double* out_value) const;
        virtual bool GetAsString(std::string* out_value) const;
        virtual bool GetAsString(string16* out_value) const;
        virtual bool GetAsList(ListValue** out_value);
        virtual bool GetAsList(const ListValue** out_value) const;

        // ����һ�������������, ����ָ��. ������ӵ�п���������Ȩ.
        virtual Value* DeepCopy() const;

        // �Ƚ�����Value����������Ƿ���ͬ.
        virtual bool Equals(const Value* other) const;

        // �Ƚ�����Value����������Ƿ���ͬ. �ܴ���NULL�����. ����NULL����Ϊ��ͬ,
        // ���Ǹ�Value::CreateNullValue()��ֵ����ͬ.
        static bool Equals(const Value* a, const Value* b);

        // TODO(sky) bug 91396: remove this when we figure out 91396.
        // If true crash when deleted.
        void set_check_on_delete(bool value) { check_on_delete_ = value; }

    protected:
        // ���ڵ����������ǲ���ȫ��(Ӧ��ʹ�������Create*Value()��̬����),
        // �������������õ�.
        explicit Value(Type type);

        // See description above setter.
        bool check_on_delete_;

    private:
        Value();

        Type type_;

        DISALLOW_COPY_AND_ASSIGN(Value);
    };

    // FundamentalValue��ʾ�򵥵Ļ�����������.
    class FundamentalValue : public Value
    {
    public:
        explicit FundamentalValue(bool in_value);
        explicit FundamentalValue(int in_value);
        explicit FundamentalValue(double in_value);
        virtual ~FundamentalValue();

        // Overridden from Value:
        virtual bool GetAsBoolean(bool* out_value) const;
        virtual bool GetAsInteger(int* out_value) const;
        virtual bool GetAsDouble(double* out_value) const;
        virtual FundamentalValue* DeepCopy() const;
        virtual bool Equals(const Value* other) const;

    private:
        union
        {
            bool boolean_value_;
            int integer_value_;
            double double_value_;
        };

        DISALLOW_COPY_AND_ASSIGN(FundamentalValue);
    };

    class StringValue : public Value
    {
    public:
        // ��UTF-8խ�ַ�����ʼ��һ��StringValue.
        explicit StringValue(const std::string& in_value);

        // ��string16��ʼ��һ��StringValue.
        explicit StringValue(const string16& in_value);

        virtual ~StringValue();

        // Overridden from Value:
        virtual bool GetAsString(std::string* out_value) const;
        virtual bool GetAsString(string16* out_value) const;
        virtual StringValue* DeepCopy() const;
        virtual bool Equals(const Value* other) const;

    private:
        std::string value_;

        DISALLOW_COPY_AND_ASSIGN(StringValue);
    };

    class BinaryValue: public Value
    {
    public:
        virtual ~BinaryValue();

        // ����һ����ʾ�����ƻ�������Value. ����ɹ��µĶ���ӹ�ָ�������Ȩ.
        // ���bufferΪNULL����NULL.
        static BinaryValue* Create(char* buffer, size_t size);

        // ��Ҫά�ֻ���������Ȩʱ, ʹ�øù��������������������ݴ����µ�BinaryValue.
        // ���bufferΪNULL����NULL.
        static BinaryValue* CreateWithCopiedBuffer(const char* buffer, size_t size);


        size_t GetSize() const { return size_; }
        char* GetBuffer() { return buffer_; }
        const char* GetBuffer() const { return buffer_; }

        // ���෽��.
        virtual BinaryValue* DeepCopy() const;
        virtual bool Equals(const Value* other) const;

    private:
        // ���캯��˽��, ����ֻ�ᴴ�����кϷ��Ļ�����ָ��ʹ�С��BinaryValue.
        BinaryValue(char* buffer, size_t size);

        char* buffer_;
        size_t size_;

        DISALLOW_COPY_AND_ASSIGN(BinaryValue);
    };

    // DictionaryValue�ṩһ��key-value�ֵ�, ͨ��"path"�������еݹ����;
    // �μ��ļ�������ע��. Keys��|std::string|s, ��ҪUTF-8����.
    class DictionaryValue : public Value
    {
    public:
        DictionaryValue();
        virtual ~DictionaryValue();

        // ��ǰ�ֵ������ָ����key�򷵻�true.
        bool HasKey(const std::string& key) const;

        // �����ֵ��Values����.
        size_t size() const { return dictionary_.size(); }

        // �����ֵ��Ƿ�Ϊ��.
        bool empty() const { return dictionary_.empty(); }

        // ����ֵ���������.
        void Clear();

        // ָ��·������Value. ·����ʽ��"<key>"��"<key>.<key>.[...]", "."�����ֵ�
        // ����һ������. ����Ȼ"."���ܳ�����key��, ����û�������������.
        // ·�������������ڵ�key, ���ߴ��ڵ�����DictionaryValueʱ, ����һ���µ�
        // DictionaryValue�����ӵ�·���϶�Ӧλ��.
        // ע���ֵ��ӹ�|in_value|������Ȩ, ����|in_value|����ΪNULL.
        void Set(const std::string& path, Value* in_value);

        // Set()�ļ���ʽ. �滻·���ϴ��ڵ�����, ��ʹ���Ͳ�ͬ.
        void SetBoolean(const std::string& path, bool in_value);
        void SetInteger(const std::string& path, int in_value);
        void SetDouble(const std::string& path, double in_value);
        void SetString(const std::string& path, const std::string& in_value);
        void SetString(const std::string& path, const string16& in_value);

        // ��Set()����, ����'.'�������⴦��. ����URLs����·��.
        void SetWithoutPathExpansion(const std::string& key, Value* in_value);

        // ָ��·����ȡValue. ·����ʽ��"<key>"��"<key>.<key>.[...]", "."�����ֵ�
        // ����һ������. ���·�������ɹ�, ͨ��|out_value|������������key��Ӧ��
        // ����, ��������true. ����, ����false�Ҳ��޸�|out_value|.
        // ע���ֵ�����ӵ�з������ݵ�����Ȩ.
        bool Get(const std::string& path, Value** out_value) const;

        // Get()�ļ���ʽ. ���·���Ϸ���·��ĩ�˵�ֵ����ָ����ʽ, �򷵻�����,
        // �Һ�������true.
        bool GetBoolean(const std::string& path, bool* out_value) const;
        bool GetInteger(const std::string& path, int* out_value) const;
        bool GetDouble(const std::string& path, double* out_value) const;
        bool GetString(const std::string& path, std::string* out_value) const;
        bool GetString(const std::string& path, string16* out_value) const;
        bool GetStringASCII(const std::string& path, std::string* out_value) const;
        bool GetBinary(const std::string& path, BinaryValue** out_value) const;
        bool GetDictionary(const std::string& path,
            DictionaryValue** out_value) const;
        bool GetList(const std::string& path, ListValue** out_value) const;

        // ��Get()���ƣ�����'.'�������⴦��. ����URLs����·��.
        bool GetWithoutPathExpansion(const std::string& key,
            Value** out_value) const;
        bool GetIntegerWithoutPathExpansion(const std::string& key,
            int* out_value) const;
        bool GetDoubleWithoutPathExpansion(const std::string& key,
            double* out_value) const;
        bool GetStringWithoutPathExpansion(const std::string& key,
            std::string* out_value) const;
        bool GetStringWithoutPathExpansion(const std::string& key,
            string16* out_value) const;
        bool GetDictionaryWithoutPathExpansion(const std::string& key,
            DictionaryValue** out_value) const;
        bool GetListWithoutPathExpansion(const std::string& key,
            ListValue** out_value) const;

        // �Ƴ��ֵ���ָ��·��(�������ֵ�, ���·������һ���ֲ���key)������. ���
        // |out_value|��Ϊ��, �Ƴ��������Լ�����Ȩת�Ƶ�out_value. ���|out_value|
        // Ϊ��, ɾ���Ƴ�������. ���|path|�ǺϷ�·������true; ���򷵻�false��
        // DictionaryValue���ݲ��ı�.
        bool Remove(const std::string& path, Value** out_value);

        // ����Remove(), ����'.'�������⴦��. ����URLs����·��.
        bool RemoveWithoutPathExpansion(const std::string& key, Value** out_value);

        // ����һ�����, ����������dictionaries��lists. ���᷵��NULL, ��ʹ|this|
        // Ϊ��.
        DictionaryValue* DeepCopyWithoutEmptyChildren();

        // �ϲ��ֵ�. ��ݹ����, �������ֵ�Ҳ��ϲ�. key��ͻʱ, ����ѡ������ֵ�
        // ����, ���еĻᱻ�滻.
        void MergeDictionary(const DictionaryValue* dictionary);

        // Swaps contents with the |other| dictionary.
        void Swap(DictionaryValue* other)
        {
            dictionary_.swap(other->dictionary_);
        }

        // key_iterator�����ֵ��е�keys�ĵ�����. ���������޸��ֵ�.
        //
        // ֻ��ʹ��XXXWithoutPathExpansion() APIs, ��Ҫʹ��XXX() APIs. ȷ������
        // '.'��keys����������.
        class key_iterator : private std::iterator<std::input_iterator_tag,
            const std::string>
        {
        public:
            explicit key_iterator(ValueMap::const_iterator itr) { itr_ = itr; }
            key_iterator operator++()
            {
                ++itr_;
                return *this;
            }
            const std::string& operator*() { return itr_->first; }
            bool operator!=(const key_iterator& other) { return itr_ != other.itr_; }
            bool operator==(const key_iterator& other) { return itr_ == other.itr_; }

        private:
            ValueMap::const_iterator itr_;
        };

        key_iterator begin_keys() const { return key_iterator(dictionary_.begin()); }
        key_iterator end_keys() const { return key_iterator(dictionary_.end()); }

        // ���෽��.
        virtual DictionaryValue* DeepCopy() const;
        virtual bool Equals(const Value* other) const;

    private:
        ValueMap dictionary_;

        DISALLOW_COPY_AND_ASSIGN(DictionaryValue);
    };

    // ��ʾValue���ݵ��б�.
    class ListValue : public Value
    {
    public:
        // ������.
        typedef ValueVector::iterator iterator;
        typedef ValueVector::const_iterator const_iterator;

        ListValue();
        virtual ~ListValue();

        // ����ǰ����.
        void Clear();

        // ����Values����.
        size_t GetSize() const { return list_.size(); }

        // �����Ƿ�Ϊ��.
        bool empty() const { return list_.empty(); }

        // ����listָ������������. ���Խ��, ���null����.
        // �ɹ�����true, �������Ϊ��������in_valueΪ�շ���false.
        bool Set(size_t index, Value* in_value);

        // ��ȡָ������������. �������ڵ�ǰ�б�����ʱ���޸�|out_value|(����true).
        // ע��listʼ��ӵ�����ݵ�����Ȩ.
        bool Get(size_t index, Value** out_value) const;

        // Get()�ļ򻯰汾. �������Ϸ�����������ָ����ʽ����ʱ���޸�|out_value|
        // (����true).
        bool GetBoolean(size_t index, bool* out_value) const;
        bool GetInteger(size_t index, int* out_value) const;
        bool GetDouble(size_t index, double* out_value) const;
        bool GetString(size_t index, std::string* out_value) const;
        bool GetString(size_t index, string16* out_value) const;
        bool GetBinary(size_t index, BinaryValue** out_value) const;
        bool GetDictionary(size_t index, DictionaryValue** out_value) const;
        bool GetList(size_t index, ListValue** out_value) const;

        // ��list�Ƴ�ָ������������. ���|out_value|�ǿ�, �Ƴ��������Լ�����Ȩת�Ƶ�
        // |out_value|. ���|out_value|Ϊ��, ɾ���Ƴ�������. ����ֻ�е�|index|�Ϸ�ʱ
        // ����true; ���򷵻�false��ListValue���ݲ��ı�.
        bool Remove(size_t index, Value** out_value);

        // �Ƴ����ҵ��ĵ�һ��|value|, ��ɾ��, �����ڷ���false.
        bool Remove(const Value& value, size_t* index);

        // ��β�����һ������.
        void Append(Value* in_value);

        // ������ݲ����������.
        // �ɹ�����true, ����Ѿ������򷵻�false.
        bool AppendIfNotPresent(Value* in_value);

        // ����������������.
        // �ɹ�����true, Խ�緵��false.
        bool Insert(size_t index, Value* in_value);

        // ��|other|��������.
        void Swap(ListValue* other)
        {
            list_.swap(other->list_);
        }


        iterator begin() { return list_.begin(); }
        iterator end() { return list_.end(); }

        const_iterator begin() const { return list_.begin(); }
        const_iterator end() const { return list_.end(); }

        // ���෽��.
        virtual bool GetAsList(ListValue** out_value);
        virtual bool GetAsList(const ListValue** out_value) const;
        virtual ListValue* DeepCopy() const;
        virtual bool Equals(const Value* other) const;

    private:
        ValueVector list_;

        DISALLOW_COPY_AND_ASSIGN(ListValue);
    };


    // �ӿ���֪��������л��ͷ����л�Value�������ʵ��.
    class ValueSerializer
    {
    public:
        virtual ~ValueSerializer();

        virtual bool Serialize(const Value& root) = 0;

        // �����л��������Value����. ������طǿ�, ������ӵ�з���Value����Ȩ. ���
        // ����NULL��error_code�ǿ�, error_code����Ϊ�ײ������. ���|error_message|
        // �ǿ�, ��������Ϣ��ÿ��ܰ�������λ��.
        virtual Value* Deserialize(int* error_code, std::string* error_str) = 0;
    };

} //namespace base

#endif //__base_value_h__