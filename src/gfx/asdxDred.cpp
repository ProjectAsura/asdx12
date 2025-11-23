//-----------------------------------------------------------------------------
// File : asdxDredcpp
// Desc : Device Removed Extended Data Reporter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <gfx/asdxDred.h>
#include <fnd/asdxRef.h>
#include <fnd/asdxMisc.h>
#include <cstdio>
#include <sstream>
#include <iomanip>


namespace {

///////////////////////////////////////////////////////////////////////////////
// BreadcrumbOpPair structure
///////////////////////////////////////////////////////////////////////////////
struct BreadcrumbOpPair
{
    D3D12_AUTO_BREADCRUMB_OP Op;
    const char*              Tag;
};

///////////////////////////////////////////////////////////////////////////////
// AllocPair structure
///////////////////////////////////////////////////////////////////////////////
struct AllocPair
{
    D3D12_DRED_ALLOCATION_TYPE Type;
    const char*                Tag;
};

// 自動パンくずリストテーブル.
static const BreadcrumbOpPair kBreadcrumbOps[] = {
    { D3D12_AUTO_BREADCRUMB_OP_SETMARKER                                        , "SETMARKER"                                           }, // 0
    { D3D12_AUTO_BREADCRUMB_OP_BEGINEVENT                                       , "BEGINEVENT"                                          }, // 1
    { D3D12_AUTO_BREADCRUMB_OP_ENDEVENT                                         , "ENDEVENT"                                            }, // 2
    { D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED                                    , "DRAWINSTANCED"                                       }, // 3
    { D3D12_AUTO_BREADCRUMB_OP_DRAWINDEXEDINSTANCED                             , "DRAWINDEXEDINSTANCED"                                }, // 4
    { D3D12_AUTO_BREADCRUMB_OP_EXECUTEINDIRECT                                  , "EXECUTEINDIRECT"                                     }, // 5
    { D3D12_AUTO_BREADCRUMB_OP_DISPATCH                                         , "DISPATCH"                                            }, // 6
    { D3D12_AUTO_BREADCRUMB_OP_COPYBUFFERREGION                                 , "COPYBUFFERREGION"                                    }, // 7
    { D3D12_AUTO_BREADCRUMB_OP_COPYTEXTUREREGION                                , "COPYTEXTUREREGION"                                   }, // 8
    { D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE                                     , "COPYRESOURCE"                                        }, // 9
    { D3D12_AUTO_BREADCRUMB_OP_COPYTILES                                        , "COPYTILES"                                           }, // 10
    { D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCE                               , "RESOLVESUBRESOURCE"                                  }, // 11
    { D3D12_AUTO_BREADCRUMB_OP_CLEARRENDERTARGETVIEW                            , "CLEARRENDERTARGETVIEW"                               }, // 12
    { D3D12_AUTO_BREADCRUMB_OP_CLEARUNORDEREDACCESSVIEW                         , "CLEARUNORDEREDACCESSVIEW"                            }, // 13
    { D3D12_AUTO_BREADCRUMB_OP_CLEARDEPTHSTENCILVIEW                            , "CLEARDEPTHSTENCILVIEW"                               }, // 14
    { D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER                                  , "RESOURCEBARRIER"                                     }, // 15
    { D3D12_AUTO_BREADCRUMB_OP_EXECUTEBUNDLE                                    , "EXECUTEBUNDLE"                                       }, // 16
    { D3D12_AUTO_BREADCRUMB_OP_PRESENT                                          , "PRESENT"                                             }, // 17
    { D3D12_AUTO_BREADCRUMB_OP_RESOLVEQUERYDATA                                 , "RESOLVEQUERYDATA"                                    }, // 18
    { D3D12_AUTO_BREADCRUMB_OP_BEGINSUBMISSION                                  , "BEGINSUBMISSION"                                     }, // 19
    { D3D12_AUTO_BREADCRUMB_OP_ENDSUBMISSION                                    , "ENDSUBMISSION"                                       }, // 20
    { D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME                                      , "DECODEFRAME"                                         }, // 21
    { D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES                                    , "PROCESSFRAMES"                                       }, // 22
    { D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT                             , "ATOMICCOPYBUFFERUINT"                                }, // 23
    { D3D12_AUTO_BREADCRUMB_OP_ATOMICCOPYBUFFERUINT64                           , "ATOMICCOPYBUFFERUINT64"                              }, // 24
    { D3D12_AUTO_BREADCRUMB_OP_RESOLVESUBRESOURCEREGION                         , "RESOLVESUBRESOURCEREGION"                            }, // 25
    { D3D12_AUTO_BREADCRUMB_OP_WRITEBUFFERIMMEDIATE                             , "WRITEBUFFERIMMEDIATE"                                }, // 26
    { D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME1                                     , "DECODEFRAME1"                                        }, // 27
    { D3D12_AUTO_BREADCRUMB_OP_SETPROTECTEDRESOURCESESSION                      , "SETPROTECTEDRESOURCESESSION"                         }, // 28
    { D3D12_AUTO_BREADCRUMB_OP_DECODEFRAME2                                     , "DECODEFRAME2"                                        }, // 29
    { D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES1                                   , "PROCESSFRAMES1"                                      }, // 30
    { D3D12_AUTO_BREADCRUMB_OP_BUILDRAYTRACINGACCELERATIONSTRUCTURE             , "BUILDRAYTRACINGACCELERATIONSTRUCTURE"                }, // 31
    { D3D12_AUTO_BREADCRUMB_OP_EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO , "EMITRAYTRACINGACCELERATIONSTRUCTUREPOSTBUILDINFO"    }, // 32
    { D3D12_AUTO_BREADCRUMB_OP_COPYRAYTRACINGACCELERATIONSTRUCTURE              , "COPYRAYTRACINGACCELERATIONSTRUCTURE"                 }, // 33
    { D3D12_AUTO_BREADCRUMB_OP_DISPATCHRAYS                                     , "DISPATCHRAYS"                                        }, // 34
    { D3D12_AUTO_BREADCRUMB_OP_INITIALIZEMETACOMMAND                            , "INITIALIZEMETACOMMAND"                               }, // 35
    { D3D12_AUTO_BREADCRUMB_OP_EXECUTEMETACOMMAND                               , "EXECUTEMETACOMMAND"                                  }, // 36
    { D3D12_AUTO_BREADCRUMB_OP_ESTIMATEMOTION                                   , "ESTIMATEMOTION"                                      }, // 37
    { D3D12_AUTO_BREADCRUMB_OP_RESOLVEMOTIONVECTORHEAP                          , "RESOLVEMOTIONVECTORHEAP"                             }, // 38
    { D3D12_AUTO_BREADCRUMB_OP_SETPIPELINESTATE1                                , "SETPIPELINESTATE1"                                   }, // 39
    { D3D12_AUTO_BREADCRUMB_OP_INITIALIZEEXTENSIONCOMMAND                       , "INITIALIZEEXTENSIONCOMMAND"                          }, // 40
    { D3D12_AUTO_BREADCRUMB_OP_EXECUTEEXTENSIONCOMMAND                          , "EXECUTEEXTENSIONCOMMAND"                             }, // 41
    { D3D12_AUTO_BREADCRUMB_OP_DISPATCHMESH                                     , "DISPATCHMESH"                                        }, // 42
    { D3D12_AUTO_BREADCRUMB_OP_ENCODEFRAME                                      , "ENCODEFRAME"                                         }, // 43
    { D3D12_AUTO_BREADCRUMB_OP_RESOLVEENCODEROUTPUTMETADATA                     , "RESOLVEENCODEROUTPUTMETADATA"                        }, // 44
    { D3D12_AUTO_BREADCRUMB_OP_BARRIER                                          , "BARRIER"                                             }, // 45
    { D3D12_AUTO_BREADCRUMB_OP_BEGIN_COMMAND_LIST                               , "BEGIN_COMMAND_LIST"                                  }, // 46
    { D3D12_AUTO_BREADCRUMB_OP_DISPATCHGRAPH                                    , "DISPATCHGRAPH"                                       }, // 47
    { D3D12_AUTO_BREADCRUMB_OP_SETPROGRAM                                       , "SETPROGRAM"                                          }, // 48
    //{ D3D12_AUTO_BREADCRUMB_OP_PROCESSFRAMES2                                   , "PROCESSFRAMES2"                                      }, // 52
};

// アロケーションタイプテーブル.
static const AllocPair kAllocTypes[] = {
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_QUEUE              , "COMMAND_QUEUE"               },  // 19
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_ALLOCATOR          , "COMMAND_ALLOCATOR"           },  // 20
    { D3D12_DRED_ALLOCATION_TYPE_PIPELINE_STATE             , "PIPELINE_STATE"              },  // 21
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_LIST               , "COMMAND_LIST"                },  // 22
    { D3D12_DRED_ALLOCATION_TYPE_FENCE                      , "FENCE"                       },  // 23
    { D3D12_DRED_ALLOCATION_TYPE_DESCRIPTOR_HEAP            , "DESCRIPTOR_HEAP"             },  // 24
    { D3D12_DRED_ALLOCATION_TYPE_HEAP                       , "HEAP"                        },  // 25
    { D3D12_DRED_ALLOCATION_TYPE_QUERY_HEAP                 , "QUERY_HEAP"                  },  // 27
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_SIGNATURE          , "COMMAND_SIGNATURE"           },  // 28
    { D3D12_DRED_ALLOCATION_TYPE_PIPELINE_LIBRARY           , "PIPELINE_LIBRARAY"           },  // 29
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER              , "VIDEO_DECORDER"              },  // 30
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_PROCESSOR            , "VIDEO_PROCESSOR"             },  // 32
    { D3D12_DRED_ALLOCATION_TYPE_RESOURCE                   , "RESOURCE"                    },  // 34
    { D3D12_DRED_ALLOCATION_TYPE_PASS                       , "PASS"                        },  // 35
    { D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSION              , "CRYPTOSESSION"               },  // 36
    { D3D12_DRED_ALLOCATION_TYPE_CRYPTOSESSIONPOLICY        , "CRYPTOSESSIONPOLICY"         },  // 37
    { D3D12_DRED_ALLOCATION_TYPE_PROTECTEDRESOURCESESSION   , "PROTECTEDRESOURCESESSION"    },  // 38
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_DECODER_HEAP         , "VIDEO_DECODER_HEAP"          },  // 39
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_POOL               , "COMMAND_POOL"                },  // 40
    { D3D12_DRED_ALLOCATION_TYPE_COMMAND_RECORDER           , "COMMAND_RECORDER"            },  // 41
    { D3D12_DRED_ALLOCATION_TYPE_STATE_OBJECT               , "STATE_OBJECT"                },  // 42
    { D3D12_DRED_ALLOCATION_TYPE_METACOMMAND                , "METACOMMAND"                 },  // 43
    { D3D12_DRED_ALLOCATION_TYPE_SCHEDULINGGROUP            , "SCHEDULINGGROUP"             },  // 44
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_ESTIMATOR     , "VIDEO_MOTION_ESTIMATOR"      },  // 45
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_MOTION_VECTOR_HEAP   , "VIDEO_MOTION_VECTOR_HEAP"    },  // 46
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_EXTENSION_COMMAND    , "VIDEO_EXTENSION_COMMAND"     },  // 47
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER              , "VIDEO_ENCODER"               },  // 48
    { D3D12_DRED_ALLOCATION_TYPE_VIDEO_ENCODER_HEAP         , "VIDEO_ENCODER_HEAP"          },  // 49
    { D3D12_DRED_ALLOCATION_TYPE_INVALID                    , "INVALID"                     },  // 0xffffffff
};

