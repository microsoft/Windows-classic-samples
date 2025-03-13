#pragma once

// Represents a pair of d3d and composition textures with their properties.
struct Texture
{
    wil::com_ptr_nothrow<ID3D11Texture2D> d3dTexture;
    wil::com_ptr_nothrow<IDCompositionTexture> compositionTexture;
};

class CApp
{
public:
    CApp();

    ~CApp();

    void Start(HWND hwnd);

    void Stop();

private:
    void Run();

    // Objects initialization.
    void MakeD3DDevice();
    void MakeDComp();
    void MakeTextures();

    // DComp helpers.
    void DCompSetTexture(const Texture& texture);
    void DCompCommit();

    // D3D Rendering helpers.
    void ClearTexture(const Texture& texture, const D2D1_COLOR_F& color);
    void UpdateBuffers();
    void RenderCircle(ID3D11Texture2D* pTexture, const D2D1_COLOR_F& color, const D2D_POINT_2F& position, float radius);

    // Scene methods.
    void InitializeScene();
    void RenderScene(const Texture& texture);
    void UpdateScene();

    // Get next available texture from the pool.
    const Texture& FindAvailableTexture();

    // Render thread helper.
    static DWORD WINAPI RenderThreadProc(LPVOID lpParameter);

    // Main HWND.
    HWND m_hMainWindow = 0;

    // Dcomp objects.
    wil::com_ptr_nothrow<IDCompositionDevice4> m_dcompDevice;
    wil::com_ptr_nothrow<IDCompositionTarget> m_dcompTarget;
    wil::com_ptr_nothrow<IDCompositionVisual2> m_rootVisual;
    wil::com_ptr_nothrow<PREVIEW_IDCompositionDynamicTexture> m_rootDynamicTexture;

    // D3D Objects.
    wil::com_ptr_nothrow<ID3D11Device5> m_d3d11Device;
    wil::com_ptr_nothrow<ID3D11DeviceContext4> m_d3d11Context;

    wil::com_ptr_nothrow<ID3D11VertexShader> m_pCircleVS;
    wil::com_ptr_nothrow<ID3D11PixelShader> m_pCirclePS;
    wil::com_ptr_nothrow<ID3D11InputLayout> m_pCircleInputLayout;
    wil::com_ptr_nothrow<ID3D11Buffer> m_pTextureSizeBuffer;

    // Texture pool.
    std::vector<Texture> m_textures;

    // Scene.
    static const UINT N = 4;
    const float RADIUS = 25.0f;
    D2D_POINT_2F positions[N];
    D2D_POINT_2F velocities[N];
    D2D1_COLOR_F colors[N] = { {0.9f, 0.2f, 0.1f, 1.0f} , {0.1f, 0.9f, 0.2f, 1.0f}, {0.1f, 0.2f, 0.9f, 1.0f}, {0.8f, 0.2f, 0.9f, 1.0f} };
    LARGE_INTEGER frequency;
    LARGE_INTEGER previousTime, currentTime;

    // Dirty rects.
    std::vector<RECT> renderedRectsPrevFrame;
    std::vector<RECT> renderedRectsCurrFrame;

    // Render thread.
    HANDLE m_hRenderThread = nullptr;
    DWORD m_dwRenderThreadId = 0;
    volatile bool m_running = false;
};
