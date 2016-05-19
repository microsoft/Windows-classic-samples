/*--

Copyright (C) Microsoft Corporation, 2006

header file for utility functions

--*/

#define MILLISECONDS_FROM_SECONDS(x) ((x)*1000)
#define MILLISECONDS_TO_SECONDS(x) ((x)/1000)

#define SYNCHRONIZE_CACHE_TIMEOUT   (260)
#define DEFAULT_OPC_TIMEOUT         (260)

#define DISC_INFORMATION_MINIMUM_SIZE  RTL_SIZEOF_THROUGH_FIELD(DISC_INFORMATION,   DiscType)

#define CoTaskMemFreeAndNull(x) \
{                               \
    CoTaskMemFree(x);           \
    (x) = NULL;                 \
}

#define LocalFreeAndNull(x)     \
{                               \
    if ((x) != NULL)            \
    {                           \
        LocalFree(x);           \
        (x) = NULL;             \
    }                           \
}

__inline void SysFreeStringAndNull(__in_opt BSTR &t) throw()
{
    ::SysFreeString(t);
    t = NULL;
    return;
}

#define ReleaseAndNull(x)       \
{                               \
    if ((x) != NULL)            \
    {                           \
        (x)->Release();         \
        (x) = NULL;             \
    }                           \
}

typedef enum _START_STOP_OPTION 
{
    StopSpinning,
    StartSpinning,
    EjectMedia,
    LoadMedia
} START_STOP_OPTION, *PSTART_STOP_OPTION;

typedef struct _SENSE_INFOMATION 
{
    UCHAR Sense;
    UCHAR Asc;
    UCHAR Ascq;
    UCHAR Reserved;
} SENSE_INFOMATION, *PSENSE_INFOMATION;

typedef VOID (*READ_DISC_INFO_CALLBACK)(__in PVOID object, __in SENSE_DATA* sense);

//
// IMPLEMENTATION NOTE: Some tasks take a fixed amount of time
//     before they show any progress, then zoom along.  To better
//     support these tasks, the estimated time is not updated until
//     at least _two_ non-zero put_CompletedSteps() are called.
//
// NOTE: This class is only valid for tasks less than about
//       49 days in length due to tick count overflow.
//
class CTaskTimeEstimator {
private:
    // BIG 3 -- Compiler auto-generates three functions if not provided,
    //          and marks them as public.  Avoid this, since this class
    //          doesn't make sense to "copy".  So, create private default
    //          constructor, copy constructor, and assignment operator.
    //          Also, avoid declaring the body of the assignment operator
    //          to ensure it's not used even internally.
    CTaskTimeEstimator() throw();
    CTaskTimeEstimator(const CTaskTimeEstimator& x) throw();
    CTaskTimeEstimator& operator=(const CTaskTimeEstimator& x) throw();
public:
    // BIG 3 -- constructor, destructor, copy operator
    CTaskTimeEstimator(const ULONG estimatedTotalNumberOfMilliseconds, const ULONG numberOfEqualSteps = 1) throw();
    ~CTaskTimeEstimator() throw();

    // Start and stop keeping track of internal time -- only use once
    VOID StartNow();
    VOID EndNow(); // equivalent to calling put_CompletedSteps(get_TotalSteps());

    // Can use these any time
    ULONG get_OriginalTotalMilliseconds() const throw();  // passed in via constructor
    ULONG get_TotalSteps()                const throw();  // passed in via constructor

    // Only use these after StartNow() has been called
    ULONG get_TotalMilliseconds()         const throw();  // (auto) estimate based on perf
    ULONG get_RemainingMilliseconds()     const throw();  // (auto) estimate based on perf
    ULONG get_ElapsedMilliseconds()       const throw();  // (auto) automatically calculated
    ULONG get_CompletedSteps()            const throw();  // number of steps completed

    // Only use these during running timer
    VOID  put_CompletedSteps(const ULONG completed); // number of steps completed

private:
    VOID  TestInvariants() const throw();
    VOID  UpdateTime() throw();

    const ULONG   m_OriginalTotalMilliseconds; 
          ULONG   m_ExpectedTotalMilliseconds; // auto-updates
          ULONG   m_ElapsedMilliseconds;       // auto-updates
          ULONG   m_StartTimeTickCount;
          BOOLEAN m_TimerStarted;
          BOOLEAN m_TimerEnded;

    const ULONG   m_TotalSteps;
          ULONG   m_PreviousCompletedSteps; // to find drives reporting in large "steps"
          ULONG   m_CompletedSteps;

          ULONG   m_NonZeroCompletedStepsCount;
          ULONG   m_FirstNonZeroElapsedMilliseconds;
          ULONG   m_FirstNonZeroCompletedSteps;


#define TimeKeeperHistoryDepth 4
          ULONG   m_NewEstimatesHistory[TimeKeeperHistoryDepth];

};

// CDiscInformation definition
class CDiscInformation
{
public:
    CDiscInformation() throw();
    ~CDiscInformation() throw();

private: 
    CDiscInformation(const CDiscInformation& x) throw(); // prevent copy constructor
    CDiscInformation& operator=(const CDiscInformation& x) throw(); // prevent assignment operator

public:
    HRESULT   Init(__in IDiscRecorder2Ex* recorder, __in BOOLEAN Reuse = FALSE ) throw();