//-----------------------------------------------------------------------------
//      パンくずリストを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_AUTO_BREADCRUMB_OP value)
{
    auto count = _countof(kBreadcrumbOps);
    for(auto i=0u; i<count; ++i)
    {
        if (value == kBreadcrumbOps[i].Op)
        { return kBreadcrumbOps[i].Tag; }
    }

    // バージョンアップとかで列挙体が増えた場合にここに来る可能性がある.
    return "UNKNOWN";
}

//-----------------------------------------------------------------------------
//      アロケーションタイプを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_DRED_ALLOCATION_TYPE value)
{
    auto count = _countof(kAllocTypes);
    for(auto i=0u; i<count; ++i)
    {
        if (value == kAllocTypes[i].Type)
        { return kAllocTypes[i].Tag; }
    }

    // バージョンアップとかで列挙体が増えた場合にここに来る可能性がある.
    return "UNKNOWN";
}

//-----------------------------------------------------------------------------
//      コマンドリストタイプを文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_COMMAND_LIST_TYPE type)
{
    switch(type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return "DIRECT";

    case D3D12_COMMAND_LIST_TYPE_BUNDLE:
        return "BUNDLE";

    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return "COMPUTE";

    case D3D12_COMMAND_LIST_TYPE_COPY:
        return "COPY";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
        return "VIDEO_DECODE";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
        return "VIDEO_PROCESS";

    case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
        return "VIDEO_ENCODE";

    case D3D12_COMMAND_LIST_TYPE_NONE:
        return "NONE";

    default:
        return "UNKNOWN";
    }
}

