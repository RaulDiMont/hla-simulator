# Context for Claude Code

## Who I am
Software engineer with 5+ years experience in Unity, XR and Android.
Currently learning HLA simulation for aerospace/defence job applications.

## What we built in the previous Claude.ai session
A distributed HLA simulation in C++ with:
- OpenRTI 0.10.0 as the HLA runtime
- JSBSim 1.2.5 with Airbus A320 BETA model
- 3 federates: AircraftFederate (publisher), RadarFederate (pub+sub), MonitorFederate (subscriber)
- Scenario: A320 on final approach to Madrid-Barajas (LEMD), tracked by ground radar

Full project is at: https://github.com/RaulDiMont/hla-simulator
Full documentation: PROJECT_SUMMARY.md and UNREAL_PLAN.md in the repo

## What we want to do next
Integrate the HLA simulator with Unreal Engine 5 and Cesium for Unreal so that:
1. The A320 position published by AircraftFederate is visualized in real time
2. The aircraft flies over georeferenced Madrid terrain (Cesium)
3. The radar range and contact are visualized as overlays

## Key technical challenges
1. OpenRTI runs in WSL2, Unreal runs natively on Windows — networking needed
2. OpenRTI needs to be compiled for Windows to be used in Unreal C++
3. Unreal uses its own build system (UBT) — OpenRTI integration requires .Build.cs setup
4. Lat/lon/alt from HLA need to be converted to Unreal world coordinates via Cesium

## Important design decisions already made
- HLA communication uses thread:// mode currently — needs to change to tcp:// for cross-process
- All federates manage their own lifecycle based on RTI callbacks
- SIMULATION_HZ = 60 (industry standard)
- No HLA Time Management yet (technical debt)

## Development environment
- Windows 10, Ryzen 7 5800X
- WSL2 Ubuntu 22.04
- Unreal Engine 5.5.4
- JetBrains Rider (IDE for Unreal C++ development)
- Visual Studio 2022 (build tools only - required by Unreal even with Rider)
- OpenRTI installed at /usr/local in WSL2
- Project in WSL2 at ~/hla_simulator

## Preferred working style
- Comments in English always
- Explain each method before writing the next
- Small, focused steps
- Document technical debt in Project.md
- Commit to GitHub after each major milestone
- Prefer European/Airbus references over American/Boeing

## How to start the HLA simulator (WSL2)
```bash
cd ~/hla_simulator/build
./aircraft_simulator
```

## Suggested first message to Claude Code
"I want to integrate my HLA simulator with Unreal Engine 5.5.4 and Cesium for Unreal.
Read CLAUDE_CODE_CONTEXT.md and UNREAL_PLAN.md from my GitHub repo 
https://github.com/RaulDiMont/hla-simulator before starting."