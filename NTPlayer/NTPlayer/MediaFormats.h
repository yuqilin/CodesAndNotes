#pragma once

#include <atlcoll.h>
#include "BaseGraph.h"


class CMediaFormatCategory
{
protected:
    CString m_label, m_description, m_specreqnote;
    CAtlList<CString> m_exts, m_backupexts;
    bool m_fAudioOnly;
    engine_t m_engine;
    bool m_fAssociable;

public:
    CMediaFormatCategory();
    CMediaFormatCategory(
        CString label, CString description, CAtlList<CString>& exts, bool fAudioOnly = false,
        CString specreqnote = _T(""), engine_t e = DirectShow, bool fAssociable = true);
    CMediaFormatCategory(
        CString label, CString description, CString exts, bool fAudioOnly = false,
        CString specreqnote = _T(""), engine_t e = DirectShow, bool fAssociable = true);
    virtual ~CMediaFormatCategory();

    void UpdateData(bool fSave);

    CMediaFormatCategory(const CMediaFormatCategory& mfc);
    CMediaFormatCategory& operator = (const CMediaFormatCategory& mfc);

    void RestoreDefaultExts();
    void SetExts(CAtlList<CString>& exts);
    void SetExts(CString exts);

    bool FindExt(CString ext) const {
        return m_exts.Find(ext.TrimLeft(_T('.')).MakeLower()) != NULL;
    }

    CString GetLabel() const { return m_label; }

    CString GetDescription() const { return m_description; }
    CString GetFilter() const;
    CString GetExts(bool fAppendEngine = false) const;
    CString GetExtsWithPeriod(bool fAppendEngine = false) const;
    CString GetBackupExtsWithPeriod(bool fAppendEngine = false) const;
    CString GetSpecReqNote() const { return m_specreqnote; }
    bool IsAudioOnly() const { return m_fAudioOnly; }
    bool IsAssociable() const { return m_fAssociable; }
    engine_t GetEngineType() const { return m_engine; }
    void SetEngineType(engine_t e) { m_engine = e; }
};

class CMediaFormats : public CAtlArray<CMediaFormatCategory>
{
protected:
    engine_t m_iRtspHandler;
    bool m_fRtspFileExtFirst;

public:
    CMediaFormats();
    virtual ~CMediaFormats();

    //CMediaFormats(const CMediaFormats& mf) { *this = mf; }
    CMediaFormats& operator=(const CMediaFormats& mf) {
        m_iRtspHandler = mf.m_iRtspHandler;
        m_fRtspFileExtFirst = mf.m_fRtspFileExtFirst;

        Copy(mf);

        return *this;
    }

    void UpdateData(bool fSave);

    engine_t GetRtspHandler(bool& fRtspFileExtFirst) const;
    void SetRtspHandler(engine_t e, bool fRtspFileExtFirst);

    bool IsUsingEngine(CString path, engine_t e) const;
    engine_t GetEngine(CString path) const;

    bool FindExt(CString ext, bool fAudioOnly = false, bool fAssociableOnly = true) const;
    const CMediaFormatCategory* FindMediaByExt(CString ext, bool fAudioOnly = false, bool fAssociableOnly = true) const;

    void GetFilter(CString& filter, CAtlArray<CString>& mask) const;
    void GetAudioFilter(CString& filter, CAtlArray<CString>& mask) const;

    static bool IsExtHidden();
};
