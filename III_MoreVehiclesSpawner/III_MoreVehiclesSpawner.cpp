#include <string>
#include "plugin.h"
#include "extensions\KeyCheck.h"
#include "common.h"
#include "CModelInfo.h"
#include "CTimer.h"
#include "CFont.h"
#include "CSprite2d.h"
#include "CStreaming.h"
#include "CBoat.h"
#include "CWorld.h"
#include "CTheScripts.h"
#include "eVehicleModel.h"

float &m_Distance = *(float *)0x5F07DC;
bool b_Counter = false;

using namespace plugin;

class MoreVehiclesSpawner {
public:
    static std::string typedBuffer;
    static std::string errorMessage;
    static std::string errorMessageBuffer;
    static unsigned int errorMessageTimer;
    static bool enabled;

    static void SpawnVehicle(unsigned int modelIndex, CVector position, float orientation) {
        unsigned char oldFlags = CStreaming::ms_aInfoForModel[modelIndex].m_nFlags;
        CStreaming::RequestModel(modelIndex, GAME_REQUIRED);
        CStreaming::LoadAllRequestedModels(false);
        if (CStreaming::ms_aInfoForModel[modelIndex].m_nLoadState == LOADSTATE_LOADED) {
            if (!(oldFlags & GAME_REQUIRED)) {
                CStreaming::SetModelIsDeletable(modelIndex);
                CStreaming::SetModelTxdIsDeletable(modelIndex);
            }
            CVehicle *vehicle = nullptr;
            if (reinterpret_cast<CVehicleModelInfo *>(CModelInfo::ms_modelInfoPtrs[modelIndex])->m_nVehicleType)
                vehicle = new CBoat(modelIndex, 1);
            else
                vehicle = new CAutomobile(modelIndex, 1);
            if (vehicle) {
                // ��������� ��������� � ������� ����
                vehicle->SetPosition(position);
                vehicle->SetOrientation(0.0f, 0.0f, orientation);
                vehicle->m_nState = 4;
                if (modelIndex == MODEL_RCBANDIT)
                    vehicle->m_nDoorLock = CARLOCK_LOCKED;
                else
                    vehicle->m_nDoorLock = CARLOCK_UNLOCKED;
                CWorld::Add(vehicle);
                CTheScripts::ClearSpaceForMissionEntity(position, vehicle);
                if (vehicle->m_nVehicleClass != VEHICLE_BOAT)
                    reinterpret_cast<CAutomobile *>(vehicle)->PlaceOnRoadProperly();
            }
        }
    }

