#include <intsafe.h>

// Set Agility SDK parameters
// see. https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/

// ReSharper disable once CppInconsistentNaming
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 4; }
// ReSharper disable once CppInconsistentNaming
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }
