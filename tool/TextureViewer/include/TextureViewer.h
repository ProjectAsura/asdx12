//-----------------------------------------------------------------------------
// File : TextureViewer.h
// Desc : Texutre Viewer Application.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string>
#include <vector>
#include <atomic>
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

///////////////////////////////////////////////////////////////////////////////
// TextureViewer class
///////////////////////////////////////////////////////////////////////////////
class TextureViewer
{
public:
    TextureViewer();

    ~TextureViewer();

    int Run();


private:
    HINSTANCE   m_hInstance = nullptr;
    HWND        m_hWnd = nullptr;
    UINT        m_Width = 0;
    UINT        m_Height = 0;
    bool        m_StandByMode = false;
    UINT64      m_FenceValue = 1;
    UINT64      m_PrevFenceValue = 0;


    IDXGIFactory2*              m_pDXGIFactory = nullptr;
    ID3D12Device*               m_pDevice = nullptr;
    IDXGISwapChain4*            m_pSwapChain = nullptr;
    ID3D12CommandQueue*         m_pGraphicsQueue = nullptr;
    ID3D12CommandAllocator*     m_pCommandAllocator[2] = {};
    ID3D12GraphicsCommandList*  m_pCommandList = nullptr;

    ID3D12DescriptorHeap*       m_pDescriptorHeapRTV = nullptr;
    ID3D12DescriptorHeap*       m_pDescriptorHeapDSV = nullptr;
    ID3D12DescriptorHeap*       m_pDescriptorHeapResource = nullptr;

    ID3D12Resource* m_pColorBuffer[2] = {};
    ID3D12Resource* m_pDepthBuffer = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE     m_ColorBufferHandle[2];
    D3D12_CPU_DESCRIPTOR_HANDLE     m_DepthBufferHandle;

    ID3D12Fence*        m_pFence = nullptr;
    HANDLE              m_FenceEvent = nullptr;

    std::atomic<bool> m_StopDraw = false;
    std::string         m_InputPath;
    std::string         m_OutputPath;
    int                 m_TextureDrawWays = 0;

    bool Init();
    void Term();
    int  MainLoop();

    void Present(UINT syncInterval);

    void Wait(UINT64 fenceValue, UINT32 timeout);
    void WaitIdle();

    void OnDraw();
    void OnResize(UINT w, UINT h);
    void OnDrop(const std::vector<std::string>& files);
    void OnMouse(int x, int y, int wheelDelta, bool isDownL, bool isDownM, bool isDownR);
    void OnKey(UINT keyCode, bool isKeyDown, bool isAltDown);
    void OnTyping(UINT keyCode);

    void UIStatus();
    void UIPopupMenu();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};