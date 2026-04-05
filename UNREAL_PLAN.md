# Unreal Engine Integration Plan

## Objective
Connect the HLA simulator to Unreal Engine so that the aircraft position 
published by AircraftFederate is visualized in real time over georeferenced 
terrain of Madrid using Cesium for Unreal.

## Architecture
HLA Federation
├── AircraftFederate  (C++/WSL2)  — publishes position
├── RadarFederate     (C++/WSL2)  — publishes radar contact
└── UnrealFederate    (C++/Win32) — subscribes to position, drives 3D visualization
│
├── Cesium for Unreal  — real-world Madrid terrain
├── A320 3D model      — aircraft mesh
└── Radar overlay      — visualizes radar range and contact

## Key Challenges

### 1. WSL2 to Windows networking
The HLA federation runs in WSL2. The Unreal federate runs natively on Windows.
OpenRTI needs to be configured to communicate across this boundary.
Solution: Use OpenRTI in network mode (tcp://) instead of thread:// mode.

### 2. OpenRTI on Windows
OpenRTI needs to be compiled for Windows (MSVC or MinGW) to be used in Unreal.
Alternative: Use MSVC-compatible HLA runtime like MAK RTI or Pitch RTI.

### 3. Unreal C++ HLA integration
Unreal uses its own build system (UBT). Integrating OpenRTI as a third-party 
library requires adding it as a module in the .Build.cs file.

## Step by Step Plan

### Phase 4.1 — OpenRTI on Windows
- Compile OpenRTI for Windows using Visual Studio
- Verify connectivity between WSL2 (rtinode) and Windows (Unreal federate)

### Phase 4.2 — Minimal Unreal Federate
- Create a new Unreal C++ project
- Integrate OpenRTI as a third-party plugin
- Implement a minimal FederateActor that subscribes to Aircraft position

### Phase 4.3 — Cesium Integration
- Install Cesium for Unreal plugin
- Configure Cesium ion token for Madrid terrain
- Georeference the scene to Madrid-Barajas coordinates

### Phase 4.4 — Aircraft Visualization
- Import or create a simple A320 mesh
- Drive its position from HLA callbacks
- Convert lat/lon/alt to Unreal world coordinates using Cesium georeferencing

### Phase 4.5 — Radar Visualization
- Draw radar range circle on terrain
- Highlight aircraft when InRange = YES
- Add simple HUD overlay with distance and bearing data

## Recommended Tools
- Unreal Engine 5.3+
- Cesium for Unreal plugin (free, Cesium ion account required)
- Visual Studio 2022 (for Unreal C++ compilation)
- Claude Code for autonomous coding assistance