
#ifndef _DSHOW_GRAPH_H_
#define _DSHOW_GRAPH_H_

class CDShowGraph
{
public:
	CDShowGraph();
	~CDShowGraph();

public:
	HRESULT				Initialize(HWND hVideoWindow,
								   HWND hNotifyWindow,
								   CDShowEngine* pEngine);
	void				UnInitialize();

	// Graph build
	HRESULT				Render(BOOL bOpenChain);
	BOOL				BringInfoToTop(DShowFilterInfo* pInfo);
	HRESULT				AddFilter(DShowFilterInfo* pInfo,
						AM_MEDIA_TYPE** pmts = NULL,
						int nMediaTypeCount = 0,
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
	void				RemoveChain(IPin * pOutputPin);


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

	BOOL				Kernel_CheckBytes(DShowFilterInfo* pInfo, CString& strSubtype);


private:
	void				EnterFunction();
	void				LeaveFunction();
	

protected:
	CMediaInfo*			m_pMediaInfo;
	HWND				m_hVideoWindow;
	HWND				m_hNotifyWindow;
	CDShowEngine*		m_pEngine;
	CDShowFilters*		m_pFilters;

	CComPtr<IFilterGraph>	m_pFilterGraph;


private:
	int					m_nCallLevel;
	CString				m_strPrefix;

};

#endif