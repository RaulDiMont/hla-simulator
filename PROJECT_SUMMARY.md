# HLA Aircraft Simulator — Project Summary

## Overview
A distributed aircraft simulation built with HLA (High Level Architecture) 
using OpenRTI, JSBSim and C++. Simulates an Airbus A320 on final approach 
to Madrid-Barajas (LEMD) with a ground radar station tracking the aircraft.

## Architecture

### Federation Structure
Federation: AircraftSimulation
├── AircraftFederate  — Publisher
│   ├── JSBSim A320 flight dynamics model
│   └── Publishes: Latitude, Longitude, Altitude
├── RadarFederate     — Publisher + Subscriber
│   ├── Subscribes to: Aircraft position
│   ├── Calculates: Distance, Bearing, IsInRange (Haversine formula)
│   └── Publishes: RadarContact data
└── MonitorFederate   — Subscriber
├── Subscribes to: Aircraft position + RadarContact
└── Prints all data to console

### FOM Modules
- `AircraftFOM.xml` — Defines Aircraft object class with Latitude, Longitude, Altitude
- `RadarFOM.xml` — Defines RadarContact object class with Distance, Bearing, IsInRange

### Simulation Scenario
- Aircraft: Airbus A320 on final approach to Madrid-Barajas (LEMD)
- Start position: ~62km north of LEMD at 6000ft, heading south at 200 knots
- Radar: Fixed ground station at LEMD (40.4939N, -3.5672W), 60km range
- Duration: 30 updates at 1Hz — aircraft enters radar range at update 8

## Tech Stack
| Component | Technology | Version |
|-----------|-----------|---------|
| HLA Runtime | OpenRTI | 0.10.0 |
| Flight Dynamics | JSBSim | 1.2.5 |
| Aircraft Model | A320 BETA | - |
| Language | C++ | 17 |
| Build System | CMake | 3.28 |
| OS | Ubuntu 22.04 (WSL2) | - |

## Key Design Decisions

### 1. Monitor lifecycle is self-managed
The Monitor decides when to stop based on its own state — when its tracked 
aircraft map is empty. It does not rely on external signals from other federates.

### 2. Federation destruction responsibility
The last federate to leave is responsible for destroying the federation. 
Aircraft and Monitor skip destruction if other federates are still joined.

### 3. removeObjectInstance drives Monitor shutdown
When the aircraft map becomes empty inside removeObjectInstance, the Monitor 
sets _running = false and exits gracefully.

### 4. No external signals between federates
Each federate manages its own lifecycle based on RTI callbacks — no shared 
flags like std::atomic<bool> between federates.

### 5. Simulation frequency
SIMULATION_HZ = 60 — industry standard for certified flight simulators. 
JSBSim runs at 60Hz, RTI is updated once per second.

### 6. Multi-instance support
MonitorFederate and RadarFederate use std::map<ObjectInstanceHandle, ...> 
to track multiple aircraft simultaneously, not just one.

## Technical Debt

### HLA Time Management
Currently using sleep_for(1/SIMULATION_HZ) to pace JSBSim timesteps.
The correct HLA approach is Time Management (TAR/TAG).
Reference: HLA standard IEEE 1516-2010, section 8.
Priority: High — required for production-grade simulation.

### A320 BETA Model Altitude Loss
The JSBSim A320 model is tagged BETA and loses altitude during simulation 
due to unvalidated aerodynamic coefficients. Options to resolve:
- Use a fully validated aircraft model
- Implement altitude hold autopilot (PID controller on elevator)
- Contribute fixes to the JSBSim A320 model

### std::wcout Thread Safety
Two or more federates writing to std::wcout simultaneously causes mixed 
output. Fix: use a mutex-protected logging system or a dedicated log thread.

## Project Structure
hla_simulator/
├── CMakeLists.txt
├── README.md
├── Project.md
├── PROJECT_SUMMARY.md
├── fom/
│   ├── AircraftFOM.xml
│   └── RadarFOM.xml
├── simulation/
│   ├── aircraft/A320/A320.xml
│   └── engine/
│       ├── CFM56_5.xml
│       └── direct.xml
└── src/
├── main.cpp
├── AircraftFederate.h / .cpp
├── MonitorFederate.h  / .cpp
└── RadarFederate.h    / .cpp

## GitHub
https://github.com/RaulDiMont/hla-simulator