//-----------------------------------------------------------------------------
//      デバイス状態を文字列に変換します.
//-----------------------------------------------------------------------------
const char* ToString(D3D12_DRED_DEVICE_STATE state)
{
    switch(state)
    {
    case D3D12_DRED_DEVICE_STATE_UNKNOWN:
        return "UNKNOWN";

    case D3D12_DRED_DEVICE_STATE_HUNG:
        return "HUNG";

    case D3D12_DRED_DEVICE_STATE_FAULT:
        return "FAULT";

    case D3D12_DRED_DEVICE_STATE_PAGEFAULT:
        return "PAGEFAULT";

    default:
        return "UKNNOWN";
    }
}

//-----------------------------------------------------------------------------
//      ログファイルに出力します.
//-----------------------------------------------------------------------------
void WriteToLogFile(const std::stringstream& stream)
{
    // 標準エラー出力に表示.
    fprintf_s(stderr, "%s", stream.str().c_str());

    const char* path = "dred.txt";

    // ログファイルに出力.
    FILE* fp = nullptr;
    auto err = fopen_s(&fp, path, "w");
    if (err != 0)
    { return; }

    fprintf_s(fp, "%s", stream.str().c_str());
    fclose(fp);
}


//-----------------------------------------------------------------------------
//      自動パンくずリストをログ出力します.
//-----------------------------------------------------------------------------
void ReportBreadcrumbNode(std::stringstream& stream, const D3D12_AUTO_BREADCRUMB_NODE* pNode)
{
    if (pNode == nullptr)
        return;

    auto count = pNode->BreadcrumbCount;
    auto lastIndex = *pNode->pLastBreadcrumbValue;

    stream << "Breadcrumb Node : " << std::hex << (void*)pNode << std::dec << std::endl;

    if (count == lastIndex && count > 0)
    { stream << "  State                : Completed." << std::endl; }
    else if (lastIndex == 0)
    { stream << "  State                : Not Started." << std::endl; }
    else
    { stream << "  State                : Incompleted!" << std::endl; }

    stream << "  BreadcrumbCount     : " << count << std::endl;
    stream << "  LastBreadcrumbValue : " << lastIndex << std::endl;
    stream << "  Has Next            : " << ((pNode->pNext == nullptr) ? "No" : "Yes") << std::endl;

    if (pNode->pCommandList != nullptr)
    {
        auto type = pNode->pCommandList->GetType();
        stream << "  CommandList : " << std::hex << (void*)pNode->pCommandList << std::dec << std::endl;
        stream << "     Type       = " << ToString(type) << std::endl;
        stream << "     DebugNameA = " << pNode->pCommandListDebugNameA << std::endl;
        stream << "     DebugNameW = " << asdx::ToStringA(pNode->pCommandListDebugNameW).c_str() << std::endl;
    }

    if (pNode->pCommandQueue != nullptr)
    {
        auto desc = pNode->pCommandQueue->GetDesc();
        stream << "  CommandQueue : " << std::hex << (void*)pNode->pCommandQueue << std::dec << std::endl;
        stream << "     Type       = " << ToString(desc.Type) << std::endl;
        stream << "     Priority   = " << desc.Priority << std::endl;
        stream << "     Flags      = " << std::hex << desc.Flags << std::dec << std::endl;
        stream << "     NodeMask   = " << std::hex << desc.NodeMask << std::dec << std::endl;
        stream << "     DebugNameA = " << pNode->pCommandQueueDebugNameA << std::endl;
        stream << "     DebugNameW = " << asdx::ToStringA(pNode->pCommandQueueDebugNameW).c_str() << std::endl;
    }

    for(auto i=0u; i<count; ++i)
    {
        const char* mark = (i < lastIndex) ? "OK" : (i == lastIndex) ? "NG" : "  ";
        auto op = pNode->pCommandHistory[i];

        stream << "    Command Index=" << std::setfill('0') << std::setw(5) << i << std::setfill(' ') << " [" << mark << "] Op = " << ToString(op) << std::endl;
    }
}

