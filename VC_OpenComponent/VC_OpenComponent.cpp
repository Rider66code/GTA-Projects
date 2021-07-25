/*
Plugin-SDK (Grand Theft Auto) source file
Authors: GTA Community. See more here
https://github.com/DK22Pac/plugin-sdk
Do not delete this comment block. Respect others' work!
*/
#include <plugin_vc.h>
#include "game_vc\common.h"
#include "game_vc\CAutomobile.h"
#include "game_vc\CTimer.h"

using namespace plugin;

const float ACTION_TIME_STEP = 0.05f;
const unsigned int TIME_FOR_KEYPRESS = 500;

class DoorsExample {
public:
    static int componentByDoorId[6]; // ������� �������� eDoors � Id ����������

    static int m_nLastTimeWhenAnyActionWasEnabled; // ��������� ����� ������� �������

    enum eDoorEventType { // ��� �������
        DOOR_EVENT_OPEN,
        DOOR_EVENT_CLOSE
    };

    class DoorEvent { // ����� �������
    public:
        bool m_active;
        eDoorEventType m_type;
        float m_openingState;

        DoorEvent() {
            m_active = false;
            m_type = DOOR_EVENT_CLOSE;
        }
    };

    class VehicleDoors {
    public:
        DoorEvent events[6]; // ������� ��� ���� 6 ������

        VehicleDoors(CVehicle *) {}
    };

    static VehicleExtendedData<VehicleDoors> VehDoors; // ���� ����������

    static void EnableDoorEvent(CAutomobile *automobile, eDoors doorId) { // �������� ������� �����
        if (automobile->IsComponentPresent(componentByDoorId[doorId])) {
            if (automobile->m_carDamage.GetDoorStatus(doorId) != DAMSTATE_NOTPRESENT) {
                DoorEvent &event = VehDoors.Get(automobile).events[doorId];
                if (event.m_type == DOOR_EVENT_OPEN)
                    event.m_type = DOOR_EVENT_CLOSE; // ���� ��������� ������� - ��������, �� ���������
                else
                    event.m_type = DOOR_EVENT_OPEN; // ���� ��������� ������� �������� - �� ���������
                event.m_active = true; // �������� ���������
                m_nLastTimeWhenAnyActionWasEnabled = CTimer::m_snTimeInMilliseconds;
            }
        }
    }

    static void ProcessDoors(CVehicle *vehicle) { // ��������� ������� ��� ����������� ����
        if (vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
            CAutomobile *automobile = reinterpret_cast<CAutomobile *>(vehicle);
            for (unsigned int i = 0; i < 6; i++) { // ������������ ��� �������
                eDoors doorId = static_cast<eDoors>(i);
                DoorEvent &event = VehDoors.Get(automobile).events[doorId];
                if (event.m_active) { // ���� ������� �������
                    if (event.m_type == DOOR_EVENT_OPEN) {
                        event.m_openingState += ACTION_TIME_STEP;
                        if (event.m_openingState > 1.0f) { // ���� ��������� �������
                            event.m_active = false; // ��������� ���������
                            automobile->OpenDoor(componentByDoorId[doorId], doorId, 1.0f); // ��������� ���������
                            event.m_openingState = 1.0f;
                        }
                        else
                            automobile->OpenDoor(componentByDoorId[doorId], doorId, event.m_openingState);
                    }
                    else {
                        event.m_openingState -= ACTION_TIME_STEP;
                        if (event.m_openingState < 0.0f) { // ���� ��������� �������
                            event.m_active = false; // ��������� ���������
                            automobile->OpenDoor(componentByDoorId[doorId], doorId, 0.0f); // ��������� ���������
                            event.m_openingState = 0.0f;
                        }
                        else
                            automobile->OpenDoor(componentByDoorId[doorId], doorId, event.m_openingState);
                    }
                }
            }
        }
    }

    static void MainProcess() { // ��������� ������� ������ � ������ �������
        if (CTimer::m_snTimeInMilliseconds > (m_nLastTimeWhenAnyActionWasEnabled + TIME_FOR_KEYPRESS)) { // ���� ������ 500 �� � ���� �������, ��� �� ������ ���������/��������� ���-��
            CVehicle *vehicle = FindPlayerVehicle();
            if (vehicle && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
                CAutomobile *automobile = reinterpret_cast<CAutomobile *>(vehicle); // ����� ��, ���������� �����. �.�. �� ����� ����� damageManager, ��� ����� ���������, ��� ��������� - ��� ���������� (CAutomobile)
                if (KeyPressed(219)) // [
                    EnableDoorEvent(automobile, BONNET); // �����
                else if (KeyPressed(221)) // ]
                    EnableDoorEvent(automobile, BOOT); // ��������
                else if (KeyPressed(186) && KeyPressed(187)) // ; =
                    EnableDoorEvent(automobile, DOOR_FRONT_LEFT); // ����� �������� �����
                else if (KeyPressed(222) && KeyPressed(187)) // ' =
                    EnableDoorEvent(automobile, DOOR_FRONT_RIGHT); // ������ �������� �����
                else if (KeyPressed(186) && KeyPressed(189)) // ; -
                    EnableDoorEvent(automobile, DOOR_REAR_LEFT); // ����� ������ �����
                else if (KeyPressed(222) && KeyPressed(189)) // ' -
                    EnableDoorEvent(automobile, DOOR_REAR_RIGHT); // ������ ������ �����
                else if (KeyPressed(VK_F12)) {
                    EnableDoorEvent(automobile, BONNET);
                    EnableDoorEvent(automobile, BOOT);
                    EnableDoorEvent(automobile, DOOR_FRONT_LEFT);
                    EnableDoorEvent(automobile, DOOR_FRONT_RIGHT);
                    EnableDoorEvent(automobile, DOOR_REAR_LEFT);
                    EnableDoorEvent(automobile, DOOR_REAR_RIGHT);
                }
            }
        }
    }

    DoorsExample() {
        Events::gameProcessEvent += MainProcess; // ��� ������������ ������� � ��������� �������
        Events::vehicleRenderEvent += ProcessDoors; // ��� ������������ �������, � ����� ��������� ��
    }
} example;

int DoorsExample::componentByDoorId[6] = { CAR_BONNET, CAR_BOOT, CAR_DOOR_LF, CAR_DOOR_RF, CAR_DOOR_LR, CAR_DOOR_RR };
int DoorsExample::m_nLastTimeWhenAnyActionWasEnabled = 0;
VehicleExtendedData<DoorsExample::VehicleDoors> DoorsExample::VehDoors;
