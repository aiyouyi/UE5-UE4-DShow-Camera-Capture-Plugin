// Created by wangpeimin@corp.netease.com


#include "DShowCameraComponent.h"
#include "LibYUVWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#include<vector>
#include<string>
#include <DShow.h>
#pragma comment(lib,"Strmiids.lib")


#define ReleaseInterface(x) \
	if ( NULL != x ) \
{ \
	x->Release( ); \
	x = NULL; \
}


struct IntVector2D
{
	IntVector2D()
	{
		IntVector2D(0, 0);
	}
	IntVector2D(int _X, int _Y)
	{
		X = _X;
		Y = _Y;
	}
	int32 X;
	int32 Y;
};

interface
	ISampleGrabberCB
	:
	public IUnknown
{
	virtual STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample) = 0;
	virtual STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) = 0;
};

///////////////////////////////////////////////////////////////////////////////////

static const IID IID_ISampleGrabberCB = { 0x0579154A, 0x2B53, 0x4994, { 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };

///////////////////////////////////////////////////////////////////////////////////

interface
	ISampleGrabber
	:
	public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE* pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE* pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long* pBufferSize, long* pBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample** ppSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB* pCallback, long WhichMethodToCallback) = 0;
};

///////////////////////////////////////////////////////////////////////////////////

static const IID IID_ISampleGrabber = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };

static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const CLSID CLSID_NullRenderer = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const CLSID CLSID_VideoEffects1Category = { 0xcc7bfb42, 0xf175, 0x11d1, { 0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59 } };
static const CLSID CLSID_VideoEffects2Category = { 0xcc7bfb43, 0xf175, 0x11d1, { 0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59 } };
static const CLSID CLSID_AudioEffects1Category = { 0xcc7bfb44, 0xf175, 0x11d1, { 0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59 } };
static const CLSID CLSID_AudioEffects2Category = { 0xcc7bfb45, 0xf175, 0x11d1, { 0xa3, 0x92, 0x0, 0xe0, 0x29, 0x1f, 0x39, 0x59 } };

class ISampleGrabberCallback : public ISampleGrabberCB
{
public:
	//typedef void (*ReceiveDataCallBack)(BYTE* pBuffer, long BufferLen ,int width, int height, void* pCaptureContext);

	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

	HRESULT STDMETHODCALLTYPE SampleCB(double Time, IMediaSample* pSample);
	HRESULT STDMETHODCALLTYPE BufferCB(double Time, BYTE* pBuffer, long BufferLen);

	ISampleGrabberCallback();
	virtual ~ISampleGrabberCallback() {}
public:
	long Width;
	long Height;
	int  BitCount;    //the number of bits per pixel (bpp)
	class DSCamera* Camera;
};


class DSCamera
{
public:
	DSCamera();
	~DSCamera();
	HRESULT InitializeEnv(); //initialize environment
	void CloseInterface(); //close all interface
	HRESULT OpenDevice(const std::wstring& DeviceName, int Width = 1920, int Height = 1080, int FPS = 60);
	HRESULT BindFilter(int deviceID, IBaseFilter** pBaseFilter);

	GUID GetCapturePreferredMediaType(int32 Width = 1920, int32 Height = 1080);
	AM_MEDIA_TYPE* GetMediaTypeInList(std::vector<AM_MEDIA_TYPE*>& listType, long width, long height, DWORD fourCC);
	AM_MEDIA_TYPE* GetMediaTypeInList(std::vector<AM_MEDIA_TYPE*>& listType, long width, long height, GUID subtype);

	void FreeMediaType(std::vector<AM_MEDIA_TYPE*>& listType);
	void ReceiveDataCallBack(BYTE* pBuffer, long BufferLen, int width, int height);

	//core::event<void(int Result, unsigned int Width, unsigned int Height, char* Data, const char* ErrorStr, bool fromlocalCamera)> ReceiveDataCallBack;
	GUID ExpectedMediaType;
	void GetDeviceList(std::vector<std::wstring>& Devices);
private:
	IGraphBuilder* GraphBuilder;
	ICaptureGraphBuilder2* CaptureGraphBuilder2;
	IMediaControl* MediaControl;
	IBaseFilter* DeviceFilter;
	ISampleGrabber* SampGrabber;
	IMediaEventEx* MediaEventEx;

	ISampleGrabberCallback SampleGrabberCallback;