//-----------------------------------------------------------------------------
//      自動パンくずリストをログ出力します.
//-----------------------------------------------------------------------------
void ReportBreadcrumbNode1(std::stringstream& stream, const D3D12_AUTO_BREADCRUMB_NODE1* pNode)
{
    if (pNode == nullptr)
        return;

    auto count = pNode->BreadcrumbCount;
    auto lastIndex = *pNode->pLastBreadcrumbValue;

    stream << "Breadcrumb Node : " << std::hex << (void*)pNode << std::dec << std::endl;

    if (count == lastIndex && count > 0)
    { stream << "  State                : Completed." << std::endl; }
    else if (lastIndex == 0)
    { stream << "  State                : Not Started." << std::endl; }
    else
    { stream << "  State                : Incompleted!" << std::endl; }

    stream << "  Breadcrumb Count     : " << count << std::endl;
    stream << "  Last BreadcrumbValue : " << lastIndex << std::endl;
    stream << "  Has Next             : " << ((pNode->pNext == nullptr) ? "No" : "Yes") << std::endl;
    stream << "  Context Count        : " << pNode->BreadcrumbContextsCount << std::endl;

    if (pNode->pCommandList != nullptr)
    {
        auto type = pNode->pCommandList->GetType();
        stream << "  CommandList : " << std::hex << (void*)pNode->pCommandList << std::dec << std::endl;
        stream << "     Type       = " << ToString(type) << std::endl;
        stream << "     DebugNameA = " << pNode->pCommandListDebugNameA << std::endl;
        stream << "     DebugNameW = " << asdx::ToStringA(pNode->pCommandListDebugNameW).c_str() << std::endl;
    }

    if (pNode->pCommandQueue != nullptr)
    {
        auto desc = pNode->pCommandQueue->GetDesc();
        stream << "  CommandQueue : " << std::hex << (void*)pNode->pCommandQueue << std::dec << std::endl;
        stream << "     Type       = " << ToString(desc.Type) << std::endl;
        stream << "     Priority   = " << desc.Priority << std::endl;
        stream << "     Flags      = " << std::hex << desc.Flags << std::dec << std::endl;
        stream << "     NodeMask   = " << std::hex << desc.NodeMask << std::dec << std::endl;
        stream << "     DebugNameA = " << pNode->pCommandQueueDebugNameA << std::endl;
        stream << "     DebugNameW = " << asdx::ToStringA(pNode->pCommandQueueDebugNameW).c_str() << std::endl;
    }

    for(auto i=0u; i<count; ++i)
    {
        const char* mark = (i < lastIndex) ? "OK" : (i == lastIndex) ? "NG" : "  ";
        auto op = pNode->pCommandHistory[i];

        stream << "    Command Index=" << std::setfill('0') << std::setw(5) << i << std::setfill(' ') << " [" << mark << "] Op = " << ToString(op) << std::endl;
    }

    for(auto i=0u; i<pNode->BreadcrumbContextsCount; ++i)
    {
        stream << "  Context Index = " << i 
               << ", BreadcrumbIndex = " << pNode->pBreadcrumbContexts[i].BreadcrumbIndex
               << ", String = " << asdx::ToStringA(pNode->pBreadcrumbContexts[i].pContextString).c_str() << std::endl;
    }
}

