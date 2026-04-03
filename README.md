# HLA Aircraft Simulator

Basic HLA federation simulator with an Aircraft publisher and a Monitor subscriber.

## Dependencies

- CMake 3.10+
- GCC with C++17 support
- OpenRTI 0.10.0

### Installing OpenRTI
```bash
sudo apt-get install -y cmake g++ libxml2-dev git
git clone https://github.com/onox/OpenRTI.git
cd OpenRTI
mkdir build && cd build
cmake ..
make -j8
sudo make install
sudo apt install pkgconf
```

## Building
```bash
mkdir build && cd build
cmake ..
make -j8
```

## Running
```bash
./aircraft_simulator
```