	void SetBuffer(int Length);
	char* Buffer;
	int BufferLength;
public:
	bool bIsOpened;


	FBufferCallBack BufferCallBack;

	static void FreeMediaType(AM_MEDIA_TYPE& MediaType)
	{
		if (MediaType.cbFormat != 0) {
			CoTaskMemFree((LPVOID)MediaType.pbFormat);
			MediaType.cbFormat = 0;
			MediaType.pbFormat = nullptr;
		}

		if (MediaType.pUnk) {
			MediaType.pUnk->Release();
			MediaType.pUnk = nullptr;
		}
	}

	static void DeleteMediaType(AM_MEDIA_TYPE* MediaType)
	{
		if (MediaType != NULL)
		{
			DSCamera::FreeMediaType(*MediaType);
			CoTaskMemFree(MediaType);
		}
	}
};


ISampleGrabberCallback::ISampleGrabberCallback()
{
}

ULONG STDMETHODCALLTYPE ISampleGrabberCallback::AddRef()
{
	return 1;
}

ULONG STDMETHODCALLTYPE ISampleGrabberCallback::Release()
{
	return 2;
}

HRESULT STDMETHODCALLTYPE ISampleGrabberCallback::QueryInterface(REFIID riid, void** ppvObject)
{
	if (NULL == ppvObject) return E_POINTER;
	if (riid == __uuidof(IUnknown))
	{
		*ppvObject = static_cast<IUnknown*>(this);
		return S_OK;
	}
	if (riid == IID_ISampleGrabberCB)
	{
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}
	return E_NOTIMPL;

}

HRESULT STDMETHODCALLTYPE ISampleGrabberCallback::SampleCB(double Time, IMediaSample* pSample)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ISampleGrabberCallback::BufferCB(double Time, BYTE* pBuffer, long BufferLen)
{
	if (Camera)
	{
		Camera->ReceiveDataCallBack(pBuffer, BufferLen, Width, Height);
	}
	return S_OK;
}




const GUID MEDIASUBTYPE_I420 =
{ 0x30323449, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };


HRESULT SetCaptureResolution(ICaptureGraphBuilder2* CaptureGraphBuilder2, IBaseFilter* DeviceFilter, IntVector2D Resolution, UINT64 FrameInterval, GUID ExpectedMediaType)
{
	IAMStreamConfig* AMStreamConfig;

	HRESULT hResult = CaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, DeviceFilter, IID_IAMStreamConfig, (void**)&AMStreamConfig);

	if (S_OK != hResult || !AMStreamConfig)
	{
		return hResult;
	}

	int iCount = 0, iSize = 0;
	hResult = AMStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (S_OK != hResult)
	{
		return hResult;
	}

	hResult = E_FAIL;
	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// Use the video capabilities structure.
		std::vector<AM_MEDIA_TYPE*> vecMediaTypes;
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS StreanConfigCaps;
			AM_MEDIA_TYPE* MediaType;
			HRESULT hr = AMStreamConfig->GetStreamCaps(iFormat, &MediaType, (BYTE*)&StreanConfigCaps);
			if (SUCCEEDED(hr))
			{
				if ((MediaType->majortype == MEDIATYPE_Video) &&
					(MediaType->subtype == ExpectedMediaType) &&
					(MediaType->formattype == FORMAT_VideoInfo) &&
					(MediaType->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
					(MediaType->pbFormat != NULL))
				{
					VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)MediaType->pbFormat;
					// pVih contains the detailed format information.
					LONG lWidth = pVih->bmiHeader.biWidth;
					LONG lHeight = pVih->bmiHeader.biHeight;

					if (lWidth == Resolution.X && lHeight == Resolution.Y)
					{
						hResult = S_OK;
						pVih->AvgTimePerFrame = FrameInterval;
						hResult = AMStreamConfig->SetFormat(MediaType);
						DSCamera::DeleteMediaType(MediaType);
						break;
					}
				}
				DSCamera::DeleteMediaType(MediaType);
			}
		}
		return hResult;
	}
	return E_FAIL;
}


