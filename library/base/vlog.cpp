
#include "vlog.h"

#include "logging.h"
#include "string_number_conversions.h"
#include "string_split.h"

namespace base
{

    const int VlogInfo::kDefaultVlogLevel = 0;

    struct VlogInfo::VmodulePattern
    {
        enum MatchTarget { MATCH_MODULE, MATCH_FILE };

        explicit VmodulePattern(const std::string& pattern);

        VmodulePattern();

        std::string pattern;
        int vlog_level;
        MatchTarget match_target;
    };

    VlogInfo::VmodulePattern::VmodulePattern(const std::string& pattern)
        : pattern(pattern),
        vlog_level(VlogInfo::kDefaultVlogLevel),
        match_target(MATCH_MODULE)
    {
        // ���ģʽ����{\, /}, ��ʾ��Ҫƥ������__FILE__��.
        std::string::size_type first_slash = pattern.find_first_of("\\/");
        if(first_slash != std::string::npos)
        {
            match_target = MATCH_FILE;
        }
    }

    VlogInfo::VmodulePattern::VmodulePattern()
        : vlog_level(VlogInfo::kDefaultVlogLevel),
        match_target(MATCH_MODULE) {}


    VlogInfo::VlogInfo(const std::string& v_switch,
        const std::string& vmodule_switch,
        int* min_log_level)
        : min_log_level_(kDefaultVlogLevel)
    {
        DCHECK(min_log_level != NULL);

        typedef std::pair<std::string, std::string> KVPair;
        int vlog_level = 0;
        if(!v_switch.empty())
        {
            if(base::StringToInt(v_switch, &vlog_level))
            {
                SetMaxVlogLevel(vlog_level);
            }
            else
            {
                LOG(WARNING) << "Could not parse v switch \""
                    << v_switch << "\"";
            }
        }

        std::vector<KVPair> kv_pairs;
        if(!base::SplitStringIntoKeyValuePairs(vmodule_switch, '=',
            ',', &kv_pairs))
        {
            LOG(WARNING) << "Could not fully parse vmodule switch \""
                << vmodule_switch << "\"";
        }
        for(std::vector<KVPair>::const_iterator it=kv_pairs.begin();
            it!=kv_pairs.end(); ++it)
        {
            VmodulePattern pattern(it->first);
            if(!base::StringToInt(it->second, &pattern.vlog_level))
            {
                LOG(WARNING) << "Parsed vlog level for \""
                    << it->first << "=" << it->second
                    << "\" as " << pattern.vlog_level;
            }
            vmodule_levels_.push_back(pattern);
        }
    }

    VlogInfo::~VlogInfo() {}

    // ����·��, ����ȥ����չ��(�������е�-inl��׺)�Ļ�������. ��ʹ��
    // FilePath�Ա�֤��־ϵͳ��������С��.
    base::StringPiece GetModule(const base::StringPiece& file)
    {
        base::StringPiece module(file);
        base::StringPiece::size_type last_slash_pos =
            module.find_last_of("\\/");
        if(last_slash_pos != base::StringPiece::npos)
        {
            module.remove_prefix(last_slash_pos+1);
        }
        base::StringPiece::size_type extension_start = module.rfind('.');
        module = module.substr(0, extension_start);
        static const char kInlSuffix[] = "-inl";
        static const int kInlSuffixLen = arraysize(kInlSuffix) - 1;
        if(module.ends_with(kInlSuffix))
        {
            module.remove_suffix(kInlSuffixLen);
        }
        return module;
    }

    int VlogInfo::GetVlogLevel(const base::StringPiece& file)
    {
        if(!vmodule_levels_.empty())
        {
            base::StringPiece module(GetModule(file));
            for(std::vector<VmodulePattern>::const_iterator it =
                vmodule_levels_.begin(); it!=vmodule_levels_.end(); ++it)
            {
                base::StringPiece target(
                    (it->match_target==VmodulePattern::MATCH_FILE) ? file : module);
                if(MatchVlogPattern(target, it->pattern))
                {
                    return it->vlog_level;
                }
            }
        }
        return GetMaxVlogLevel();
    }

    void VlogInfo::SetMaxVlogLevel(int level)
    {
        // ��־�ȼ��Ǹ��߳�(ԽС��ʾ���صȼ�Խ��).
        *min_log_level_ = -level;
    }

    int VlogInfo::GetMaxVlogLevel() const
    {
        return -*min_log_level_;
    }

    bool MatchVlogPattern(const base::StringPiece& string,
        const base::StringPiece& vlog_pattern)
    {
        base::StringPiece p(vlog_pattern);
        base::StringPiece s(string);
        // ���ַ��Ƚ�ֱ������'*'�ַ�.
        while(!p.empty() && !s.empty() && (p[0]!='*'))
        {
            switch(p[0])
            {
                // б�߱Ƚ�ƥ��б��.
            case '/':
            case '\\':
                if((s[0]!='/') && (s[0]!='\\'))
                {
                    return false;
                }
                break;

                // '?'����ƥ�������ַ�.
            case '?':
                break;

                // �����ַ��������.
            default:
                if(p[0] != s[0])
                {
                    return false;
                }
                break;
            }
            p.remove_prefix(1), s.remove_prefix(1);
        }

        // ����, ��ģʽ��ֻ��ƥ����ַ���.
        if(p.empty())
        {
            return s.empty();
        }

        // �ϲ�������'*'�ַ�, ������һ��.
        while(!p.empty() && (p[0]=='*'))
        {
            p.remove_prefix(1);
        }

        // ����������һ��'*'�ַ�, �����ģʽ����ʾƥ������.
        if(p.empty())
        {
            return true;
        }

        // ��Ϊ������'*'��p��Ϊ��, ���s�ķǿ��ִ��ܺ�pƥ��, ��ʾƥ��ɹ�.
        while(!s.empty())
        {
            if(MatchVlogPattern(s, p))
            {
                return true;
            }
            s.remove_prefix(1);
        }

        // �޷�ƥ��.
        return false;
    }

} //namespace base