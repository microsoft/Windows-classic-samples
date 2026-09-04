#include "DirectCompositionTextureAndDynamicTexture.h"

// Vertex structure for d3d.
struct SimpleVertex
{
    D2D_POINT_2F Pos;
    D2D_COLOR_F Color;
};

// Vertex structure for d3d.
struct TextureSizeBuffer
{
    D2D_SIZE_F screenSize;
    D2D_SIZE_F padding;
};

CApp::CApp() {}

CApp::~CApp()
{
    Stop();
}

void
CApp::Start(HWND hwnd)
{
    m_hMainWindow = hwnd;
    m_running = true;
    m_hRenderThread = CreateThread(
        nullptr,                   // Default security attributes
        0,                         // Default stack size
        RenderThreadProc,          // Thread start address
        this,                      // Parameter to pass to the thread
        0,                         // Creation flags
        &m_dwRenderThreadId        // Receive thread identifier
    );

    if (m_hRenderThread == nullptr) {
        m_running = false;
    }
}

void
CApp::Stop()
{
    if (m_running)
    {
        m_running = false;
        WaitForSingleObject(m_hRenderThread, INFINITE);

        CloseHandle(m_hRenderThread);
    }
}

DWORD WINAPI CApp::RenderThreadProc(LPVOID lpParameter)
{
    CApp* pThis = static_cast<CApp*>(lpParameter);
    if (pThis)
    {
        pThis->Run();
    }
    return 0;
}

// Make D3D resources to utilize with the presentation API
void
CApp::MakeD3DDevice()
{
    unsigned flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_SINGLETHREADED;

    // Create the D3D device.
    wil::com_ptr_nothrow<ID3D11Device> d3dDevice;
    wil::com_ptr_nothrow<ID3D11DeviceContext> d3dDeviceContext;
    EXIT_IF_FAILED(D3D11CreateDevice(
        nullptr,                                // No adapter.
        D3D_DRIVER_TYPE_HARDWARE,               // Hardware device.
        nullptr,                                // No module.
        flags,                                  // Defined flags.
        nullptr, 0,                             // Highest available feature level.
        D3D11_SDK_VERSION,                      // API version.
        &d3dDevice,                            // Resulting interface pointer.
        nullptr,                                // Actual feature level.
        &d3dDeviceContext));                   // Device context.

    EXIT_IF_FAILED(d3dDevice.query_to(&m_d3d11Device));
    EXIT_IF_FAILED(d3dDeviceContext.query_to(&m_d3d11Context));

    //=== Shader source code (inline strings) ===

    const char* circleVSHLSL = R"(
        cbuffer TextureSizeBuffer : register(b0)
        {
            float2 screenSize;  // (texture_width, texture_height)
            float2 padding;     // padding to ensure 16-byte alignment
        };

        struct VS_INPUT
        {
            float3 pos : POSITION;
            float4 col : COLOR;
        };

        struct PS_INPUT
        {
            float4 pos : SV_POSITION;
            float4 col : COLOR;
        };

        PS_INPUT VSMain(VS_INPUT input)
        {
            PS_INPUT output;
            // Convert pixel coordinates to normalized device coordinates
            float x_ndc = (input.pos.x / screenSize.x) * 2.0f - 1.0f;
            float y_ndc = 1.0f - (input.pos.y / screenSize.y) * 2.0f;
            
            output.pos = float4(x_ndc, y_ndc, input.pos.z, 1.0f);
            //output.pos = float4(input.pos, 1.0f);
            output.col = input.col;
            return output;
        }
    )";

    const char* circlePSHLSL = R"(
        struct PS_INPUT
        {
            float4 pos : SV_POSITION;
            float4 col : COLOR;
        };

        float4 PSMain(PS_INPUT input) : SV_Target
        {
            return input.col;
        }
    )";

    // Compile Vertex Shader.
    wil::com_ptr_nothrow<ID3DBlob> pVSBlob = nullptr;
    EXIT_IF_FAILED(D3DCompile(circleVSHLSL, strlen(circleVSHLSL), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &pVSBlob, nullptr));
    EXIT_IF_FAILED(m_d3d11Device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pCircleVS));

    // Define the input layout for our vertex shader.
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(SimpleVertex, Pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, offsetof(SimpleVertex, Color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layoutDesc);
    EXIT_IF_FAILED(m_d3d11Device->CreateInputLayout(layoutDesc, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pCircleInputLayout));

    // Compile Pixel Shader.
    ID3DBlob* pPSBlob = nullptr;
    EXIT_IF_FAILED(D3DCompile(circlePSHLSL, strlen(circlePSHLSL), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pPSBlob, nullptr));
    EXIT_IF_FAILED(m_d3d11Device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pCirclePS));

    // Create screen size buffer.
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(TextureSizeBuffer);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    EXIT_IF_FAILED(m_d3d11Device->CreateBuffer(&cbDesc, nullptr, &m_pTextureSizeBuffer));
}

// Creates a pool of Texture objects. FindAvailableTexture uses this pool to find a texture
// for rendering the next frame. For simplicity not shown, see original Composition Texture
// examples for more details.
void
CApp::MakeTextures()
{
    // Check for composition texture support.
    BOOL supported = FALSE;
    EXIT_IF_FAILED(m_dcompDevice->CheckCompositionTextureSupport(m_d3d11Device.get(), &supported));
    if (!supported)
    {
        EXIT_IF_FAILED(E_FAIL);
    }

    auto make_texture = [&](UINT w, UINT h)
        {
            wil::unique_handle bufferHandle;

            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = w;
            textureDesc.Height = h;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags =
                D3D11_RESOURCE_MISC_SHARED |
                D3D11_RESOURCE_MISC_SHARED_NTHANDLE;

            wil::com_ptr_nothrow<ID3D11Texture2D> d3dTexture;
            EXIT_IF_FAILED(m_d3d11Device->CreateTexture2D(&textureDesc, nullptr, &d3dTexture));
            wil::com_ptr_nothrow<IDCompositionTexture> compositionTexture;
            EXIT_IF_FAILED(m_dcompDevice->CreateCompositionTexture(d3dTexture.get(), &compositionTexture));

            Texture texture;
            EXIT_IF_FAILED(compositionTexture.query_to(&texture.compositionTexture));
            texture.d3dTexture = d3dTexture;
            return texture;
        };

    for (int i = 0; i < 4; i++)
    {
        m_textures.push_back(make_texture(TEXTURE_WIDTH, TEXTURE_HEIGHT));
    }
}

void
CApp::Run()
{
    EXIT_IF_FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

    MakeD3DDevice();
    MakeDComp();
    MakeTextures();

    InitializeScene();

    while (m_running)
    {
        auto texture = FindAvailableTexture();

        RenderScene(texture);
        UpdateScene();

        DCompSetTexture(texture);
        DCompCommit();
    }

    CoUninitialize();
}

void
CApp::MakeDComp()
{
    // Make dcomp and hook the content into the tree
    EXIT_IF_FAILED(DCompositionCreateDevice3(nullptr, IID_PPV_ARGS(&m_dcompDevice)));
    auto dcompDesktopDevice = m_dcompDevice.try_query<IDCompositionDesktopDevice>();
    EXIT_IF_FAILED(dcompDesktopDevice->CreateTargetForHwnd(m_hMainWindow, TRUE, &m_dcompTarget));
    EXIT_IF_FAILED(m_dcompDevice->CreateVisual(&m_rootVisual));
    EXIT_IF_FAILED(m_dcompTarget->SetRoot(m_rootVisual.get()));

    auto dcompDevice5 = m_dcompDevice.try_query<PREVIEW_IDCompositionDevice5>();
    if (dcompDevice5 != nullptr)
    {
        EXIT_IF_FAILED(dcompDevice5->CreateDynamicTexture(&m_rootDynamicTexture));
        EXIT_IF_FAILED(m_rootVisual->SetContent(m_rootDynamicTexture.get()));
    }

    EXIT_IF_FAILED(m_dcompDevice->Commit());
}

void
CApp::DCompSetTexture(const Texture& texture)
{
    // Set current texture. If dynamic texture is available we use SetTexture with dirty rects.
    // Otherwise we fall back to calling SetContent on the visual directly.
    if (m_rootDynamicTexture != nullptr)
    {
        // Dirty rect collection. For this example everything that was rendered on previous frame and
        // on current frame is considered dirt.
        std::vector<RECT> dirtyRects;
        dirtyRects.insert(dirtyRects.begin(), renderedRectsPrevFrame.begin(), renderedRectsPrevFrame.end());
        dirtyRects.insert(dirtyRects.begin(), renderedRectsCurrFrame.begin(), renderedRectsCurrFrame.end());
        EXIT_IF_FAILED(m_rootDynamicTexture->SetTexture(
            texture.compositionTexture.get(),
            dirtyRects.data(),
            dirtyRects.size()));
    }
    else
    {
        EXIT_IF_FAILED(m_rootVisual->SetContent(texture.compositionTexture.get()));
    }
    renderedRectsPrevFrame = renderedRectsCurrFrame;
    renderedRectsCurrFrame.clear();
}