    static void Update() {
        KeyCheck::Update(); // �������� ��������� ������
        if (FindPlayerPed()) {
            //
            if (KeyCheck::CheckWithDelay('O', 1000)) {
                if (b_Counter) {
                    m_Distance = 2.0f; b_Counter = false;
                }
                else {
                    m_Distance = -2.0f; b_Counter = true;
                }
            }
            //
            if (KeyCheck::CheckJustDown(VK_TAB)) { // ���� ������ Tab - �������� ��� ��������� �������
                enabled = !enabled;
                typedBuffer.clear();
                errorMessageBuffer.clear();
            }
            if (enabled) {
                errorMessage.clear();
                if (KeyCheck::CheckWithDelay(VK_BACK, 200)) { // ���� ����� Backspace - ������� ��������� ������ � ������
                    if (typedBuffer.size() > 0) {
                        typedBuffer.pop_back();
                    }
                }
                else {
                    for (int i = 0; i <= 9; i++) {
                        if (KeyCheck::CheckWithDelay(i + 48, 200)) {
                            if (typedBuffer.size() == 4)
                                errorMessage = "Too many digits!";
                            else {
                                typedBuffer.push_back(i + 48); // ��������� ������ � ����� ������
                            }
                            break;
                        }
                    }
                }
                if (KeyCheck::CheckJustDown(45)) { // ���� ������ Insert - ������� ���������
                    if (typedBuffer.size() > 0) {
                        unsigned int modelId = std::stoi(typedBuffer);
                        if (modelId < 5500) {
                            int modelType = CModelInfo::IsVehicleModelType(modelId);
                            if (modelType != -1) {
                                if (modelType == VEHICLE_AUTOMOBILE || modelType == VEHICLE_BOAT) {
                                    SpawnVehicle(modelId, FindPlayerPed()->TransformFromObjectSpace(CVector(0.0f, 4.0f, 0.0f)), FindPlayerPed()->m_fRotationCur + 1.5707964f);
                                    errorMessageBuffer.clear(); // ������� ������� �� ������ (���� ��� ���� �� ������)
                                }
                                else
                                    errorMessage = "Can't spawn a train, heli and plane";
                            }
                            else
                                errorMessage = "This model is not a vehicle!";
                        }
                        else
                            errorMessage = "ID is too big!";
                    }
                    else
                        errorMessage = "Please enter model Id!";
                }
                //
                if (KeyCheck::CheckJustDown('P')) {
                    if (typedBuffer.size() > 0) {
                        unsigned int modelId = std::stoi(typedBuffer);
                        if (modelId < 5500) {
                            int modelType = CModelInfo::IsVehicleModelType(modelId);
                            if (modelType != -1) {
                                if (modelType == VEHICLE_AUTOMOBILE || modelType == VEHICLE_BOAT) {
                                    /*CVector vehiclePos = { -858.0f, -540.0f, 11.0f };
                                    float vehicleAngle = 0.19f;
                                    SpawnVehicle(modelId, vehiclePos, vehicleAngle);*/
                                    SpawnVehicle(modelId, FindPlayerPed()->TransformFromObjectSpace(CVector(1.5f, 6.0f, 0.0f)), FindPlayerPed()->m_fRotationCur + 2.36f); // 0.79 2.36
                                    errorMessageBuffer.clear();
                                }
                                else
                                    errorMessage = "Can't spawn a train, heli and plane";
                            }
                            else
                                errorMessage = "This model is not a vehicle!";
                        }
                        else
                            errorMessage = "ID is too big!";
                    }
                    else
                        errorMessage = "Please enter model Id!";
                }
                //
            }
        }
        else
            enabled = false;
    }

    static void Render() {
        if (enabled) {
            CSprite2d::DrawRect(CRect(10.0f, 10.0f, 300.0f, 130.0f), CRGBA(0, 0, 0, 100));
            CSprite2d::DrawRect(CRect(150.0f, 50.0f, 230.0f, 52.0f), CRGBA(255, 255, 255, 255));

            CFont::SetScale(0.8f, 1.9f);
            CFont::SetColor(CRGBA(255, 255, 255, 255));
            CFont::SetJustifyOn();
            CFont::SetFontStyle(0);
            CFont::SetPropOn();
            CFont::SetWrapx(300.0f);
            CFont::PrintString(15.0f, 15.0f, "Model ID:");
            if (typedBuffer.size() > 0)
                CFont::PrintString(160.0f, 15.0f, const_cast<char*>(typedBuffer.c_str()));
            if (errorMessage.size() > 0) {
                errorMessageBuffer = errorMessage;
                errorMessageTimer = CTimer::m_snTimeInMilliseconds;
            }
            if (errorMessageBuffer.size() > 0 && CTimer::m_snTimeInMilliseconds < (errorMessageTimer + 2000)) {
                CFont::SetColor(CRGBA(255, 0, 0, 255));
                CFont::PrintString(15.0f, 55.0f, const_cast<char*>(errorMessageBuffer.c_str()));
            }
        }
    }

    MoreVehiclesSpawner() {
        Events::gameProcessEvent += Update;
        Events::drawingEvent += Render;
    };
} moreVehiclesSpawner;

std::string MoreVehiclesSpawner::typedBuffer;
std::string MoreVehiclesSpawner::errorMessage;
std::string MoreVehiclesSpawner::errorMessageBuffer;
unsigned int MoreVehiclesSpawner::errorMessageTimer = 0;
bool MoreVehiclesSpawner::enabled = false;
