
#ifndef _PALYERCORE_DSGRAPH_H_
#define _PALYERCORE_DSGRAPH_H_

class CDSGraph
{
public:
	CDSGraph();
	~CDSGraph();

public:
	HRESULT				Create(HWND hVideoWindow,
							   HWND hNotifyWindow,
							   CDSEngine* pEngine);
	void				Destory();

	HRESULT				OpenMedia(CMediaInfo* pMediaInfo);
	void				CloseMedia();

	// Graph build
	HRESULT				Render(BOOL bOpenChain);
	HRESULT				AddFilter(DSFilterInfo* pInfo,
						CAtlList<MediaTypeItem>& mts,
						IBaseFilter ** ppFilter = NULL);
	IBaseFilter *		FindFilter(const TCHAR * pcszClsId);
	BOOL				IsFilterExists(const TCHAR * pcszClsId);
	BOOL				GetFilterPin(IBaseFilter * pFilter, BOOL bOutPin, IPin ** ppPin);
	BOOL				GetPinsCount(IBaseFilter * pFilter,
									int * pnAll,
									int * pnIn,
									int * pnInConnected,
									int * pnOut,
									int * pnOutConnected);
	HRESULT				RemoveFilter(IBaseFilter * pFilter, const TCHAR * pcszClsId = NULL);
	void				RemoveAllFilter(void);
	//void				RemoveChain(IPin * pOutputPin);

	// Play Control
	BOOL				Run(void);
	BOOL				Stop(void);
	BOOL				Pause(void);
	void				Abort(BOOL bStop);
	BOOL				IsRunning(void);
	BOOL				IsStopped(void);
	BOOL				IsPaused(void);

protected:

	// Kernel Function
	void				Kernel_SortInfoByExtension();

	BOOL				Kernel_CheckBytes(DSFilterInfo* pInfo, CString& strSubtype);

	HRESULT				Kernel_RenderMediaTypes(IPin* pPinOut, CAtlList<MediaTypeItem>& mts);


private:
	void				EnterFunction();
	void				LeaveFunction();
	

protected:
	CMediaInfo*			m_pMediaInfo;
	HWND				m_hVideoWindow;
	HWND				m_hNotifyWindow;
	CDSEngine*			m_pEngine;

	BOOL				m_bVideoRendered;

	CComPtr<IFilterGraph>	m_pJFilterGraph;


private:
	int					m_nCallLevel;
	CString				m_strPrefix;

};

#endif