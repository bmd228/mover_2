#include <windows.h>
#include <iostream>
#include <comdef.h>

// {00000000-0000-0000-0000-000000000001} замените на ваш уникальный IID
const IID IID_IMyComInterface = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};

class IMyComInterface : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE MyFunction(BSTR input, BSTR* output) = 0;
};

int main()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        std::cerr << "CoInitialize failed." << std::endl;
        return 1;
    }

    IMyComInterface* pMyComInterface = nullptr;
    CLSID clsid;
    hr = CLSIDFromString(OLESTR("{00000000-0000-0000-0000-000000000000}"), &clsid); // Замените на ваш CLSID
    if (FAILED(hr))
    {
        std::cerr << "CLSIDFromString failed." << std::endl;
        CoUninitialize();
        return 1;
    }

    hr = CoCreateInstance(clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_IMyComInterface, (void**)&pMyComInterface);
    if (FAILED(hr))
    {
        std::cerr << "CoCreateInstance failed." << std::endl;
        CoUninitialize();
        return 1;
    }

    BSTR input = SysAllocString(L"Hello from 64-bit application!");
    BSTR output;
    hr = pMyComInterface->MyFunction(input, &output);
    if (SUCCEEDED(hr))
    {
        std::wcout << L"Received from 32-bit COM server: " << output << std::endl;
    }
    else
    {
        std::cerr << "MyFunction failed." << std::endl;
    }

    SysFreeString(input);
    SysFreeString(output);
    pMyComInterface->Release();
    CoUninitialize();
    return 0;
}
