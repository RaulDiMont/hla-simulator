#ifndef AIRCRAFT_FEDERATE_H
#define AIRCRAFT_FEDERATE_H

#include <memory>
#include <vector>

#include <RTI/RTIambassador.h>
#include <RTI/RTIambassadorFactory.h>
#include <RTI/NullFederateAmbassador.h>

#include <JSBSim/FGFDMExec.h>
#include <JSBSim/initialization/FGInitialCondition.h>

// Aircraft position and attitude data structure
struct AircraftState {
    double latitude;   // degrees
    double longitude;  // degrees
    double altitude;   // feet above sea level
};

// Simulation frequency in Hz - industry standard for certified flight simulators
static const int SIMULATION_HZ = 60;
const int seconds = 60;

// Aircraft federate: uses JSBSim to simulate A320 flight dynamics
// and publishes position to the HLA federation
class AircraftFederate : public rti1516e::NullFederateAmbassador {
public:
    AircraftFederate();
    ~AircraftFederate();

    // Connect to RTI, create and join the federation
    void initialize(const std::wstring& federationName,
                    const std::vector<std::wstring>& fomModules);

    // Main simulation loop
    void run();

    // Disconnect from the federation
    void shutdown();

private:
    // Initialize JSBSim with the A320 model
    void initializeJSBSim();

    // Publish Aircraft object class attributes to the RTI
    void publishAircraft();

    // Update JSBSim by one timestep and read back position
    void calculatePosition();

    // Send a position update to all subscribers
    void updatePosition();

    // RTI ambassador: our interface to communicate with the RTI
    std::unique_ptr<rti1516e::RTIambassador> _rtiAmbassador;

    // JSBSim flight dynamics model executor
    std::unique_ptr<JSBSim::FGFDMExec> _fdm;

    // Handles to identify objects and attributes in the FOM
    rti1516e::ObjectClassHandle    _aircraftClassHandle;
    rti1516e::AttributeHandle      _latitudeHandle;
    rti1516e::AttributeHandle      _longitudeHandle;
    rti1516e::AttributeHandle      _altitudeHandle;
    rti1516e::ObjectInstanceHandle _aircraftInstance;

    // Current aircraft state read from JSBSim
    AircraftState _state;

    // Federation name
    std::wstring _federationName;
};

#endif // AIRCRAFT_FEDERATE_H