//-----------------------------------------------------------------------------
//      アロケーションノードをログ出力します.
//-----------------------------------------------------------------------------
void ReportAllocationNode(std::stringstream& stream, const D3D12_DRED_ALLOCATION_NODE* pNode)
{
    if (pNode == nullptr)
        return;

    stream << "AllocationNode : " << std::hex << (void*)pNode << std::dec << std::endl;
    stream << "  AllocationType = " << ToString(pNode->AllocationType) << std::endl;
    stream << "  ObjectNameA    = " << pNode->ObjectNameA << std::endl;
    stream << "  ObjectNameW    = " << asdx::ToStringA(pNode->ObjectNameW).c_str() << std::endl;
    stream << "  Has Next       = " << ((pNode->pNext != nullptr) ? "Yes" : "No") << std::endl;
}

//-----------------------------------------------------------------------------
//      アロケーションノードをログ出力します.
//-----------------------------------------------------------------------------
void ReportAllocationNode1(std::stringstream& stream, const D3D12_DRED_ALLOCATION_NODE1* pNode)
{
    if (pNode == nullptr)
        return;

    stream << "AllocationNode : " << std::hex << (void*)pNode << std::dec << std::endl;
    stream << "  AllocationType = " << ToString(pNode->AllocationType) << std::endl;
    stream << "  ObjectNameA    = " << pNode->ObjectNameA << std::endl;
    stream << "  ObjectNameW    = " << asdx::ToStringA(pNode->ObjectNameW).c_str() << std::endl;
    stream << "  Has Next       = " << ((pNode->pNext != nullptr) ? "Yes" : "No") << std::endl;
    stream << "  pObject        = " << std::hex << (void*)pNode->pObject << std::dec << std::endl;
}

