#pragma once

#include "IMYMFT.h"
#include "CSampleQueue.h"
#include <Mfidl.h>

#define MFT_MAX_STREAMS             1
#define MFT_OUTPUT_WIDTH            1280
#define MFT_OUTPUT_HEIGHT           720
#define MFT_FRAMERATE_NUMERATOR     30
#define MFT_FRAMERATE_DENOMINATOR   1
#define MFT_DEFAULT_SAMPLE_DURATION 300000 // 1/30th of a second in hundred nanoseconds

#define MFT_MAX_OUTPUT_STOP_NEED_INPUT    5 // Stop sending need input when output equal max value

enum eMFTStatus
{
    MYMFT_STATUS_INPUT_ACCEPT_DATA      = 0x00000001,   /* The MFT can accept input data */
    MYMFT_STATUS_OUTPUT_SAMPLE_READY    = 0x00000002,
    MYMFT_STATUS_STREAM_STARTED         = 0x00000004,
    MYMFT_STATUS_DRAINING               = 0x00000008,   /* See http://msdn.microsoft.com/en-us/library/dd940418(v=VS.85).aspx
                                                        ** While the MFT is is in this state, it must not send
                                                        ** any more METransformNeedInput events. It should continue
                                                        ** to send METransformHaveOutput events until it is out of
                                                        ** output. At that time, it should send METransformDrainComplete */
};

extern  const   GUID*   g_ppguidInputTypes[];
extern  const   DWORD   g_dwNumInputTypes;
extern  const   GUID*   g_ppguidOutputTypes[];
extern  const   DWORD   g_dwNumOutputTypes;
extern  const   GUID    MYMFT_MFSampleExtension_Marker;

