
#pragma once


#define BeginEnumPins(pBaseFilter, pEnumPins, pPin)                                 \
{                                                                                   \
	CComPtr<IEnumPins> pEnumPins;                                                   \
	if (pBaseFilter && SUCCEEDED(pBaseFilter->EnumPins(&pEnumPins)))                \
	{                                                                               \
		for (CComPtr<IPin> pPin; S_OK == pEnumPins->Next(1, &pPin, 0); pPin = NULL) \
		{

#define EndEnumPins }}}


HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin);