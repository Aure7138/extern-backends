#pragma once
#include <cstdint>
#include <memory>

class Pointers
{
public:
	uintptr_t CPedFactory; // CPedFactory
	uintptr_t CNetworkPlayerMgr; // CNetworkPlayerMgr : rage::netPlayerMgrBase
	uintptr_t GtaThread; // GtaThread : rage::scrThread
	uintptr_t CPickupData; // CPickupData : rage::datBase
	uintptr_t iReplayInterface;
	// CReplayInterfaceGame : iReplayInterface
	// CReplayInterfaceCamera : CReplayInterface<camBaseCamera> : iReplayInterface
	// CReplayInterfaceVeh : CReplayInterface<CVehicle> : iReplayInterface
	// CReplayInterfacePed : CReplayInterface<CPed> : iReplayInterface
	// CReplayInterfacePickup : CReplayInterface<CPickup> : iReplayInterface
	// CReplayInterfaceObject : CReplayInterface<CObject> : iReplayInterface
public:
	uintptr_t GlobalPTR;
	uintptr_t BlipPTR;
	uintptr_t AimCPedPTR;
	uintptr_t FriendlistPTR;
	uintptr_t WeatherADDR;
	uintptr_t SettingsADDR;
	uintptr_t ThermalADDR;
	uintptr_t NightvisionADDR;
	uintptr_t BlackoutADDR;
};

inline std::unique_ptr<Pointers> g_pointers;