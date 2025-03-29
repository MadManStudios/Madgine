
#include <assert.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <array>

#include <comdef.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include "../src/DirectX12/d3dx12.h"

static void log(std::string_view s)
{
    static std::ofstream fOut { "dx12drivertest.log" };

    std::cout << s << std::endl;
    fOut << s << std::endl;
}

static void __stdcall dxDebugOutput(D3D12_MESSAGE_CATEGORY category,
    D3D12_MESSAGE_SEVERITY severity,
    D3D12_MESSAGE_ID id,
    LPCSTR message,
    void *context)
{
    std::stringstream ss;

    switch (severity) {
    case D3D12_MESSAGE_SEVERITY_CORRUPTION:
        ss << "Fatal: ";
        break;
    case D3D12_MESSAGE_SEVERITY_ERROR:
        ss << "Error: ";
        break;
    case D3D12_MESSAGE_SEVERITY_WARNING:
        ss << "Warning: ";
        break;
    case D3D12_MESSAGE_SEVERITY_INFO:
        ss << "Info: ";
        break;
    case D3D12_MESSAGE_SEVERITY_MESSAGE:
        ss << "Message: ";
        break;
    }

    ss << "(" << id << "): " << message;
    log(ss.str());
}

inline void dx12Check(const char *file, size_t line, HRESULT result = 0)
{
    if (FAILED(result)) {
        _com_error error { result };
        std::stringstream ss;
        ss << "DX12-Error (" << result << "): " << error.ErrorMessage() << "\n";
        ss << " at " << file << "(" << line << ")";
        log(ss.str());
        std::terminate();
    }
}

void check(HRESULT hr, std::stringstream &ss)
{
    if (FAILED(hr)) {
        ss << "Failed (" << std::hex << hr << "): \n";
        _com_error error { hr };
        ss << error.ErrorMessage();
    } else {
        ss << "Success!";
    }
}

#define DX12_CHECK(...) dx12Check(__FILE__, __LINE__, __VA_ARGS__)

struct ReleaseDeleter {
    template <typename T>
    void operator()(T *ptr)
    {
        ptr->Release();
    }
};

template <typename T>
struct ReleasePtr : std::unique_ptr<T, ReleaseDeleter> {
    using std::unique_ptr<T, ReleaseDeleter>::unique_ptr;
    ReleasePtr(const ReleasePtr &other)
        : std::unique_ptr<T, ReleaseDeleter> { other.get() }
    {
        this->get()->AddRef();
    }
    ReleasePtr(ReleasePtr &&) = default;
    template <typename U>
    ReleasePtr(ReleasePtr<U> &&other)
        : std::unique_ptr<T, ReleaseDeleter> { std::move(other) }
    {
    }

    ReleasePtr &operator=(ReleasePtr &&) = default;

    T **operator&()
    {
        assert(!*this);
        return reinterpret_cast<T **>(this);
    }

    T *const *operator&() const
    {
        return reinterpret_cast<T *const *>(this);
    }

    operator T *() const
    {
        return this->get();
    }
};

static ReleasePtr<IDXGIAdapter1> GetHardwareAdapter(IDXGIFactory4 *pFactory)
{
    for (UINT adapterIndex = 0;; ++adapterIndex) {
        ReleasePtr<IDXGIAdapter1> pAdapter;
        if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter)) {
            // No more adapters to enumerate.
            break;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
            return pAdapter;
        }
    }
    return {};
}

void logProperties(ID3D12Device *device)
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS options;

    HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
    DX12_CHECK(hr);

    log("Device Properties:");
    log("    ResourceHeapTier: " + std::to_string(options.ResourceHeapTier));
    log("");
}

void testCreateHeap(ID3D12Device *device)
{
    log("Testing CreateHeap");

    std::stringstream ss;

    for (size_t s = 8388608; s <= 8388608; s *= 2) {

        ss << "Size: " << s << "...\n";

        for (auto [flags, name] : std::array<std::pair<D3D12_HEAP_FLAGS, const char *>, 2> { { { D3D12_HEAP_FLAG_NONE, "None" },
                 { D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES, "Deny Textures" } } }) {
            ss << "  Flags: " << name << "... ";

            CD3DX12_HEAP_DESC heapDesc = CD3DX12_HEAP_DESC { s, D3D12_HEAP_TYPE_DEFAULT, 0, flags };

            ReleasePtr<ID3D12Heap> heap;

            HRESULT hr = device->CreateHeap(
                &heapDesc,
                IID_PPV_ARGS(&heap));
            check(hr, ss);
            ss << "\n";
        }
    }

    log(ss.str());
}

void testCreateCommitedResource(ID3D12Device *device)
{
    log("Testing CreateCommitedResource");

    std::stringstream ss;
    for (size_t s = 8388608; s <= 8388608; s *= 2) {

        ss << "Size: " << s << "...\n";

        auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(s);

        for (auto [type, name] : std::array<std::pair<D3D12_HEAP_TYPE, const char *>, 3> { { { D3D12_HEAP_TYPE_DEFAULT, "Default" },
                 { D3D12_HEAP_TYPE_UPLOAD, "Upload" },
                 { D3D12_HEAP_TYPE_READBACK, "Readback" } } }) {
            ss << "  Type: " << name << "... ";
            auto heapDesc = CD3DX12_HEAP_PROPERTIES(type);

            ReleasePtr<ID3D12Resource> res;

            HRESULT hr = device->CreateCommittedResource(
                &heapDesc,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                type == D3D12_HEAP_TYPE_READBACK ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&res));
            check(hr, ss);
            ss << "\n";
        }
    }

    log(ss.str());
}

int main()
{
    ReleasePtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        ReleasePtr<ID3D12Debug1> debugController1;
        if (SUCCEEDED(debugController->QueryInterface(IID_PPV_ARGS(&debugController1)))) {
            //debugController1->SetEnableGPUBasedValidation(true);
            log("Enabled Debug Layer");
        }
    }

    ReleasePtr<IDXGIFactory4> factory;

    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    DX12_CHECK(hr);

    ReleasePtr<IDXGIAdapter1> hardwareAdapter = GetHardwareAdapter(factory);

    ReleasePtr<ID3D12Device> device;

    log("Creating Device...");
    hr = D3D12CreateDevice(hardwareAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
    DX12_CHECK(hr);
    log("Success\n");

    {
        ReleasePtr<ID3D12InfoQueue1> infoQueue;
        hr = device->QueryInterface(IID_PPV_ARGS(&infoQueue));

        DWORD cookie;
        if (SUCCEEDED(hr)) {
            hr = infoQueue->RegisterMessageCallback(
                &dxDebugOutput,
                D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &cookie);
            DX12_CHECK(hr);
        }
    }

    logProperties(device);

    testCreateHeap(device);
    testCreateCommitedResource(device);
}