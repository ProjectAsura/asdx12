//-----------------------------------------------------------------------------
// File : asdxModelManagerImpl.cpp
// Desc : Model Manager Implementation.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxModelManagerImpl.h"
#include <res/asdxResModel.h>
#include <fnd/asdxLogger.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ModelManager class
///////////////////////////////////////////////////////////////////////////////
ModelManager ModelManager::s_Instance = {};

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ModelManager::ModelManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ModelManager::~ModelManager()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      シングルトンインスタンスを取得します.
//-----------------------------------------------------------------------------
ModelManager& ModelManager::Instance()
{ return s_Instance; }

//-----------------------------------------------------------------------------
//      初期化処理を行います.
//-----------------------------------------------------------------------------
bool ModelManager::Init(uint32_t maxInstanceCount)
{
    if (m_Init)
        return false;

    auto size = uint32_t(sizeof(uint32_t) * maxInstanceCount);
    m_OffsetAllocator.Init(size, maxInstanceCount);
    m_Init = true;

    return true;
}

//-----------------------------------------------------------------------------
//      終了処理を行います.
//-----------------------------------------------------------------------------
void ModelManager::Term()
{
    if (!m_Init)
        return;

    // モデルリストを破棄します.
    {
        auto itr = m_Models.begin();
        while(itr != m_Models.end())
        {
            auto model = &(*itr);
            itr = m_Models.erase(itr);
            if (model)
            {
                model->Release();
                model = nullptr;
            }
        }
    }

    // モデルインスタンスリストを破棄します.
    {
        auto itr = m_ModelInstances.begin();
        while(itr != m_ModelInstances.end())
        {
            auto instance = &(*itr);
            itr = m_ModelInstances.erase(itr);
            if (instance)
            {
                instance->Release();
                instance = nullptr;
            }
        }
    }

    // オフセットアロケータの終了処理.
    m_OffsetAllocator.Term();

    m_Init = false;
}

//-----------------------------------------------------------------------------
//      初期化済みかどうかチェックします.
//-----------------------------------------------------------------------------
bool ModelManager::IsInit() const
{ return m_Init; }

//-----------------------------------------------------------------------------
//      モデルを生成します.
//-----------------------------------------------------------------------------
bool ModelManager::CreateModel(const std::vector<uint8_t>& blob, IModel** ppModel)
{
    ModelBinary binary;
    std::vector<uint8_t> copyBlob = blob;
    binary.Load(std::move(copyBlob));

    Model* pModel = nullptr;
    if (!Model::Create(binary, &pModel))
    {
        ELOG("Error : Model::Create() Failed.");
        return false;
    }

    m_Models.push_back(pModel);
    (*ppModel) = pModel;
    return true;
}

//-----------------------------------------------------------------------------
//      モデルインスタンスを生成します.
//-----------------------------------------------------------------------------
bool ModelManager::CreateModelInstance(IModel* pModel, IModelInstance** ppInstance)
{
    auto handle = m_OffsetAllocator.Alloc(1);

    ModelInstance* pInstance = nullptr;
    if (!ModelInstance::Create(static_cast<Model*>(pModel), handle, &pInstance))
    {
        ELOG("Error : ModelInstance::Create() Failed.");
        return false;
    }

    m_ModelInstances.push_back(pInstance);
    (*ppInstance) = pInstance;
    return true;
}

//-----------------------------------------------------------------------------
//      各モデルに対して処理を行います.
//-----------------------------------------------------------------------------
void ModelManager::ForEachModel(OnModel action, void* userData)
{
    if (action == nullptr)
        return;

    for(auto& itr : m_Models)
        action(&itr, userData);
}

//-----------------------------------------------------------------------------
//      各モデルインスタンスに対して処理を行います.
//-----------------------------------------------------------------------------
void ModelManager::ForEachModelInstance(OnModelInstance action, void* userData)
{
    if (action == nullptr)
        return;

    for(auto& itr : m_ModelInstances)
        action(&itr, userData);
}

//-----------------------------------------------------------------------------
//      モデルを管理対象から外します.
//-----------------------------------------------------------------------------
void ModelManager::RemoveModel(Model* pModel)
{
    m_Models.erase(pModel);
}

//-----------------------------------------------------------------------------
//      モデルインスタンスを管理対象から外します.
//-----------------------------------------------------------------------------
void ModelManager::RemoveModelInstance(ModelInstance* pInstance)
{
    auto& handle = pInstance->GetOffsetHandle();
    m_OffsetAllocator.Free(handle);
    m_ModelInstances.erase(pInstance);
}

//-----------------------------------------------------------------------------
//      モデルリストを取得します.
//-----------------------------------------------------------------------------
List<Model>& ModelManager::GetModels()
{ return m_Models; }

//-----------------------------------------------------------------------------
//      モデルインスタンスリストを取得します.
//-----------------------------------------------------------------------------
List<ModelInstance>& ModelManager::GetModelInstances()
{ return m_ModelInstances; }

//-----------------------------------------------------------------------------
//      モデルリストを取得します.
//-----------------------------------------------------------------------------
const List<Model>& ModelManager::GetModels() const
{ return m_Models; }

//-----------------------------------------------------------------------------
//      モデルインスタンスリストを取得します.
//-----------------------------------------------------------------------------
const List<ModelInstance>& ModelManager::GetModelInstances() const
{ return m_ModelInstances; }

//-----------------------------------------------------------------------------
//      モデルマネージャの初期化処理を行います.
//-----------------------------------------------------------------------------
bool InitModelManager(uint32_t maxInstanceCount)
{ return ModelManager::Instance().Init(maxInstanceCount); }

//-----------------------------------------------------------------------------
//      モデルマネージャの終了処理を行います.
//-----------------------------------------------------------------------------
void TermModelManager()
{ ModelManager::Instance().Term(); }

//-----------------------------------------------------------------------------
//      モデルマネージャが初期化済みかどうかチェックします.
//-----------------------------------------------------------------------------
bool IsInitModelManager()
{ return ModelManager::Instance().IsInit(); }

//-----------------------------------------------------------------------------
//      モデルマネージャを取得します.
//-----------------------------------------------------------------------------
IModelManager& GetModelManager()
{ return ModelManager::Instance(); }

} // namespace asdx