void
CApp::DCompCommit()
{
    EXIT_IF_FAILED(m_dcompDevice->Commit());
}

void
CApp::RenderCircle(ID3D11Texture2D* pTexture, const D2D1_COLOR_F& color, const D2D_POINT_2F& position, float radius)
{
    // Create a render target view from the texture.
    wil::com_ptr_nothrow<ID3D11RenderTargetView> pRTV;
    wil::com_ptr_nothrow<ID3D11Resource> pResource;
    EXIT_IF_FAILED(pTexture->QueryInterface(IID_PPV_ARGS(&pResource)));
    EXIT_IF_FAILED(m_d3d11Device->CreateRenderTargetView(pResource.get(), nullptr, &pRTV));

    // Set render target and matching viewport.
    ID3D11RenderTargetView* rtvs[] = { pRTV.get() };
    m_d3d11Context->OMSetRenderTargets(1, rtvs, nullptr);
    D3D11_TEXTURE2D_DESC desc = {};
    pTexture->GetDesc(&desc);
    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<FLOAT>(desc.Width);
    vp.Height = static_cast<FLOAT>(desc.Height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_d3d11Context->RSSetViewports(1, &vp);

    // Set the shaders that know how to draw a circle.
    m_d3d11Context->VSSetShader(m_pCircleVS.get(), nullptr, 0);
    m_d3d11Context->PSSetShader(m_pCirclePS.get(), nullptr, 0);

    // Bind the vertex buffer, index buffer and input layout for a full-screen quad.
    // (Assume m_pQuadVB, m_pQuadIB and m_pCircleInputLayout are created during initialization.)
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    wil::com_ptr_nothrow<ID3D11Buffer> m_pQuadVB;
    wil::com_ptr_nothrow<ID3D11Buffer> m_pQuadIB;

    // Define the circle as a polygon with a center and 16 segments.
    const int numSegments = 24;
    const int numVertices = numSegments + 1; // center + 16 circumference vertices
    std::vector<SimpleVertex> vertices;
    vertices.reserve(numVertices);

    vertices.push_back({ position, color });
    for (int i = 0; i < numSegments; ++i)
    {
        float angle = 6.2832f * static_cast<float>(i) / numSegments;
        float x = position.x + radius * cosf(angle);
        float y = position.y + radius * sinf(angle);
        vertices.push_back({ {x, y}, color });
    }

    std::vector<UINT> indices;
    indices.reserve(numSegments * 3);
    for (int i = 0; i < numSegments; ++i)
    {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back((i + 1) % numSegments + 1);
    }

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = static_cast<UINT>(sizeof(SimpleVertex) * vertices.size());
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA vbInitData = {};
    vbInitData.pSysMem = vertices.data();
    EXIT_IF_FAILED(m_d3d11Device->CreateBuffer(&vbDesc, &vbInitData, &m_pQuadVB));


    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA ibInitData = {};
    ibInitData.pSysMem = indices.data();
    EXIT_IF_FAILED(m_d3d11Device->CreateBuffer(&ibDesc, &ibInitData, &m_pQuadIB));

    ID3D11Buffer* buffers[] = { m_pQuadVB.get() };
    ID3D11Buffer* constantBuffers[] = { m_pTextureSizeBuffer.get() };

    m_d3d11Context->IASetInputLayout(m_pCircleInputLayout.get());
    m_d3d11Context->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
    m_d3d11Context->IASetIndexBuffer(m_pQuadIB.get(), DXGI_FORMAT_R32_UINT, 0);
    m_d3d11Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_d3d11Context->VSSetConstantBuffers(0, 1, constantBuffers);

    m_d3d11Context->DrawIndexed(static_cast<UINT>(indices.size()), 0, 0);

    renderedRectsCurrFrame.push_back(RECT{
        static_cast<LONG>(floor(position.x - radius)),
        static_cast<LONG>(floor(position.y - radius)),
        static_cast<LONG>(ceil(position.x + radius)),
        static_cast<LONG>(ceil(position.y + radius)),
        });
}

void
CApp::ClearTexture(const Texture& texture, const D2D1_COLOR_F& color)
{
    wil::com_ptr_nothrow<ID3D11RenderTargetView> renderTargetView;
    wil::com_ptr_nothrow<ID3D11Resource> resource;
    EXIT_IF_FAILED(texture.d3dTexture.query_to(&resource));
    EXIT_IF_FAILED(m_d3d11Device->CreateRenderTargetView(resource.get(), nullptr, &renderTargetView));
    float fcolors[] = { color.b, color.g, color.r, color.a };
    m_d3d11Context->ClearRenderTargetView(renderTargetView.get(), fcolors);
}

void
CApp::UpdateBuffers()
{
    TextureSizeBuffer ssb;
    ssb.screenSize = { static_cast<float>(TEXTURE_WIDTH), static_cast<float>(TEXTURE_HEIGHT) };

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    EXIT_IF_FAILED(m_d3d11Context->Map(m_pTextureSizeBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
    memcpy(mappedResource.pData, &ssb, sizeof(TextureSizeBuffer));
    m_d3d11Context->Unmap(m_pTextureSizeBuffer.get(), 0);
}

// Finds first availalble texture in the pool. Checks fence to ensure that
// texture is not being used by DWM at the moment. For simplicity not shown.
// See original Composition Texture doc for more details.
const Texture&
CApp::FindAvailableTexture()
{
    Texture* pSoonToBeAvailableTexture = nullptr;

    while (pSoonToBeAvailableTexture == nullptr)
    {
        // Find an available texture
        for (auto texture = m_textures.begin(); texture != m_textures.end(); texture++)
        {
            wil::com_ptr_nothrow<ID3D11Fence> fence;
            UINT64 fenceValue;
            EXIT_IF_FAILED(texture->compositionTexture->GetAvailableFence(&fenceValue, IID_PPV_ARGS(&fence)));

            if (fence != nullptr)
            {
                auto completedValue = fence->GetCompletedValue();
                if (completedValue >= fenceValue)
                {
                    return *texture;
                    break;
                }
                else if (pSoonToBeAvailableTexture == nullptr)
                {
                    pSoonToBeAvailableTexture = &*texture;
                }
            }
        }

        if (pSoonToBeAvailableTexture != nullptr)
        {
            wil::com_ptr_nothrow<ID3D11Fence> fence;
            UINT64 fenceValue;
            wil::unique_event waitEvent;
            waitEvent.create();
            EXIT_IF_FAILED(pSoonToBeAvailableTexture->compositionTexture->GetAvailableFence(
                &fenceValue,
                IID_PPV_ARGS(&fence)));
            EXIT_IF_FAILED(fence->SetEventOnCompletion(fenceValue, waitEvent.get()));
            waitEvent.wait();
        }

        if (pSoonToBeAvailableTexture == nullptr)
        {
            Sleep(5);
        }
    }

    return *pSoonToBeAvailableTexture;
}

void
CApp::InitializeScene()
{
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&previousTime);

    auto randf = [](float min, float max) { return static_cast<float>(rand()) / RAND_MAX * (max - min) + min; };
    for (int i = 0; i < N; i++)
    {
        positions[i] = { randf(RADIUS, TEXTURE_WIDTH - RADIUS), randf(RADIUS, TEXTURE_HEIGHT - RADIUS) };
        velocities[i] = { randf(-320.0, 320.0), randf(-320.0, 320.0) };
    }
}

void
CApp::RenderScene(const Texture& texture)
{
    ClearTexture(texture, { 0.2f, 0.2f, 0.2f, 1.0f });
    UpdateBuffers();

    for (UINT i = 0; i < N; i++)
    {
        RenderCircle(texture.d3dTexture.get(), colors[i], positions[i], RADIUS);
    }
}

void
CApp::UpdateScene()
{
    QueryPerformanceCounter(&currentTime);
    LONGLONG elapsedCounts = currentTime.QuadPart - previousTime.QuadPart;
    float dt = static_cast<float>(elapsedCounts) / frequency.QuadPart;
    previousTime = currentTime;

    for (UINT i = 0; i < N; i++)
    {
        positions[i].x += velocities[i].x * dt;
        positions[i].y += velocities[i].y * dt;
        if (positions[i].x < RADIUS && velocities[i].x < 0.0f) {
            velocities[i].x = -velocities[i].x;
        }
        if (positions[i].x > TEXTURE_WIDTH - RADIUS && velocities[i].x > 0.0f) {
            velocities[i].x = -velocities[i].x;
        }
        if (positions[i].y < RADIUS && velocities[i].y < 0.0f) {
            velocities[i].y = -velocities[i].y;
        }
        if (positions[i].y > TEXTURE_HEIGHT - RADIUS && velocities[i].y > 0.0f) {
            velocities[i].y = -velocities[i].y;
        }
    }
}