//-----------------------------------------------------------------------------
//      ページフォルトをログ出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput(std::stringstream& stream, const D3D12_DRED_PAGE_FAULT_OUTPUT& pageFaultOutput)
{
    stream << "Page Fault Virtual Address : " << std::hex << pageFaultOutput.PageFaultVA << std::dec << std::endl;

    {
        stream << "--- Existing Allocation Node ---" << std::endl;
        auto pNode = pageFaultOutput.pHeadExistingAllocationNode;
        if (pNode == nullptr)
        { stream << "None." << std::endl; }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode(stream, pNode);
                pNode = pNode->pNext;
            }
        }

        stream << "--- End Existing Allocation Node --- " << std::endl;
    }

    {
        stream << "--- Recent Freed Allocation Node ---" << std::endl;
        auto pNode = pageFaultOutput.pHeadRecentFreedAllocationNode;
        if (pNode == nullptr)
        { stream << "None." << std::endl; }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode(stream, pNode);
                pNode = pNode->pNext;
            }
        }

        stream << "--- End Recent Freed Allocation Node ---" << std::endl;
    }
}

//-----------------------------------------------------------------------------
//      ページフォルトをログ出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput1(std::stringstream& stream, const D3D12_DRED_PAGE_FAULT_OUTPUT1& pageFaultOutput)
{
    stream << "Page Fault Virtual Address : " << std::hex << pageFaultOutput.PageFaultVA << std::dec << std::endl;

    {
        stream << "--- Existing Allocation Node ---" << std::endl;
        auto pNode = pageFaultOutput.pHeadExistingAllocationNode;
        if (pNode == nullptr)
        { stream << "None." << std::endl; }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode1(stream, pNode);
                pNode = pNode->pNext;
            }
        }

        stream << "--- End Existing Allocation Node --- " << std::endl;
    }

    {
        stream << "--- Recent Freed Allocation Node ---" << std::endl;
        auto pNode = pageFaultOutput.pHeadRecentFreedAllocationNode;
        if (pNode == nullptr)
        { stream << "None." << std::endl; }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode1(stream, pNode);
                pNode = pNode->pNext;
            }
        }

        stream << "--- End Recent Freed Allocation Node ---" << std::endl;
    }
}

