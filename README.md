# HLA Aircraft Simulator

Distributed HLA simulation of an Airbus A320 on final approach to 
Madrid-Barajas (LEMD), tracked by a ground radar station.

## Dependencies

- CMake 3.10+
- GCC with C++17 support
- OpenRTI 0.10.0
- JSBSim 1.2.5

### Installing dependencies

```bash
sudo apt-get install -y cmake g++ libxml2-dev git pkgconf
```

### Installing OpenRTI

```bash
git clone https://github.com/onox/OpenRTI.git
cd OpenRTI
mkdir build && cd build
cmake ..
make -j8
sudo make install
cd ~
```

### Installing JSBSim

```bash
git clone https://github.com/JSBSim-Team/jsbsim.git
cd jsbsim
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON
make -j8
sudo make install
cd ~
```

## Building

```bash
git clone https://github.com/RaulDiMont/hla-simulator.git
cd hla-simulator
mkdir build && cd build
cmake ..
make -j8
```

## Running

```bash
cd hla-simulator/build
./aircraft_simulator
```

## Simulation Scenario

- **Aircraft:** Airbus A320 on final approach to Madrid-Barajas (LEMD)
- **Start position:** ~62km north of LEMD at 6000ft, heading south at 200 knots
- **Radar:** Fixed ground station at LEMD (40.4939N, 3.5672W), 60km range
- **Duration:** 60 updates at 1Hz — aircraft enters radar range at update 8

## Architecture

See [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md) for full architecture and design decisions.

## Related Projects

- [hla-unreal-visualizer](https://github.com/RaulDiMont/hla-unreal-visualizer) — Unreal Engine 5.5 visualization layer for this simulator