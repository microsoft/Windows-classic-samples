/*
    Copyright (c) 2009 Microsoft Corporation

    Module Name:
        swriter.h
*/


#pragma once


#define COMPONENT_NAME_LENGTH 16


class SampleWriter : public CVssWriterEx2
{
public:
    SampleWriter();
    virtual ~SampleWriter();

    STDMETHODIMP Initialize();
    STDMETHODIMP Uninitialize();

    STDMETHODIMP_(bool) OnIdentify(IVssCreateWriterMetadata *pMetadata);
    STDMETHODIMP_(bool) OnPrepareBackup(IVssWriterComponents *pComponents);
    STDMETHODIMP_(bool) OnPrepareSnapshot();
    STDMETHODIMP_(bool) OnFreeze();
    STDMETHODIMP_(bool) OnThaw();
    STDMETHODIMP_(bool) OnPostSnapshot(IVssWriterComponents *pComponents);
    STDMETHODIMP_(bool) OnAbort();
    STDMETHODIMP_(bool) OnBackupComplete(IVssWriterComponents *pComponents);
    STDMETHODIMP_(bool) OnBackupShutdown(VSS_ID SnapshotSetId);

    STDMETHODIMP_(bool) OnPreRestore(IVssWriterComponents *pComponents);
    STDMETHODIMP_(bool) OnPostRestore(IVssWriterComponents *pComponents);

private:
    STDMETHODIMP_(bool) CreateAccountName(PWSTR *pwszAccountName, PSID pSid);
    STDMETHODIMP_(bool) AddComponent(IVssCreateWriterMetadata *pMetadata, PCWSTR wszProfile, PCWSTR wszPath);
    STDMETHODIMP_(bool) AddComponentForUserProfile(IVssCreateWriterMetadata *pMetadata, PCWSTR wszSid);
    STDMETHODIMP_(bool) AddComponents(IVssCreateWriterMetadata *pMetadata);
};


class CQueue
{
public:
    CQueue(PCWSTR wszPath, PCWSTR wszComponentPath, PCWSTR wszComponentName)
    {
        DWORD   dwLength    = 0;
        m_wszPath = NULL;
        ZeroMemory(m_wszComponentPath, COMPONENT_NAME_LENGTH * 3 * sizeof(WCHAR));
        ZeroMemory(m_wszComponentName, COMPONENT_NAME_LENGTH * sizeof(WCHAR));

        if (wszPath != NULL)
        {
            // For the purpose of this sample make it best effort
            dwLength = (DWORD)wcslen(wszPath) + 1;
            m_wszPath = (PWSTR)malloc(dwLength * sizeof(WCHAR));
            if (m_wszPath == NULL)
                return;
            ZeroMemory(m_wszPath, dwLength * sizeof(WCHAR));
            memcpy(m_wszPath, wszPath, (dwLength - 1) * sizeof(WCHAR));

            if (wszComponentPath != NULL)
            {
                dwLength = (DWORD)wcslen(wszComponentPath) + 1;
                if (dwLength > COMPONENT_NAME_LENGTH * 3)
                    dwLength = COMPONENT_NAME_LENGTH * 3;
                memcpy(m_wszComponentPath, wszComponentPath, (dwLength - 1) * sizeof(WCHAR));
            }

            if (wszComponentName != NULL)
            {
                dwLength = (DWORD)wcslen(wszComponentName) + 1;
                if (dwLength > COMPONENT_NAME_LENGTH)
                    dwLength = COMPONENT_NAME_LENGTH;
                memcpy(m_wszComponentName, wszComponentName, (dwLength - 1) * sizeof(WCHAR));
            }
        }
    }

    // Destructor does not delete its child
    virtual ~CQueue() {};

    CQueue* Enqueue(CQueue *child)
    {
        m_next = child;
        return this;
    }

    CQueue* GetNext()
    {
        return m_next;
    }

    PCWSTR GetComponentPath()
    {
        return m_wszComponentPath;
    }

    PCWSTR GetComponentName()
    {
        return m_wszComponentName;
    }

    PCWSTR GetPath()
    {
        return m_wszPath;
    }

private:
    PWSTR   m_wszPath;
    WCHAR   m_wszComponentPath[COMPONENT_NAME_LENGTH * 3];
    WCHAR   m_wszComponentName[COMPONENT_NAME_LENGTH];
    CQueue  *m_next;
};


typedef struct _SAMPLE_COMPONENT_TYPE
{
    WCHAR   wszComponent[COMPONENT_NAME_LENGTH];
    WCHAR   wszComponentCaption[32];
    WCHAR   wszFileGroupMask[8];
} SAMPLE_COMPONENT_TYPE, *PSAMPLE_COMPONENT_TYPE;