//-----------------------------------------------------------------------------
//      ページフォルトをログ出力します.
//-----------------------------------------------------------------------------
void ReportPageFaultOutput2(std::stringstream& stream, const D3D12_DRED_PAGE_FAULT_OUTPUT2& pageFaultOutput)
{
    stream << "Page Fault Virtual Address : " << std::hex << pageFaultOutput.PageFaultVA << std::dec << std::endl;
    stream << "Page Fault Flags           : " << std::hex << pageFaultOutput.PageFaultFlags << std::dec << std::endl;

    {
        stream << "--- Existing Allocation Node ---" << std::endl;
        auto pNode = pageFaultOutput.pHeadExistingAllocationNode;
        if (pNode == nullptr)
        { stream << "None." << std::endl; }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode1(stream, pNode);
                pNode = pNode->pNext;
            }
        }

        stream << "--- End Existing Allocation Node --- " << std::endl;
    }

    {
        stream << "--- Recent Freed Allocation Node ---" << std::endl;
        auto pNode = pageFaultOutput.pHeadRecentFreedAllocationNode;
        if (pNode == nullptr)
        { stream << "None." << std::endl; }
        else
        {
            while(pNode != nullptr)
            {
                ReportAllocationNode1(stream, pNode);
                pNode = pNode->pNext;
            }
        }

        stream << "--- End Recent Freed Allocation Node ---" << std::endl;
    }
}

//-----------------------------------------------------------------------------
//      デバイス削除拡張データをログ出力します.
//-----------------------------------------------------------------------------
bool ReportDred0(std::stringstream& stream, ID3D12Device* pDevice)
{
    asdx::RefPtr<ID3D12DeviceRemovedExtendedData> dred;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(dred.GetAddress()));
    if (FAILED(hr))
    { return false; }

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbsOutput = {};
    hr = dred->GetAutoBreadcrumbsOutput(&breadcrumbsOutput);
    if (SUCCEEDED(hr))
    {
        auto pNode = breadcrumbsOutput.pHeadAutoBreadcrumbNode;
        while(pNode != nullptr)
        {
            ReportBreadcrumbNode(stream, pNode);
            pNode = pNode->pNext;
        }
    }

    D3D12_DRED_PAGE_FAULT_OUTPUT pageFaultOutput = {};
    hr = dred->GetPageFaultAllocationOutput(&pageFaultOutput);
    if (SUCCEEDED(hr))
    { ReportPageFaultOutput(stream, pageFaultOutput); }

    WriteToLogFile(stream);

    return true;
}

//-----------------------------------------------------------------------------
//      デバイス削除拡張データをログ出力します.
//-----------------------------------------------------------------------------
bool ReportDred1(std::stringstream& stream, ID3D12Device* pDevice)
{
    asdx::RefPtr<ID3D12DeviceRemovedExtendedData1> dred;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(dred.GetAddress()));
    if (FAILED(hr))
    { return false; }

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbsOutput1 = {};
    hr = dred->GetAutoBreadcrumbsOutput1(&breadcrumbsOutput1);
    if (SUCCEEDED(hr))
    {
        auto pNode = breadcrumbsOutput1.pHeadAutoBreadcrumbNode;
        while(pNode != nullptr)
        {
            ReportBreadcrumbNode1(stream, pNode);
            pNode = pNode->pNext;
        }
    }
    else
    {
        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumbsOutput = {};
        hr = dred->GetAutoBreadcrumbsOutput(&breadcrumbsOutput);
        if (SUCCEEDED(hr))
        {
            auto pNode = breadcrumbsOutput.pHeadAutoBreadcrumbNode;
            while(pNode != nullptr)
            {
                ReportBreadcrumbNode(stream, pNode);
                pNode = pNode->pNext;
            }
        }
    }

    D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFaultOutput1 = {};
    hr = dred->GetPageFaultAllocationOutput1(&pageFaultOutput1);
    if (SUCCEEDED(hr))
    { ReportPageFaultOutput1(stream, pageFaultOutput1); }
    else
    {
        D3D12_DRED_PAGE_FAULT_OUTPUT pageFaultOutput = {};
        hr = dred->GetPageFaultAllocationOutput(&pageFaultOutput);
        if (SUCCEEDED(hr))
        { ReportPageFaultOutput(stream, pageFaultOutput); }
    }

    WriteToLogFile(stream);

    return true;
}

