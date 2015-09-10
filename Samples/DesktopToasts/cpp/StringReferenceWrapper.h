class StringReferenceWrapper
{  
public:  
  
    // Constructor which takes an existing string buffer and its length as the parameters.
    // It fills an HSTRING_HEADER struct with the parameter.      
    // Warning: The caller must ensure the lifetime of the buffer outlives this      
    // object as it does not make a copy of the wide string memory.      

    StringReferenceWrapper(_In_reads_(length) PCWSTR stringRef, _In_ UINT32 length) throw()  
    {  
        HRESULT hr = WindowsCreateStringReference(stringRef, length, &_header, &_hstring);  
          
        if (FAILED(hr))  
        {  
            RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);  
        }  
    }

	~StringReferenceWrapper()
	{
		WindowsDeleteString(_hstring);
	}

    template <size_t N>  
    StringReferenceWrapper(_In_reads_(N) wchar_t const (&stringRef)[N]) throw()  
    {  
        UINT32 length = N-1; 
        HRESULT hr = WindowsCreateStringReference(stringRef, length, &_header, &_hstring);
          
        if (FAILED(hr))
        {  
            RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);  
        }  
    }

    template <size_t _>  
    StringReferenceWrapper(_In_reads_(_) wchar_t (&stringRef)[_]) throw()  
    {  
        UINT32 length;  
        HRESULT hr = SizeTToUInt32(wcslen(stringRef), &length);  
          
        if (FAILED(hr))
        {  
            RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);  
        }  
          
        WindowsCreateStringReference(stringRef, length, &_header, &_hstring);
    }
 
    HSTRING Get() const throw()  
    {  
        return _hstring;  
    }  
   
  
private:               
    HSTRING             _hstring;  
    HSTRING_HEADER      _header;  
};  