    ULONG           get_DiscStatus() throw();
    BOOLEAN         get_Erasable() throw();

private:
    HRESULT           Init(__in_bcount(BufferSize) BYTE* Buffer, __in LONG BufferSize, __in BOOLEAN Reuse = FALSE) throw();
    static HRESULT    ValidateInitData(__in_bcount(BufferSize) BYTE* Buffer, __in LONG BufferSize) throw();

    DISC_INFORMATION* m_DiscInfo;
    ULONG             m_DiscInfoSize;
    ULONG             m_DiscInfoAvailableSize;
};


HRESULT PreventAllowMediumRemoval(__in IDiscRecorder2Ex* recorder, const BOOLEAN lockMedia, const BOOLEAN persistentBit = 0);
HRESULT SendStartStopUnitCommand(__in IDiscRecorder2Ex* recorder, const START_STOP_OPTION option);
HRESULT GetCurrentPhysicalMediaType(__in IDiscRecorder2Ex* recorder, __out IMAPI_MEDIA_PHYSICAL_TYPE * value);
HRESULT SendSetCDSpeed(__in IDiscRecorder2* discRecorder,
                       const IMAPI_MEDIA_PHYSICAL_TYPE mediaType,
                       const ULONG KBps,
                       const ULONG rotationType
                       );

BOOLEAN BstrIsValidClientName(__in_xcount(SysStringLen(value)) BSTR value);

HRESULT UpdateCurrentDriveProperties(__in IDiscRecorder2* discRecorder,
                                     const IMAPI_MEDIA_PHYSICAL_TYPE mediaType, 
                                     __out ULONG *currentSpeedSectorsPerSecond,
                                     __out ULONG *currentSpeedKBps, 
                                     __out VARIANT_BOOL *currentRotationTypeIsPureCAV
                                     );

HRESULT WaitForReadDiscInfo(__in IDiscRecorder2Ex* recorder, const ULONG secondsToTry, __in_opt PVOID object = NULL, __in_opt __callback READ_DISC_INFO_CALLBACK callback = NULL);
HRESULT ReadMediaCapacity(__in IDiscRecorder2Ex* recorder, __out ULONG* bytesPerBlock, __out ULONG* useSectors);
HRESULT RequestAutomaticOPC(__in IDiscRecorder2Ex* recorder);
HRESULT SendSynchronizeCacheCommand(__in IDiscRecorder2Ex* recorder, ULONG timeout = SYNCHRONIZE_CACHE_TIMEOUT, BOOLEAN immediate = TRUE); // default timeout of 5 minutes should be safe
// Uses the command set to determine if the media is blank with 100% certainty
HRESULT GetMediaPhysicallyBlank(__in IDiscRecorder2Ex* discRecorder,
                                __out VARIANT_BOOL* pPhysicallyBlank);

// Uses heuristics to determine if the media is blank
// Mainly used for DVD+RW and DVD-RAM media types
HRESULT GetMediaHeuristicallyBlank(__in IDiscRecorder2Ex* discRecorder,
                                   __out VARIANT_BOOL* pHeuristicallyBlank);
BOOLEAN IsSenseDataInTable(__in_ecount(entries) PSENSE_INFOMATION table, const LONG entries, __in PSENSE_DATA senseData);
const BOOLEAN TranslateSenseInfoToHResult(
                                        __in_bcount(1)                  const CDB * Cdb,
                                        __in_bcount(sizeof(SENSE_DATA)) const SENSE_DATA * Sense,
                                        __out                           HRESULT * HResult
                                        );
HRESULT FuzzyConvert_KBps2SectorsPerSecond(const IMAPI_MEDIA_PHYSICAL_TYPE mediaType,
                                           const ULONG writeSpeedKBps,
                                           __out ULONG *writeSpeedSectorsPerSecond
                                           );

const HRESULT CreateVariantSafeArrayFromEnums(
    __in_ecount(valueCount)   const LONG * values,
    __in                      const ULONG valueCount,
    __deref_out                     SAFEARRAY** result
    );

__inline DWORD CountOfSetBits(__in ULONGLONG value) throw()
{
    ULONG i = 0;
    while (value != 0)
    {
        value &= value - 1; // this drops the lowest set bit
        i++;
    }
    return i;
}

__inline void SafeArrayDestroyAndNull(__in_opt LPSAFEARRAY &t ) throw()
{
    if (t != NULL)
    {
        SafeArrayDestroy(t);
        t = NULL;
    }
}

// Inline helper functions
__inline bool IS_CD_MEDIA(IMAPI_MEDIA_PHYSICAL_TYPE mediaType) {
    return (((mediaType == IMAPI_MEDIA_TYPE_CDROM) ||
             (mediaType == IMAPI_MEDIA_TYPE_CDR) ||
             (mediaType == IMAPI_MEDIA_TYPE_CDRW)) ? true : false);
}

__inline bool IS_DVD_MEDIA(IMAPI_MEDIA_PHYSICAL_TYPE mediaType) {
    return (((mediaType == IMAPI_MEDIA_TYPE_DVDROM) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDDASHR) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDDASHRW) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDPLUSR) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDPLUSRW) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER) ||
             (mediaType == IMAPI_MEDIA_TYPE_DVDRAM)) ? true : false);
}
