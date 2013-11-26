#include "stdafx.h"
#include "CodecsInfo.h"

//////////////////////////////////////////////////////////////////////////
bool CodecsInfo::CheckTypes(const CAtlArray<GUID>& types, bool fExactMatch)
{
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
CodecsList::CodecsList()
{

}

CodecsList::~CodecsList()
{
    RemoveAll();
}

void CodecsList::RemoveAll()
{
    while (!m_codecs.IsEmpty())
    {
        const codecs_t& f = m_codecs.RemoveHead();
        if (f.autodelete)
        {
            delete f.info;
        }
    }

    m_sortedcodecs.RemoveAll();
}

void CodecsList::Insert(CodecsInfo* info, int group, bool exactmatch, bool autodelete, bool preferred)
{
    bool bInsert = true;

    POSITION pos = m_codecs.GetHeadPosition();
    while (pos)
    {
        codecs_t& f = m_codecs.GetNext(pos);

        if (info == f.info)
        {
            player_log(kLogLevelTrace, _T("CodecsList::Insert, Rejected (exact duplicate)"));
            bInsert = false;
            break;
        }

        if (group != f.group)
        {
            continue;
        }

        if (info->clsid.GetLength() != 0 && info->clsid == f.info->clsid
            && f.info->enable == false)
        {
            player_log(kLogLevelTrace, _T("CodecsList::Insert, Rejected (same filter with merit DO_NOT_USE already in the list)"));
            bInsert = false;
            break;
        }
    }

    if (bInsert)
    {
        player_log(kLogLevelTrace, _T("CodecsList::Insert %d %d 0x%016I64X '%s'"), group, exactmatch, info->merit,
            info->name.IsEmpty() ? info->clsid : info->name);

        codecs_t f = {(int)m_codecs.GetCount(), info, group, exactmatch, autodelete, preferred};
        m_codecs.AddTail(f);

        m_sortedcodecs.RemoveAll();
    }
    else if (autodelete)
    {
        delete info;
    }
}

POSITION CodecsList::GetHeadPosition()
{
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
//             if (sort[i].info->enable)
//             {
//                 m_sortedcodecs.AddTail(sort[i].info);
//             }
        }
    }

#ifdef _DEBUG
    player_log(kLogLevelTrace, _T("Sorting filters:"));

    POSITION pos = m_sortedcodecs.GetHeadPosition();
    while (pos)
    {
        CodecsInfo* info = m_sortedcodecs.GetNext(pos);
        //TRACE(_T("FGM: - %016I64x '%s'\n"), pFGF->GetMerit(), pFGF->GetName().IsEmpty() ? CStringFromGUID(pFGF->GetCLSID()) : CString(pFGF->GetName()));
        player_log(kLogLevelTrace, _T("filter - %s, %s, 0x%016I64x"), info->name, info->clsid, info->merit);
    }
#endif

    return m_sortedcodecs.GetHeadPosition();
}

CodecsInfo* CodecsList::GetNext(POSITION& pos)
{
    return m_sortedcodecs.GetNext(pos);
}

int CodecsList::codecs_cmp(const void* a, const void* b)
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

    if ((int)fa->info->type < (int)fb->info->type) {
        return -1;
    }
    if ((int)fa->info->type > (int)fb->info->type) {
        return +1;
    }

    if (fa->preferred && !fb->preferred) {
        return -1;
    }
    if (!fa->preferred && fb->preferred) {
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

void CodecsListEx::Insert(CodecsInfo* info, int group, int exactmatch, int custom_priority)
{
    bool bInsert = true;

    POSITION pos = m_codecs.GetHeadPosition();
    while (pos)
    {
        codecs_t& f = m_codecs.GetNext(pos);

        if (info == f.info)
        {
            player_log(kLogLevelTrace, _T("CodecsList::Insert, Rejected (exact duplicate)"));
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
        player_log(kLogLevelTrace, _T("CodecsList::Insert [%d] [%d] [%d] [%d] '%s'"),
            group, exactmatch, custom_priority, info->priority, 
            info->name.IsEmpty() ? info->clsid : info->name);

        codecs_t f = {(int)m_codecs.GetCount(), info, group, exactmatch, custom_priority};
        m_codecs.AddTail(f);

        m_sortedcodecs.RemoveAll();
    }
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

    player_log(kLogLevelTrace, _T("Sorting filters:"));
    POSITION pos = m_sortedcodecs.GetHeadPosition();
    int i = 0;
    while (pos)
    {
        CodecsInfo* info = m_sortedcodecs.GetNext(pos);
        player_log(kLogLevelTrace, _T("Filter[%d] - %s, %s, %d"), i++, info->name, info->clsid, info->priority);
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

//     if ((int)fa->info->type < (int)fb->info->type) {
//         return -1;
//     }
//     if ((int)fa->info->type > (int)fb->info->type) {
//         return +1;
//     }

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
