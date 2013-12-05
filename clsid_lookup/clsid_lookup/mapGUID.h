#pragma once

#include <map>

__inline bool operator<(REFGUID left, REFGUID right)
{
    return memcmp(&left, &right, sizeof(GUID)) < 0;
//     if (left.Data1 < right.Data1)
//         return true;
//     else if (left.Data1 == right.Data1)
//     {
//         if (left.Data2 < right.Data2)
//             return true;
//         else if (left.Data2 == right.Data2)
//         {
//             if (left.Data3 < right.Data3)
//                 return true;
//             else if (left.Data3 == left.Data3)
//             {
//                 if (memcmp(left.Data4, right.Data4, 8) < 0)
//                     return true;
//             }
//         }
//     }
//     return false;

}

typedef std::multimap<GUID, LPCTSTR> mapGUID;


class KnownGuid
{
public:
    static void Load();
    static void Free();
    static LPCTSTR LookupGUID(const GUID& guid);
    static const GUID* LookupFriendlyName(LPCTSTR name);
    static void Dump();

protected:

    class mapGUIDFinder
    {
    public:
        mapGUIDFinder(LPCTSTR name) : m_name(name) {}
        bool operator()(const mapGUID::value_type& pair) const
        {
            return _tcscmp(pair.second, m_name) == 0;
        }
    private:
        LPCTSTR m_name;
    };

    static mapGUID m_mapGUID;
};