HRESULT SelectVideoDeviceByName(const std::wstring& strName, IBaseFilter** pOutFilter)
{
	HRESULT hr = S_OK;
	*pOutFilter = NULL;
	ICreateDevEnum* pCreateDevEnum;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (LPVOID*)&pCreateDevEnum);
	if (!pCreateDevEnum || FAILED(hr))
	{
		return hr;
	}

	IEnumMoniker* pEnumMoniker;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMoniker, 0);

	if (!pEnumMoniker || FAILED(hr))
	{
		return hr;
	}

	pEnumMoniker->Reset();

	ULONG ulFetched = 0;
	IMoniker* pMoniker;
	hr = S_OK;

	while ((hr = pEnumMoniker->Next(1, &pMoniker, &ulFetched)) == S_OK)
	{
		IPropertyBag* pPropertyBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropertyBag);
		if (hr != S_OK || !pPropertyBag)
		{
			continue;
		}

		VARIANT devVar;
		::VariantInit(&devVar);
		devVar.vt = VT_BSTR;
		hr = pPropertyBag->Read(L"FriendlyName", &devVar, NULL);
		if (hr != S_OK)
		{
			continue;
		}
		std::wstring FriendlyName = devVar.bstrVal;
		::VariantClear(&devVar);
		if (FriendlyName == strName)
		{
			//IBaseFilter* pCap;
			hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)pOutFilter);
			return S_OK;
		}
		pMoniker->Release();
	}
	return E_FAIL;
}


DSCamera::DSCamera()
{
	//COM Library Initialize
	if (FAILED(CoInitialize(NULL)))
	{
		return;
	}
	//initialize member variable
	DeviceFilter = NULL;
	CaptureGraphBuilder2 = NULL;
	GraphBuilder = NULL;
	MediaControl = NULL;
	MediaEventEx = NULL;
	SampGrabber = NULL;
	bIsOpened = false;

	Buffer = nullptr;
	BufferLength = 0;

	SampleGrabberCallback.Camera = this;
	InitializeEnv();	
}

DSCamera::~DSCamera()
{
	CloseInterface();
	CoUninitialize();
	SetBuffer(0);
}

HRESULT DSCamera::InitializeEnv()
{
	HRESULT hr;

	//Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (LPVOID*)&GraphBuilder);
	if (FAILED(hr))	return hr;

	//Create the capture graph builder
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2, (LPVOID*)&CaptureGraphBuilder2);
	if (FAILED(hr))	return hr;

	//Obtain interfaces for media control and Video Window
	hr = GraphBuilder->QueryInterface(IID_IMediaControl, (LPVOID*)&MediaControl);
	if (FAILED(hr))	return hr;

	hr = GraphBuilder->QueryInterface(IID_IMediaEventEx, (LPVOID*)&MediaEventEx);
	if (FAILED(hr))	return hr;

	CaptureGraphBuilder2->SetFiltergraph(GraphBuilder);
	if (FAILED(hr))	return hr;

	return hr;
}

void DSCamera::CloseInterface()
{
	if (MediaControl)	MediaControl->Stop();
	if (MediaEventEx)	MediaEventEx->SetNotifyWindow(NULL, WM_APP + 100, 0);
	//release interface
	ReleaseInterface(DeviceFilter);
	ReleaseInterface(CaptureGraphBuilder2);
	ReleaseInterface(GraphBuilder);
	ReleaseInterface(MediaControl);
	ReleaseInterface(MediaEventEx);
	ReleaseInterface(SampGrabber);
}

HRESULT DSCamera::BindFilter(int deviceID, IBaseFilter** pBaseFilter)
{
	ICreateDevEnum* pDevEnum;
	IEnumMoniker* pEnumMon;
	IMoniker* pMoniker;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (LPVOID*)&pDevEnum);
	if (SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumMon, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;
			return hr;
		}
		pEnumMon->Reset();
		ULONG cFetched;
		int index = 0;
		while (hr = pEnumMon->Next(1, &pMoniker, &cFetched), hr == S_OK, index <= deviceID)
		{
			IPropertyBag* pProBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (LPVOID*)&pProBag);
			if (SUCCEEDED(hr))
			{
				if (index == deviceID)
				{
					pMoniker->BindToObject(0, 0, IID_IBaseFilter, (LPVOID*)pBaseFilter);
				}
			}
			pMoniker->Release();
			index++;
		}
		pEnumMon->Release();
	}
	return hr;
}

