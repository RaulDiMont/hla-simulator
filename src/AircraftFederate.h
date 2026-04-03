#include <vector>
#include <memory>  // Must be included before any OpenRTI headers
#ifndef AIRCRAFT_FEDERATE_H
#define AIRCRAFT_FEDERATE_H

#include <RTI/RTIambassador.h>
#include <RTI/RTIambassadorFactory.h>
#include <RTI/NullFederateAmbassador.h>

// Aircraft position data structure
struct AircraftState {
    double latitude;
    double longitude;
    double altitude;
};

// Aircraft federate: publishes aircraft position to the federation
class AircraftFederate : public rti1516e::NullFederateAmbassador {
public:
    AircraftFederate();
    ~AircraftFederate();

    // Connect to RTI, create and join the federation
  void initialize(const std::wstring& federationName,
                const std::vector<std::wstring>& fomModules);

    // Main simulation loop: updates and publishes position
    void run();

    // Disconnect from the federation
    void shutdown();

private:
    // Publish Aircraft object class attributes to the RTI
    void publishAircraft();

    // Send a position update to all subscribers
    void updatePosition();

    // RTI ambassador: our interface to communicate with the RTI
    std::unique_ptr<rti1516e::RTIambassador> _rtiAmbassador;

    // Handles to identify objects and attributes in the FOM
    rti1516e::ObjectClassHandle  _aircraftClassHandle;
    rti1516e::AttributeHandle    _latitudeHandle;
    rti1516e::AttributeHandle    _longitudeHandle;
    rti1516e::AttributeHandle    _altitudeHandle;
    rti1516e::ObjectInstanceHandle _aircraftInstance;

    // Current aircraft state
    AircraftState _state;

    // Federation and federate names
    std::wstring _federationName;
};

#endif // AIRCRAFT_FEDERATE_H