//-----------------------------------------------------------------------------
//      デバイス削除拡張データをログ出力します.
//-----------------------------------------------------------------------------
bool ReportDred2(std::stringstream& stream, ID3D12Device* pDevice)
{
    asdx::RefPtr<ID3D12DeviceRemovedExtendedData2> dred;
    auto hr = pDevice->QueryInterface(IID_PPV_ARGS(dred.GetAddress()));
    if (FAILED(hr))
    { return false; }

    auto state = dred->GetDeviceState();
    stream << "DeviceState           : " <<  ToString(state) << std::endl;

    D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 breadcrumbsOutput1 = {};
    hr = dred->GetAutoBreadcrumbsOutput1(&breadcrumbsOutput1);
    if (SUCCEEDED(hr))
    {
        auto pNode = breadcrumbsOutput1.pHeadAutoBreadcrumbNode;
        while(pNode != nullptr)
        {
            ReportBreadcrumbNode1(stream, pNode);
            pNode = pNode->pNext;
        }
    }
    else
    {
        D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadCrumbsOutput = {};
        hr = dred->GetAutoBreadcrumbsOutput(&breadCrumbsOutput);
        if (SUCCEEDED(hr))
        {
            auto pNode = breadCrumbsOutput.pHeadAutoBreadcrumbNode;
            while(pNode != nullptr)
            {
                ReportBreadcrumbNode(stream, pNode);
                pNode = pNode->pNext;
            }
        }
    }

    D3D12_DRED_PAGE_FAULT_OUTPUT2 pageFalutOutput2 = {};
    hr = dred->GetPageFaultAllocationOutput2(&pageFalutOutput2);
    if (SUCCEEDED(hr))
    { ReportPageFaultOutput2(stream, pageFalutOutput2); }
    else
    {
        D3D12_DRED_PAGE_FAULT_OUTPUT1 pageFaultOutput1 = {};
        hr = dred->GetPageFaultAllocationOutput1(&pageFaultOutput1);
        if (SUCCEEDED(hr))
        { ReportPageFaultOutput1(stream, pageFaultOutput1); }
        else
        {
            D3D12_DRED_PAGE_FAULT_OUTPUT pageFaultOutput = {};
            hr = dred->GetPageFaultAllocationOutput(&pageFaultOutput);
            if (SUCCEEDED(hr))
            { ReportPageFaultOutput(stream, pageFaultOutput); }
        }
    }

    WriteToLogFile(stream);

    return true;
}

} // namespace

namespace asdx {

//-----------------------------------------------------------------------------
//      DRED情報を出力します.
//-----------------------------------------------------------------------------
void ReportDRED(ID3D12Device8* pDevice, HRESULT hr)
{
    if (pDevice == nullptr)
        return;

    auto reason = pDevice->GetDeviceRemovedReason();

    std::stringstream stream;
    stream << "Error Code            : " << std::hex << hr << std::dec << std::endl;
    stream << "Error Message         : " << std::system_category().message(hr).c_str() << std::endl;
    stream << "Device Removed Reason : " << std::system_category().message(reason).c_str() << std::endl;

    // 新しいバージョンから順に実行していき，実行出来たら終了.

    // DRED 1.2
    if (ReportDred2(stream, pDevice))
    { return; }

    // DRED 1.1
    if (ReportDred1(stream, pDevice))
    { return; }

    // DRED 1.0
    if (ReportDred0(stream, pDevice))
    { return; }

    // それでもダメな場合は，エラーコードのみログ出力して終了.
    WriteToLogFile(stream);
}

} // namespace asdx
