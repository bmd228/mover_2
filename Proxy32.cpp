#include <windows.h>
#include <iostream>
#include <comdef.h>
#include "Your32BitLibrary.h" // Включите заголовок вашей 32-разрядной библиотеки

// {00000000-0000-0000-0000-000000000000} замените на ваш уникальный CLSID
const CLSID CLSID_MyComObject = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

// {00000000-0000-0000-0000-000000000001} замените на ваш уникальный IID
const IID IID_IMyComInterface = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};

class IMyComInterface : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE MyFunction(BSTR input, BSTR* output) = 0;
};

class MyComObject : public IMyComInterface
{
public:
    MyComObject() : refCount(1) {}

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        if (riid == IID_IUnknown || riid == IID_IMyComInterface)
        {
            *ppvObject = static_cast<IMyComInterface*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG count = InterlockedDecrement(&refCount);
        if (count == 0)
        {
            delete this;
            return 0;
        }
        return count;
    }

    // IMyComInterface
    HRESULT STDMETHODCALLTYPE MyFunction(BSTR input, BSTR* output)
    {
        // Вызов функции из 32-разрядной библиотеки
        std::wstring result = ProcessData(_bstr_t(input));
        *output = SysAllocString(result.c_str());
        return S_OK;
    }

private:
    LONG refCount;
};

class MyClassFactory : public IClassFactory
{
public:
    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        if (riid == IID_IUnknown || riid == IID_IClassFactory)
        {
            *ppvObject = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG count = InterlockedDecrement(&refCount);
        if (count == 0)
        {
            delete this;
            return 0;
        }
        return count;
    }

    // IClassFactory
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject)
    {
        if (pUnkOuter != nullptr)
        {
            return CLASS_E_NOAGGREGATION;
        }

        MyComObject* pObject = new MyComObject();
        HRESULT hr = pObject->QueryInterface(riid, ppvObject);
        pObject->Release();
        return hr;
    }

    HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock)
    {
        if (fLock)
        {
            InterlockedIncrement(&serverLocks);
        }
        else
        {
            InterlockedDecrement(&serverLocks);
        }
        return S_OK;
    }

private:
    LONG refCount;
    static LONG serverLocks;
};

LONG MyClassFactory::serverLocks = 0;

HRESULT RegisterServer()
{
    // Регистрация CLSID и других необходимых регистрационных записей
    // ...
    return S_OK;
}

HRESULT UnregisterServer()
{
    // Удаление CLSID и других регистрационных записей
    // ...
    return S_OK;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

extern "C" HRESULT __stdcall DllRegisterServer()
{
    return RegisterServer();
}

extern "C" HRESULT __stdcall DllUnregisterServer()
{
    return UnregisterServer();
}

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (rclsid == CLSID_MyComObject)
    {
        MyClassFactory* pFactory = new MyClassFactory();
        HRESULT hr = pFactory->QueryInterface(riid, ppv);
        pFactory->Release();
        return hr;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    return (MyClassFactory::serverLocks == 0) ? S_OK : S_FALSE;
}
