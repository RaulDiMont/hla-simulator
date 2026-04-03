#include "AircraftFederate.h"

#include <RTI/encoding/BasicDataElements.h>
#include <iostream>
#include <thread>
#include <chrono>

// ------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------

AircraftFederate::AircraftFederate()
{
    // Initial position: Madrid, Spain at 10000m altitude
    _state.latitude  = 40.4168;
    _state.longitude = -3.7038;
    _state.altitude  = 10000.0;
}

AircraftFederate::~AircraftFederate()
{
}

// ------------------------------------------------------------
// initialize: connect to RTI, create and join the federation
// ------------------------------------------------------------

void AircraftFederate::initialize(const std::wstring& federationName,
                                  const std::wstring& fomPath)
{
    _federationName = federationName;

    // Create the RTI ambassador (our communication channel with the RTI)
    rti1516e::RTIambassadorFactory factory;
    _rtiAmbassador = factory.createRTIambassador();

    // Connect to the RTI using thread mode (single process, no network)
    _rtiAmbassador->connect(*this, rti1516e::HLA_EVOKED, L"thread://");

    // Create the federation execution using our FOM file
    try {
        _rtiAmbassador->createFederationExecution(federationName, fomPath);
        std::wcout << L"[Aircraft] Federation created." << std::endl;
    } catch (const rti1516e::FederationExecutionAlreadyExists&) {
        // Another federate already created it, that is fine
        std::wcout << L"[Aircraft] Federation already exists, joining." << std::endl;
    }

    // Join the federation as "AircraftFederate"
    _rtiAmbassador->joinFederationExecution(L"AircraftFederate", federationName);
    std::wcout << L"[Aircraft] Joined federation." << std::endl;

    // Resolve FOM handles for Aircraft class and its attributes
    _aircraftClassHandle = _rtiAmbassador->getObjectClassHandle(L"HLAobjectRoot.Aircraft");
    _latitudeHandle      = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Latitude");
    _longitudeHandle     = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Longitude");
    _altitudeHandle      = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Altitude");

    // Publish the Aircraft object class
    publishAircraft();

    // Register one instance of Aircraft in the federation
    _aircraftInstance = _rtiAmbassador->registerObjectInstance(_aircraftClassHandle);
    std::wcout << L"[Aircraft] Aircraft instance registered." << std::endl;
}

// ------------------------------------------------------------
// publishAircraft: declare to the RTI that we will publish Aircraft
// ------------------------------------------------------------

void AircraftFederate::publishAircraft()
{
    // Build the set of attributes we want to publish
    rti1516e::AttributeHandleSet attributes;
    attributes.insert(_latitudeHandle);
    attributes.insert(_longitudeHandle);
    attributes.insert(_altitudeHandle);

    // Declare to the RTI that this federate publishes these attributes
    _rtiAmbassador->publishObjectClassAttributes(_aircraftClassHandle, attributes);
    std::wcout << L"[Aircraft] Publishing Aircraft attributes." << std::endl;
}

// ------------------------------------------------------------
// run: main simulation loop
// ------------------------------------------------------------

void AircraftFederate::run()
{
    std::wcout << L"[Aircraft] Starting simulation loop." << std::endl;

    for (int i = 0; i < 20; ++i) {

        // Simple flight model: move north at 0.01 degrees per second
        _state.latitude += 0.01;

        // Send updated position to all subscribers
        updatePosition();

        std::wcout << L"[Aircraft] Update " << i + 1
                   << L" | Lat: " << _state.latitude
                   << L" Lon: " << _state.longitude
                   << L" Alt: " << _state.altitude
                   << std::endl;

        // Wait 1 second before next update
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// ------------------------------------------------------------
// updatePosition: serialize and send position to the RTI
// ------------------------------------------------------------

void AircraftFederate::updatePosition()
{
    // Build the attribute-value map to send to the RTI
    // HLAfloat64BE wraps the double and encode() converts it to bytes
    rti1516e::AttributeHandleValueMap attributes;
    attributes[_latitudeHandle]  = rti1516e::HLAfloat64BE(_state.latitude).encode();
    attributes[_longitudeHandle] = rti1516e::HLAfloat64BE(_state.longitude).encode();
    attributes[_altitudeHandle]  = rti1516e::HLAfloat64BE(_state.altitude).encode();

    // Tag to identify this update (empty for now)
    rti1516e::VariableLengthData tag;

    // Send the update to the RTI
    _rtiAmbassador->updateAttributeValues(_aircraftInstance, attributes, tag);
}

// ------------------------------------------------------------
// shutdown: resign and destroy the federation
// ------------------------------------------------------------

void AircraftFederate::shutdown()
{
    // Resign from the federation
    _rtiAmbassador->resignFederationExecution(rti1516e::NO_ACTION);
    std::wcout << L"[Aircraft] Resigned from federation." << std::endl;

    // Destroy the federation (will fail silently if others are still joined)
    try {
        _rtiAmbassador->destroyFederationExecution(_federationName);
        std::wcout << L"[Aircraft] Federation destroyed." << std::endl;
    } catch (const rti1516e::FederatesCurrentlyJoined&) {
        // Other federates still active, federation will be destroyed later
        std::wcout << L"[Aircraft] Other federates still joined, skipping destroy." << std::endl;
    }
}