HRESULT DSCamera::OpenDevice(const std::wstring& DeviceName, int Width/*=1920*/, int Height/*=1080*/, int FPS/*=60*/)
{
	HRESULT hr;
	IBaseFilter* pSampleGrabberFilter;
	if (bIsOpened)
	{
		CloseInterface();
		InitializeEnv();
	}

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID*)&pSampleGrabberFilter);
	if (FAILED(hr))	return hr;
	//bind device filter
	hr = SelectVideoDeviceByName(DeviceName, &DeviceFilter);
	if (FAILED(hr))	return hr;
	hr = GraphBuilder->AddFilter(DeviceFilter, L"Video Filter");
	if (FAILED(hr))	return hr;

	hr = GraphBuilder->AddFilter(pSampleGrabberFilter, L"Sample Grabber");
	if (FAILED(hr))	return hr;

	hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (LPVOID*)&SampGrabber);
	if (FAILED(hr))	return hr;

	//set media type
	AM_MEDIA_TYPE MediaType;
	ZeroMemory(&MediaType, sizeof(AM_MEDIA_TYPE));
	//Find the current bit depth
	HDC hdc = GetDC(NULL);
	int BitDepth = GetDeviceCaps(hdc, BITSPIXEL);
	SampleGrabberCallback.BitCount = BitDepth;
	ReleaseDC(NULL, hdc);
	//Set the media type
	MediaType.majortype = MEDIATYPE_Video;
	switch (BitDepth)
	{
	case  8:
		MediaType.subtype = MEDIASUBTYPE_RGB8;
		break;
	case 16:
		MediaType.subtype = MEDIASUBTYPE_RGB555;
		break;
	case 24:
		MediaType.subtype = MEDIASUBTYPE_RGB24;
		break;
	case 32:
		MediaType.subtype = MEDIASUBTYPE_RGB32;
		break;
	default:
		return E_FAIL;
	}
	MediaType.formattype = FORMAT_VideoInfo;

	UINT64 FrameInterval = UINT64(10000000.0 / double(60));
	ExpectedMediaType = GetCapturePreferredMediaType(Width,Height);
	SetCaptureResolution(CaptureGraphBuilder2, DeviceFilter, IntVector2D(Width, Height), FrameInterval, ExpectedMediaType);

	hr = CaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, DeviceFilter, NULL, pSampleGrabberFilter);
	if (FAILED(hr))	return hr;

	hr = SampGrabber->GetConnectedMediaType(&MediaType);
	if (FAILED(hr))	return hr;


	VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)MediaType.pbFormat;
	SampleGrabberCallback.Width = vih->bmiHeader.biWidth;
	SampleGrabberCallback.Height = vih->bmiHeader.biHeight;
	// Configure the Sample Grabber
	hr = SampGrabber->SetOneShot(FALSE);
	if (FAILED(hr))	return hr;
	hr = SampGrabber->SetBufferSamples(TRUE);
	if (FAILED(hr))	return hr;
	// 1 = Use the BufferCB callback method.
	hr = SampGrabber->SetCallback(&SampleGrabberCallback, 1);

	hr = MediaControl->Run();
	if (FAILED(hr))	return hr;

	if (MediaType.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)MediaType.pbFormat);
		MediaType.cbFormat = 0;
		MediaType.pbFormat = NULL;
	}
	if (MediaType.pUnk != NULL)
	{
		MediaType.pUnk->Release();
		MediaType.pUnk = NULL;
	}
	bIsOpened = true;
	return hr;
}


GUID DSCamera::GetCapturePreferredMediaType(int32 Width , int32 Height )
{
	if (NULL == CaptureGraphBuilder2)
	{
		return MEDIASUBTYPE_None;
	}
	int nCount(0);
	int nSize(0);
	IAMStreamConfig* pConfig(NULL);
	HRESULT hr = CaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, DeviceFilter, IID_IAMStreamConfig, (void**)&pConfig);
	if (FAILED(hr))
	{
		return MEDIASUBTYPE_None;
	}
	hr = pConfig->GetNumberOfCapabilities(&nCount, &nSize);
	if (sizeof(VIDEO_STREAM_CONFIG_CAPS) != nSize)
	{
		return MEDIASUBTYPE_None;
	}
	VIDEO_STREAM_CONFIG_CAPS scc;
	AM_MEDIA_TYPE* pmt = NULL;
	std::vector<AM_MEDIA_TYPE*>	videoSupportMediaTypeList;
	for (int i = 0; i < nCount; i++)
	{
		pmt = NULL;
		hr = pConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE*>(&scc));
		if (FAILED(hr))
		{
			continue;
		}
		videoSupportMediaTypeList.push_back(pmt);
	}

	AM_MEDIA_TYPE* preferredAM = NULL;

	std::vector<std::pair<GUID, std::wstring> > mediaSubTypeList;
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_MJPG, L"MJPG"));
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_YUY2, L"YUY2"));
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_YV12, L"YV12"));
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_I420, L"I420"));
	//mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_RGB32, L"RGB32"));
	//mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_RGB24, L"RGB24"));	

	for (size_t index = 0; index < mediaSubTypeList.size(); ++index)
	{
		preferredAM = GetMediaTypeInList(videoSupportMediaTypeList, Width, Height, mediaSubTypeList[index].first);
		if (preferredAM)
		{
			GUID type = preferredAM->subtype;
			FreeMediaType(videoSupportMediaTypeList);
			return type;
		}
	}

	preferredAM = GetMediaTypeInList(videoSupportMediaTypeList, Width, Height, MAKEFOURCC('H', 'D', 'Y', 'C'));
	if (preferredAM)
	{
		GUID type = preferredAM->subtype;
		FreeMediaType(videoSupportMediaTypeList);
		return type;
	}
	FreeMediaType(videoSupportMediaTypeList);
	return MEDIASUBTYPE_RGB24;
}

