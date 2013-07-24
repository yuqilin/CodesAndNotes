#ifndef __NTLOG_SECTION_PROTECT_H__
#define __NTLOG_SECTION_PROTECT_H__

#pragma pack(push, 1)
class SectionProtect
{
public:
    SectionProtect(LPCRITICAL_SECTION lpCriticalSection, BOOL = FALSE) : m_lpCriticalSection(NULL)
    {
        if (NULL != lpCriticalSection)
        {
            EnterCriticalSection(lpCriticalSection);
            m_lpCriticalSection = lpCriticalSection;
        }
    }
    ~SectionProtect()
    {
        if (m_lpCriticalSection)
        {
            LeaveCriticalSection(m_lpCriticalSection);
            m_lpCriticalSection = NULL;
        }
    }
    void Submit()
    {
        if (m_lpCriticalSection)
        {
            LeaveCriticalSection(m_lpCriticalSection);
            m_lpCriticalSection = NULL;
        }
    }
private:
    LPCRITICAL_SECTION m_lpCriticalSection;
};
#pragma pack(pop)

#endif
