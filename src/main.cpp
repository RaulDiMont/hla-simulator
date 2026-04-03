#include <iostream>
#include <thread>

#include "AircraftFederate.h"
#include "MonitorFederate.h"

// Path to the FOM file relative to the build directory
const std::wstring FOM_PATH        = L"../fom/AircraftFOM.xml";
const std::wstring FEDERATION_NAME = L"AircraftSimulation";

// Run the Monitor federate in a separate thread
void runMonitor()
{
    MonitorFederate monitor;

    try {
        monitor.initialize(FEDERATION_NAME, FOM_PATH);
        monitor.run();
        monitor.shutdown();
    } catch (const rti1516e::Exception& e) {
        std::wcout << L"[Monitor] RTI exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::wcout << L"[Monitor] Exception: " << e.what() << std::endl;
    }
}

// Run the Aircraft federate in the main thread
void runAircraft()
{
    AircraftFederate aircraft;

    try {
        aircraft.initialize(FEDERATION_NAME, FOM_PATH);
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

    // Launch the Monitor in a separate thread so both federates run concurrently
    std::thread monitorThread(runMonitor);

    // Give the Monitor time to join and subscribe before the Aircraft starts
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Run the Aircraft in the main thread
    runAircraft();

    // Wait for the Monitor thread to finish
    monitorThread.join();

    std::wcout << L"=== Simulation complete ===" << std::endl;

    return 0;
}