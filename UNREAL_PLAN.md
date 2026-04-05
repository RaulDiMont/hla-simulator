# Unreal Engine Integration Plan

## Objective
Connect the HLA simulator to Unreal Engine 5.5 so that the aircraft position 
published by AircraftFederate is visualized in real time over georeferenced 
terrain of Madrid using Cesium for Unreal.

This demonstrates integration between HLA + JSBSim + Unreal + Cesium — 
exactly the stack used in professional aeronautical simulators, and directly 
relevant to the target job offer (simulation software engineer, Getafe).

## Architecture
HLA Federation (C++/WSL2)
├── AircraftFederate  — publishes A320 position (lat/lon/alt) via HLA
├── RadarFederate     — publishes radar contact data via HLA
└── MonitorFederate   — subscribes to position + radar (console output)
Unreal Engine 5.5 (Windows/Rider)
└── UnrealFederate (C++ Actor)
├── Subscribes to Aircraft position via HLA
├── Converts lat/lon/alt → Unreal world coordinates via Cesium
├── Drives A320 mesh position in real time
└── Visualizes radar range and InRange state as overlays
│
├── Cesium for Unreal  — real-world Madrid-Barajas terrain
├── A320 3D mesh       — simple aircraft model
└── Radar overlay      — range circle + contact highlight

## Development Environment
- OS: Windows 10
- CPU: AMD Ryzen 7 5800X
- Unreal Engine: 5.5.4
- IDE: JetBrains Rider
- Build tools: Visual Studio 2022 (MSVC compiler, required by Unreal)
- HLA runtime: OpenRTI 0.10.0 (in WSL2)

## Key Challenges

### 1. WSL2 to Windows networking
The HLA federation runs in WSL2. The Unreal federate runs natively on Windows.
OpenRTI needs to communicate across this boundary.
Solution: Switch from thread:// to tcp:// mode in OpenRTI, configure WSL2 
network bridge.

### 2. OpenRTI on Windows
OpenRTI must be compiled for Windows (MSVC) to be usable in Unreal C++.
This requires building OpenRTI with Visual Studio build tools.

### 3. Unreal C++ + OpenRTI integration
Unreal uses its own build system (UBT). OpenRTI must be added as a 
third-party module in the .Build.cs file.

### 4. Coordinate conversion
JSBSim outputs lat/lon/alt (WGS84). Unreal uses a local cartesian coordinate 
system. Cesium for Unreal provides the georeferencing API to convert between 
the two systems.

## Step by Step Plan

### Phase 4.1 — OpenRTI on Windows
- Compile OpenRTI for Windows using Visual Studio build tools
- Run rtinode in WSL2, connect from Windows — verify cross-boundary networking
- Write a minimal Windows C++ test that joins the HLA federation

### Phase 4.2 — Unreal Project Setup
- Create a new Unreal Engine 5.5 C++ project
- Install Cesium for Unreal plugin from Epic Marketplace
- Configure Cesium ion token
- Add Madrid-Barajas terrain and verify georeferencing

### Phase 4.3 — OpenRTI as Unreal Third-Party Plugin
- Add OpenRTI headers and .lib files to the Unreal project
- Configure ThirdParty module in .Build.cs
- Verify compilation with a minimal HLA include

### Phase 4.4 — UnrealFederate Actor
- Implement C++ Actor that connects to the HLA federation
- Subscribe to Aircraft position (Latitude, Longitude, Altitude)
- Use Cesium UCesiumGlobeAnchorComponent to position an Actor at lat/lon/alt
- Attach a simple A320 mesh to the Actor

### Phase 4.5 — Radar Visualization
- Subscribe to RadarContact (Distance, Bearing, IsInRange)
- Draw radar range circle on terrain using Unreal procedural mesh
- Change aircraft highlight color when IsInRange = YES

## Recommended First Steps for Claude Code
1. Read this file and CLAUDE_CODE_CONTEXT.md from the GitHub repo
2. Start with Phase 4.1 — compile OpenRTI for Windows
3. Verify WSL2 ↔ Windows HLA connectivity before touching Unreal