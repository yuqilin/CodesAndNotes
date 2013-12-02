#include "stdafx.h"
#include "CodecsInfo.h"

//////////////////////////////////////////////////////////////////////////
bool CodecsInfo::CheckTypes(const CAtlArray<GUID>& types, bool fExactMatch)
{
    if (types.GetCount() == 0)
    {
        return fExactMatch ? false : true;
    }

    POSITION pos = this->mediatypes.GetHeadPosition();
    while (pos)
    {
        const MediaTypeItem& item = this->mediatypes.GetNext(pos);
        GUID majortype = GUIDFromCString(item.majortype);
        GUID subtype = GUIDFromCString(item.subtype);

        for (int i = 0, len = types.GetCount() & ~1; i < len; i += 2)
        {
            if (fExactMatch)
            {
                if (majortype == types[i] && majortype != GUID_NULL
                    && subtype == types[i + 1] && subtype != GUID_NULL)
                {
                    return true;
                }
            }
            else
            {
                if ((majortype == GUID_NULL /*|| types[i] == GUID_NULL*/ || majortype == types[i])
                    && (subtype == GUID_NULL /*|| types[i + 1] == GUID_NULL*/ || subtype == types[i + 1]))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
CodecsListEx::CodecsListEx()
{

}

CodecsListEx::~CodecsListEx()
{
    RemoveAll();
}

void CodecsListEx::RemoveAll()
{
    m_codecs.RemoveAll();
    m_sortedcodecs.RemoveAll();
}

bool CodecsListEx::Insert(CodecsInfo* info, int group, int exactmatch, int custom_priority)
{
    bool bInsert = true;

    POSITION pos = m_codecs.GetHeadPosition();
    while (pos)
    {
        codecs_t& f = m_codecs.GetNext(pos);

        if (info == f.info)
        {
            player_log(kLogLevelTrace, _T("CodecsListEx::Insert, Rejected (exact duplicate)"));
            bInsert = false;
            break;
        }

        if (group != f.group)
        {
            continue;
        }
    }

    if (bInsert)
    {
        codecs_t f = {(int)m_codecs.GetCount(), info, group, exactmatch, custom_priority};
        m_codecs.AddTail(f);
        m_sortedcodecs.RemoveAll();
    }

    return bInsert;
}

POSITION CodecsListEx::GetHeadPosition()
{
    if (m_codecs.GetCount() == 0)
    {
        return NULL;
    }
    if (m_sortedcodecs.IsEmpty())
    {
        CAtlArray<codecs_t> sort;
        sort.SetCount(m_codecs.GetCount());
        POSITION pos = m_codecs.GetHeadPosition();
        for (int i = 0; pos; i++)
        {
            sort[i] = m_codecs.GetNext(pos);
        }
        qsort(&sort[0], sort.GetCount(), sizeof(sort[0]), codecs_cmp);
        for (size_t i = 0; i < sort.GetCount(); i++)
        {
            m_sortedcodecs.AddTail(sort[i].info);
        }
    }

    //player_log(kLogLevelTrace, _T("Sorting filters:"));
    POSITION pos = m_sortedcodecs.GetHeadPosition();
    int i = 0;
    while (pos)
    {
        CodecsInfo* info = m_sortedcodecs.GetNext(pos);
        //player_log(kLogLevelTrace, _T("Filter[%d] - %s, %s, %d"), i++, info->name, info->clsid, info->priority);
    }
    
    return m_sortedcodecs.GetHeadPosition();
}

CodecsInfo* CodecsListEx::GetNext(POSITION& pos)
{
    return m_sortedcodecs.GetNext(pos);
}

//group  exact_match  [codecs_type]  custom_priority  default_priority  index
int CodecsListEx::codecs_cmp(const void* a, const void* b)
{
    codecs_t* fa = (codecs_t*)a;
    codecs_t* fb = (codecs_t*)b;

    if (fa->group < fb->group) {
        return -1;
    }
    if (fa->group > fb->group) {
        return +1;
    }

    if (fa->exactmatch && !fb->exactmatch) {
        return -1;
    }
    if (!fa->exactmatch && fb->exactmatch) {
        return +1;
    }

    if (fa->custom_priority > fb->custom_priority) {
        return -1;
    }
    if (fa->custom_priority < fb->custom_priority) {
        return +1;
    }

    if (fa->info->priority > fb->info->priority) {
        return -1;
    }
    if (fa->info->priority < fb->info->priority) {
        return +1;
    }

    if (fa->index < fb->index) {
        return -1;
    }
    if (fa->index > fb->index) {
        return +1;
    }

    return 0;
}
