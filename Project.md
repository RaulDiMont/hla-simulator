# HLA Simulator - Project Notes

## Federate Roles

| | AircraftFederate | MonitorFederate |
|--|-----------------|-----------------|
| Role | Publisher | Subscriber |
| Overridden callbacks | None | `discoverObjectInstance` + `reflectAttributeValues` |
| Key method | `publishAircraft()` | `subscribeAircraft()` |
| Loop | Fixed 20 iterations | Continuous until `_running = false` |

## Technical Debt

### HLA Time Management
Currently the simulation loop uses `sleep_for(1/SIMULATION_HZ)` to pace 
JSBSim timesteps in real time. This is a simplification — the correct HLA 
approach is to implement Time Management:
- **Time Regulation**: Aircraft federate controls simulation time
- **Time Constrained**: Monitor and Radar federates respect simulation time  
- **Time Advance Request (TAR)**: federates request to advance to next timestep
- **Time Advance Grant (TAG)**: RTI confirms all federates can advance

Reference: HLA standard IEEE 1516-2010, section 8 (Time Management)
Priority: High — required for any production-grade HLA simulation

### A320 BETA Model Altitude Loss
The JSBSim A320 model is tagged as BETA and does not maintain level flight
due to unvalidated aerodynamic coefficients. Altitude is currently fixed
at 10000ft as a workaround. Options to resolve:
- Use a fully validated aircraft model
- Implement a simple altitude hold autopilot (PID controller on elevator)
- Contribute fixes to the JSBSim A320 model