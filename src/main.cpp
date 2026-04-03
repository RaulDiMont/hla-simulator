#include <memory>
#include <iostream>
#include <thread>
#include <vector>

#include "AircraftFederate.h"
#include "MonitorFederate.h"
#include "RadarFederate.h"

// Federation name
const std::wstring FEDERATION_NAME = L"AircraftSimulation";

// All FOM modules used in this federation
const std::vector<std::wstring> FOM_MODULES = {
    L"../fom/AircraftFOM.xml",
    L"../fom/RadarFOM.xml"
};

// Run the Monitor federate in a separate thread
void runMonitor()
{
    MonitorFederate monitor;
    try {
        monitor.initialize(FEDERATION_NAME, FOM_MODULES);
        monitor.run();
        monitor.shutdown();
    } catch (const rti1516e::Exception& e) {
        std::wcout << L"[Monitor] RTI exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::wcout << L"[Monitor] Exception: " << e.what() << std::endl;
    }
}

// Run the Radar federate in a separate thread
void runRadar()
{
    RadarFederate radar;
    try {
        radar.initialize(FEDERATION_NAME, FOM_MODULES);
        radar.run();
        radar.shutdown();
    } catch (const rti1516e::Exception& e) {
        std::wcout << L"[Radar] RTI exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::wcout << L"[Radar] Exception: " << e.what() << std::endl;
    }
}

// Run the Aircraft federate in the main thread
void runAircraft()
{
    AircraftFederate aircraft;
    try {
        aircraft.initialize(FEDERATION_NAME, FOM_MODULES);
        aircraft.run();
        aircraft.shutdown();
    } catch (const rti1516e::Exception& e) {
        std::wcout << L"[Aircraft] RTI exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::wcout << L"[Aircraft] Exception: " << e.what() << std::endl;
    }
}

int main()
{
    std::wcout << L"=== HLA Aircraft Simulator ===" << std::endl;

    // Launch Monitor and Radar in separate threads
    std::thread monitorThread(runMonitor);
    std::thread radarThread(runRadar);

    // Give Monitor and Radar time to join and subscribe before Aircraft starts
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Run the Aircraft in the main thread
    runAircraft();

    // Wait for all threads to finish
    monitorThread.join();
    radarThread.join();

    std::wcout << L"=== Simulation complete ===" << std::endl;
    return 0;
}