void DSCamera::SetBuffer(int Length)
{
	if (Length==0)
	{
		Buffer ? delete[] Buffer : 0;
		Buffer = nullptr;
	}
	else
	{
		if (BufferLength<Length)
		{
			Buffer ? delete[] Buffer : 0;
			Buffer = new char[Length];			
		}
	}
	BufferLength = Length;
}

void DSCamera::FreeMediaType(std::vector<AM_MEDIA_TYPE*>& listType)
{
	std::vector<AM_MEDIA_TYPE*>::iterator it = listType.begin();
	for (; it != listType.end(); it++)
	{
		AM_MEDIA_TYPE* ptr = (*it);
		if (ptr) {
			::DSCamera::FreeMediaType(*ptr);
			ptr = NULL;
		}
	}
}


void DSCamera::ReceiveDataCallBack(BYTE* pBuffer, long BufferLen, int width, int height)
{

	uint32_t format = 0;
	if (ExpectedMediaType == MEDIASUBTYPE_YV12)
	{
		format = libyuv::FOURCC_YV12;
	}
	else if (ExpectedMediaType == MEDIASUBTYPE_MJPG)
	{
		format = libyuv::FOURCC_MJPG;
	}
	else if (ExpectedMediaType == MEDIASUBTYPE_YUY2)
	{
		format = libyuv::FOURCC_YUY2;
	}
	else if (ExpectedMediaType == MEDIASUBTYPE_I420)
	{
		format = libyuv::FOURCC_I420;
	}

	SetBuffer(width * height * 4);
	if (format != 0)
	{
		int ret = LibYUVWrapper::LibYUVConvertToARGB((unsigned char*)pBuffer, BufferLen, (uint8_t*)Buffer, width * 4,
			0, 0, width, height,
			width, height,
			libyuv::kRotate0, format);
	}
	else
	{
		memcpy_s(Buffer, BufferLen, pBuffer, BufferLen);
	}

	FCameraDataBuffer CameraDataBuffer((uint8*)Buffer, width, height);
	BufferCallBack.Broadcast(CameraDataBuffer);
}




void DSCamera::GetDeviceList(std::vector<std::wstring>& Devices)
{
	Devices.clear();
	HRESULT hr = S_OK;
	ICreateDevEnum* pCreateDevEnum;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,IID_ICreateDevEnum, (LPVOID*)&pCreateDevEnum);
	if (!pCreateDevEnum || FAILED(hr))
	{
		return;
	}

	IEnumMoniker* pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);

	if (!pEm || FAILED(hr))
	{
		return;
	}

	pEm->Reset();

	ULONG ulFetched = 0;
	IMoniker* pM;
	hr = S_OK;

	while ((hr = pEm->Next(1, &pM, &ulFetched)) == S_OK)
	{
		IPropertyBag* pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		if (hr != S_OK || !pBag)
		{
			continue;
		}

		VARIANT devVar;
		::VariantInit(&devVar);
		devVar.vt = VT_BSTR;
		hr = pBag->Read(L"FriendlyName", &devVar, NULL);
		if (hr != S_OK)
		{
			continue;
		}
		std::wstring friendlyName = devVar.bstrVal;
		::VariantClear(&devVar);
		Devices.push_back(friendlyName);
		pM->Release();
	}
}