class CHWMFT: 
    public IMFTransform,
    public IMFShutdown,
    public IMFMediaEventGenerator,
    public IMFAsyncCallback,
    public IMYMFT
    /*****************************
    ** Todo: if you add interfaces
    ** ensure that you add them 
    ** to QueryInterface
    *****************************/
{
public:
    static  volatile    ULONG   m_ulNumObjects;                 // Total object count

    // Initializer
    static  HRESULT     CreateInstance(IMFTransform** ppHWMFT);

#pragma region IUnknown
    // IUnknown Implementations
    ULONG   __stdcall   AddRef(void);
    HRESULT __stdcall   QueryInterface(
                                REFIID  riid,
                                void**  ppvObject
                                );
    ULONG   __stdcall   Release(void);
#pragma endregion IUnknown

#pragma region IMFTransform
    // IMFTransform Implementations
    HRESULT __stdcall   AddInputStreams(
                                  DWORD   dwStreams,
                                  DWORD*  pdwStreamIDs
                                  );
    HRESULT __stdcall   DeleteInputStream(
                                  DWORD   dwStreamID
                                  );
    HRESULT __stdcall   GetAttributes(
                                  IMFAttributes** ppAttributes
                                  );
    HRESULT __stdcall   GetInputAvailableType(
                                  DWORD           dwInputStreamID,
                                  DWORD           dwTypeIndex,
                                  IMFMediaType**  ppType
                                  );
    HRESULT __stdcall   GetInputCurrentType(
                                  DWORD           dwInputStreamID,
                                  IMFMediaType**  ppType
                                  );
    HRESULT __stdcall   GetInputStatus(
                                  DWORD   dwInputStreamID,
                                  DWORD*  pdwFlags
                                  );
    HRESULT __stdcall   GetInputStreamAttributes(
                                  DWORD           dwInputStreamID,
                                  IMFAttributes** ppAttributes
                                  );
    HRESULT __stdcall   GetInputStreamInfo(
                                  DWORD                   dwInputStreamID,
                                  MFT_INPUT_STREAM_INFO*  pStreamInfo
                                  );
    HRESULT __stdcall   GetOutputAvailableType(
                                  DWORD           dwOutputStreamID,
                                  DWORD           dwTypeIndex,
                                  IMFMediaType**  ppType
                                  );
    HRESULT __stdcall   GetOutputCurrentType(
                                  DWORD           dwOutputStreamID,
                                  IMFMediaType**  ppType
                                  );
    HRESULT __stdcall   GetOutputStatus(
                                  DWORD*  pdwFlags
                                  );
    HRESULT __stdcall   GetOutputStreamAttributes(
                                  DWORD           dwOutputStreamID,
                                  IMFAttributes** ppAttributes
                                  );
    HRESULT __stdcall   GetOutputStreamInfo(
                                  DWORD                   dwOutputStreamID,
                                  MFT_OUTPUT_STREAM_INFO* pStreamInfo
                                  );
    HRESULT __stdcall   GetStreamCount(
                                  DWORD*  pdwInputStreams,
                                  DWORD*  pdwOutputStreams
                                  );
    HRESULT __stdcall   GetStreamIDs(
                                  DWORD   dwInputIDArraySize,
                                  DWORD*  pdwInputIDs,
                                  DWORD   dwOutputIDArraySize,
                                  DWORD*  pdwOutputIDs
                                  );
    HRESULT __stdcall   GetStreamLimits(
                                  DWORD*  pdwInputMinimum,
                                  DWORD*  pdwInputMaximum,
                                  DWORD*  pdwOutputMinimum,
                                  DWORD*  pdwOutputMaximum
                                  );
    HRESULT __stdcall   ProcessEvent(
                                  DWORD           dwInputStreamID,
                                  IMFMediaEvent*  pEvent
                                  );
    HRESULT __stdcall   ProcessInput(
                                  DWORD       dwInputStreamID,
                                  IMFSample*  pSample,
                                  DWORD       dwFlags
                                  );
    HRESULT __stdcall   ProcessMessage(
                                  MFT_MESSAGE_TYPE eMessage,
                                  ULONG_PTR ulParam
                                  );
    HRESULT __stdcall   ProcessOutput(
                                  DWORD                   dwFlags,
                                  DWORD                   dwOutputBufferCount,
                                  MFT_OUTPUT_DATA_BUFFER* pOutputSamples,
                                  DWORD*                  pdwStatus
                                  );
    HRESULT __stdcall   SetInputType(
                                  DWORD           dwInputStreamID,
                                  IMFMediaType*   pType,
                                  DWORD           dwFlags
                                  );
    HRESULT __stdcall   SetOutputBounds(
                                  LONGLONG hnsLowerBound,
                                  LONGLONG hnsUpperBound
                                  );
    HRESULT __stdcall   SetOutputType(
                                DWORD           dwOutputStreamID,
                                IMFMediaType*   pType,
                                DWORD           dwFlags
                                );
#pragma endregion IMFTransform

#pragma region IMFShutdown
    // IMFShutdown Implementations
    HRESULT __stdcall   GetShutdownStatus(
                                MFSHUTDOWN_STATUS*  pStatus
        );
    HRESULT __stdcall   Shutdown(void);
#pragma endregion IMFShutdown

#pragma region IMFMediaEventGenerator
    // IMFMediaEventGenerator Implementations
    HRESULT __stdcall   BeginGetEvent(
                                IMFAsyncCallback*   pCallback,
                                IUnknown*           punkState
                                );
    HRESULT __stdcall   EndGetEvent(
                                IMFAsyncResult* pResult,
                                IMFMediaEvent** ppEvent
                                );
    HRESULT __stdcall   GetEvent(
                                DWORD           dwFlags,
                                IMFMediaEvent** ppEvent
                                );
    HRESULT __stdcall   QueueEvent(
                                MediaEventType      met,
                                REFGUID             guidExtendedType,
                                HRESULT             hrStatus,
                                const PROPVARIANT*  pvValue
                                );
#pragma endregion IMFMediaEventGenerator

#pragma region IMFAsyncCallback
    // IMFAsyncCallback Implementations
    HRESULT __stdcall   GetParameters(
                                DWORD*  pdwFlags,
                                DWORD*  pdwQueue
                                );
    HRESULT __stdcall   Invoke(
                                IMFAsyncResult *pAsyncResult
                                );
#pragma endregion IMFAsyncCallback

    // IMYMFT Implementations
    HRESULT __stdcall   DecodeInputFrame(
                                IMFSample*  pInputSample
                                );

protected:
                        CHWMFT(void);
                        ~CHWMFT(void);

    HRESULT             InitializeTransform(void);
    HRESULT             CheckInputType(
                                IMFMediaType*   pMT
                                );
    HRESULT             CheckOutputType(
                                IMFMediaType*   pMT
                                );
    HRESULT             ShutdownEventQueue(void);

    /******* MFT Media Event Handlers**********/
    HRESULT             OnStartOfStream(void);
    HRESULT             OnEndOfStream(void);
    HRESULT             OnDrain(
                                const UINT32 un32StreamID
                                );
    HRESULT             OnFlush(void);
    HRESULT             OnMarker(
                                const ULONG_PTR pulID
                                );
    /***********End Event Handlers************/
    HRESULT             RequestSample(
                                const UINT32 un32StreamID
                                );
    HRESULT             FlushSamples(void);
    HRESULT             ScheduleFrameDecode(void);
    HRESULT             SendEvent(void);
    BOOL                IsLocked(void);
    BOOL                IsMFTReady(void);

    // Member variables
            volatile    ULONG               m_ulRef;
            volatile    ULONG               m_ulSampleCounter;
                        IMFMediaType*       m_pInputMT;
                        IMFMediaType*       m_pOutputMT;
                        IMFAttributes*      m_pAttributes;
                        IMFMediaEventQueue* m_pEventQueue;
                        DWORD               m_dwStatus;
                        DWORD               m_dwNeedInputCount;
                        DWORD               m_dwHaveOutputCount;
                        DWORD               m_dwDecodeWorkQueueID;
                        LONGLONG            m_llCurrentSampleTime;
                        BOOL                m_bFirstSample;
                        BOOL                m_bShutdown;
                        BOOL                m_bDXVA;
                        CSampleQueue*       m_pInputSampleQueue;
                        CSampleQueue*       m_pOutputSampleQueue;
                        CRITICAL_SECTION    m_csLock;
};

HRESULT DuplicateAttributes(
                        IMFAttributes*  pDest,
                        IMFAttributes*  pSource
                        );
