//
// Defines the queue label and path
//
#define STR_QUEUE_NAME	L".\\IStreamTest"
#define STR_QUEUE_LABEL L"Object Test Queue"

// COM exception handler
void dump_com_error(_com_error &e)
{
    _tprintf(_T("Oops - hit an error!\n"));
    _tprintf(_T("\a\tCode = %08lx\n"), e.Error());
    _tprintf(_T("\a\tCode meaning = %s\n"), e.ErrorMessage());
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
    _tprintf(_T("\a\tSource = %s\n"), (LPCTSTR) bstrSource);
    _tprintf(_T("\a\tDescription = %s\n"), (LPCTSTR) bstrDescription);
}


struct InitOle {
    InitOle() { CoInitialize(NULL); }   // Initialize Component Object Model(COM) library
    ~InitOle() { CoUninitialize(); }	// Uninitialize Component Object Model(COM) library
};

