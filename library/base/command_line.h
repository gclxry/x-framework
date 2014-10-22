
// ����������������.
// ��ѡ���ؿ���ͨ���Ⱥź����һ����ֵ���, ����"-switch=value".
// ����switchΪǰ׺�Ĳ�������Ϊ���Ӳ���. ��"--"��β�Ĳ�����ֹswitch�Ľ���,
// ���º����������ݱ���Ϊһ�����Ӳ���.
// ��һ��ֻ����CommandLine������ʾ��ǰ����������������, ������main()������
// ��ʼ��(��������ƽ̨��ͬ����).

#ifndef __base_command_line_h__
#define __base_command_line_h__

#pragma once

#include <map>
#include <string>
#include <vector>

class FilePath;

class CommandLine
{
public:
    // �����в�������
    typedef std::wstring StringType;

    typedef StringType::value_type CharType;
    typedef std::vector<StringType> StringVector;
    typedef std::map<std::string, StringType> SwitchMap;

    // ����һ���µĿ�������. |program|�ǽ����еĳ�����(argv[0]).
    enum NoProgram { NO_PROGRAM };
    explicit CommandLine(NoProgram no_program);

    // ͨ��argv[0]�����µ�CommandLine����.
    explicit CommandLine(const FilePath& program);

    // Construct a new command line from an argument list.
    CommandLine(int argc, const CharType* const* argv);
    explicit CommandLine(const StringVector& argv);

    ~CommandLine();

    // ��ʼ����ǰ����������CommandLine����. ��Windowsƽ̨���Ե�������
    // ����, ֱ�ӽ���GetCommandLineW(). ��ΪCRT�����������в����ɿ�,
    // �����ǻ�����Ҫ����CommandLineToArgvW����������.
    static void Init(int argc, const char* const* argv);

    // ���ٵ�ǰ���̵�CommandLine����. ����Ҫ���õײ�⵽��ʼ״̬ʱ����(�����ⲿ
    // ���ÿ���Ҫ���������³�ʼ��ʱ). ���Initֻ�����ù�һ��, ������main()������,
    // ����Ҫǿ�Ƶ���Reset().
    static void Reset();

    // CommandLine������ʾ��ǰ���̵�������. ע��: ����ֵ�ǿ��޸ĵ�,
    // ���̰߳�ȫ, �����޸�ʱҪȷ����ȷ��.
    static CommandLine* ForCurrentProcess();

    static CommandLine FromString(const std::wstring& command_line);

    // Initialize from an argv vector.
    void InitFromArgv(int argc, const CharType* const* argv);
    void InitFromArgv(const StringVector& argv);

    // ���س�ʼ���������ַ���. ����! ������Ҫʹ��, ��Ϊ���ŵ���Ϊ�ǲ���ȷ��.
    StringType GetCommandLineString() const;

    // Returns the original command line string as a vector of strings.
    const StringVector& argv() const { return argv_; }

    // ����/�����������еĳ�����(��һ���ַ���).
    FilePath GetProgram() const;
    void SetProgram(const FilePath& program);

    // ��������а���ָ�������򷵻�true.(��������Сд�޹�)
    bool HasSwitch(const std::string& switch_string) const;

    // ����ָ�����ص�ֵ. ���������ֵ���߲�����, ���ؿ��ַ���.
    std::string GetSwitchValueASCII(const std::string& switch_string) const;
    FilePath GetSwitchValuePath(const std::string& switch_string) const;
    StringType GetSwitchValueNative(const std::string& switch_string) const;

    // ��ȡ���еĿ���
    const SwitchMap& GetSwitches() const { return switches_; }

    // Ϊ��������ӿ���[ֵ��ѡ].
    // ����! �ڿ��ؽ�����"--"���������Ч��.
    void AppendSwitch(const std::string& switch_string);
    void AppendSwitchPath(const std::string& switch_string,
        const FilePath& path);
    void AppendSwitchNative(const std::string& switch_string,
        const StringType& value);
    void AppendSwitchASCII(const std::string& switch_string,
        const std::string& value);

    // ����һ���������п���ָ���Ŀ���(����ֵ, �������). һ�����������ӽ���.
    void CopySwitchesFrom(const CommandLine& source,
        const char* const switches[], size_t count);

    // Get the remaining arguments to the command.
    StringVector GetArgs() const;

    // ��Ӳ���.
    // ע������: ��Ҫʱ�����ŰѲ����������Ա㱻��ȷ����Ϊһ������.
    // AppendArg��Ҫ����ASCII��; ��ASCII����ᱻ��Ϊ��UTF-8�����ʽ.
    void AppendArg(const std::string& value);
    void AppendArgPath(const FilePath& value);
    void AppendArgNative(const StringType& value);

    // �����һ�������е����в���. ���|include_program|��true, |other|
    // �ĳ�����Ҳ�ᱻ��ӽ���.
    void AppendArguments(const CommandLine& other, bool include_program);

    // �ڵ�ǰ�����в�������, �����������, ��"valgrind" ���� "gdb --args".
    void PrependWrapper(const StringType& wrapper);

    // ͨ�������������ַ������г�ʼ��, ������������ǵ�һ���ַ���.
    void ParseFromString(const std::wstring& command_line);

private:
    // ��ֹȱʡ���캯��; ������ʽָ��������.
    CommandLine();
    // ���������캯��, ��Ϊ�����´����ǰ���̵������в���ӱ�־λ. ����:
    //     CommandLine cl(*CommandLine::ForCurrentProcess());
    //     cl.AppendSwitch(...);

    // CommandLine������ʾ��ǰ���̵�������.
    static CommandLine* current_process_commandline_;

    // The argv array: { program, [(--|-|/)switch[=value]]*, [--], [argument]* }
    StringVector argv_;

    // �������Ŀ���ֵ��.
    SwitchMap switches_;

    // The index after the program and switches, any arguments start here.
    size_t begin_args_;
};

#endif //__base_command_line_h__