static BITMAPINFOHEADER* GetVideoBMIHeader(const AM_MEDIA_TYPE* pMT)
{
	return (pMT->formattype == FORMAT_VideoInfo) ?
		&reinterpret_cast<VIDEOINFOHEADER*>(pMT->pbFormat)->bmiHeader :
		&reinterpret_cast<VIDEOINFOHEADER*>(pMT->pbFormat)->bmiHeader;
}

AM_MEDIA_TYPE* DSCamera::GetMediaTypeInList(std::vector<AM_MEDIA_TYPE*>& listType, long width, long height, DWORD fourCC)
{
	AM_MEDIA_TYPE* pAM = NULL;
	std::vector<AM_MEDIA_TYPE*>::iterator it = listType.begin();
	for (; it != listType.end(); it++)
	{
		VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)(*it)->pbFormat;
		BITMAPINFOHEADER* bmiHeader = GetVideoBMIHeader(*it);

		if (bmiHeader->biCompression == fourCC
			&& pvih->bmiHeader.biHeight == height
			&& pvih->bmiHeader.biWidth == width)
		{
			pAM = *it;
			break;
		}
	}
	return pAM;
}

AM_MEDIA_TYPE* DSCamera::GetMediaTypeInList(std::vector<AM_MEDIA_TYPE*>& listType, long width, long height, GUID subtype)
{
	AM_MEDIA_TYPE* pAM = NULL;
	std::vector<AM_MEDIA_TYPE*>::iterator it = listType.begin();
	for (; it != listType.end(); it++)
	{
		VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)(*it)->pbFormat;
		if ((*it)->subtype == subtype
			&& pvih->bmiHeader.biHeight == height
			&& pvih->bmiHeader.biWidth == width)
		{
			pAM = *it;
			break;
		}
	}
	return pAM;
}

// Sets default values for this component's properties
UDShowCameraComponent::UDShowCameraComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Camera = nullptr;
	bShowCameraPicture = true;
	CameraPicture = nullptr;
	// ...
}


// Called when the game starts
void UDShowCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	Camera = new class DSCamera();
	Camera->BufferCallBack.AddDynamic(this, &UDShowCameraComponent::OnReceivedDataBuffer);
	// ...
	
}


void UDShowCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete Camera;
	Camera = nullptr;
	Super::EndPlay(EndPlayReason);
	if (CameraPicture)
	{
		CameraPicture->RemoveFromRoot();
		CameraPicture->ConditionalBeginDestroy();
		CameraPicture = nullptr;
	}
}

// Called every frame
void UDShowCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UDShowCameraComponent::GetDeviceList(TArray<FString>& DeviceNames)
{
	DeviceNames.Empty();
	std::vector<std::wstring> wDeviceNames;
	Camera->GetDeviceList(wDeviceNames);
	for (auto &Device:wDeviceNames)
	{
		DeviceNames.Add(FString(Device.c_str()));
	}
}

void UDShowCameraComponent::OpenDevice(const FString& DeviceName, int32 Width, int32 Height, int32 FPS)
{
	Camera->OpenDevice(std::wstring(*DeviceName),Width,Height,FPS);
}

void UDShowCameraComponent::OnReceivedDataBuffer(const FCameraDataBuffer& CameraBuffer)
{
	UpdateCameraTexture(CameraBuffer);
	BufferCallBack.Broadcast(CameraBuffer);
}


void UDShowCameraComponent::UpdateCameraTexture(const FCameraDataBuffer& CameraBuffer)
{
	if (bShowCameraPicture)
	{
		if (CameraDataBuffer.DataWidth != CameraBuffer.DataWidth || CameraDataBuffer.DataHeight != CameraBuffer.DataHeight)
		{
			this->CameraPicture = UTexture2D::CreateTransient(CameraBuffer.DataWidth, CameraBuffer.DataHeight);
			this->CameraPicture->UpdateResource();
			this->CameraDataBuffer = CameraBuffer;
		}
		else if (CameraPicture)
		{
			CameraDataBuffer = CameraBuffer;
			FUpdateTextureRegion2D region(0, 0, 0, 0, CameraDataBuffer.DataWidth, CameraDataBuffer.DataHeight);
			this->CameraPicture->UpdateTextureRegions(0, 1, &region, CameraDataBuffer.DataWidth * 4, 4, (uint8*)CameraBuffer.DataBuffer);
		}
	}
}

#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"