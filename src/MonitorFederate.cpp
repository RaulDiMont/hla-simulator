#include "MonitorFederate.h"

#include <RTI/encoding/BasicDataElements.h>
#include <iostream>

// ------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------

MonitorFederate::MonitorFederate() : _running(false)
{
}

MonitorFederate::~MonitorFederate()
{
}

// ------------------------------------------------------------
// initialize: connect to RTI and join the federation
// ------------------------------------------------------------

void MonitorFederate::initialize(const std::wstring &federationName,
                                 const std::vector<std::wstring> &fomModules)
{
    _federationName = federationName;

    // Create the RTI ambassador
    rti1516e::RTIambassadorFactory factory;
    _rtiAmbassador = factory.createRTIambassador();

    // Connect to the RTI using thread mode
    _rtiAmbassador->connect(*this, rti1516e::HLA_EVOKED, L"thread://");

    // Create the federation execution using all FOM modules
    try
    {
        _rtiAmbassador->createFederationExecution(federationName, fomModules);
        std::wcout << L"[Monitor] Federation created." << std::endl;
    }
    catch (const rti1516e::FederationExecutionAlreadyExists &)
    {
        std::wcout << L"[Monitor] Federation already exists, joining." << std::endl;
    }

    // Join the federation as "MonitorFederate"
    _rtiAmbassador->joinFederationExecution(L"MonitorFederate", federationName);
    std::wcout << L"[Monitor] Joined federation." << std::endl;

    // Resolve FOM handles for Aircraft class and its attributes
    _aircraftClassHandle = _rtiAmbassador->getObjectClassHandle(L"HLAobjectRoot.Aircraft");
    _latitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Latitude");
    _longitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Longitude");
    _altitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Altitude");

    // Subscribe to Aircraft attributes
    subscribeAircraft();
}

// ------------------------------------------------------------
// subscribeAircraft: declare to RTI that we want to receive Aircraft updates
// ------------------------------------------------------------

void MonitorFederate::subscribeAircraft()
{
    // Build the set of attributes we want to receive
    rti1516e::AttributeHandleSet attributes;
    attributes.insert(_latitudeHandle);
    attributes.insert(_longitudeHandle);
    attributes.insert(_altitudeHandle);

    // Declare to the RTI that this federate subscribes to these attributes
    _rtiAmbassador->subscribeObjectClassAttributes(_aircraftClassHandle, attributes);
    std::wcout << L"[Monitor] Subscribed to Aircraft attributes." << std::endl;
}

// ------------------------------------------------------------
// run: main loop, process incoming RTI callbacks
// ------------------------------------------------------------

void MonitorFederate::run()
{
    std::wcout << L"[Monitor] Waiting for aircraft updates..." << std::endl;

    _running = true;
    while (_running)
    {
        // Ask the RTI to process pending callbacks (blocking up to 1 second)
        _rtiAmbassador->evokeMultipleCallbacks(0.1, 1.0);
    }
}

// ------------------------------------------------------------
// discoverObjectInstance: called by RTI when a new aircraft appears
// ------------------------------------------------------------

void MonitorFederate::discoverObjectInstance(
    rti1516e::ObjectInstanceHandle theObject,
    rti1516e::ObjectClassHandle theObjectClass,
    std::wstring const &theObjectInstanceName)
    RTI_THROW((rti1516e::FederateInternalError))
{
    // Register the new aircraft in both maps
    _aircraftMap[theObject] = AircraftState{0.0, 0.0, 0.0};
    _aircraftNames[theObject] = theObjectInstanceName;

    std::wcout << L"[Monitor] New aircraft discovered: "
               << theObjectInstanceName
               << L" (total tracked: " << _aircraftMap.size() << L")"
               << std::endl;
}

// ------------------------------------------------------------
// removeObjectInstance: called by RTI when an aircraft leaves
// ------------------------------------------------------------

void MonitorFederate::removeObjectInstance(
    rti1516e::ObjectInstanceHandle theObject,
    rti1516e::VariableLengthData const &theUserSuppliedTag,
    rti1516e::OrderType sentOrder,
    rti1516e::SupplementalRemoveInfo theRemoveInfo)
    RTI_THROW((rti1516e::FederateInternalError))
{
    auto it = _aircraftMap.find(theObject);
    if (it == _aircraftMap.end())
        return;

    std::wcout << L"[Monitor] Aircraft left the federation: "
               << _aircraftNames[theObject]
               << L" (remaining: " << _aircraftMap.size() - 1 << L")"
               << std::endl;

    // Remove the aircraft from both maps
    _aircraftMap.erase(it);
    _aircraftNames.erase(theObject);

    // If no more aircraft are being tracked, stop the monitor
    if (_aircraftMap.empty())
    {
        std::wcout << L"[Monitor] No more aircraft in federation, shutting down." << std::endl;
        _running = false;
    }
}

// ------------------------------------------------------------
// reflectAttributeValues: called by RTI when an aircraft sends an update
// ------------------------------------------------------------

void MonitorFederate::reflectAttributeValues(
    rti1516e::ObjectInstanceHandle theObject,
    rti1516e::AttributeHandleValueMap const &theAttributeValues,
    rti1516e::VariableLengthData const &theUserSuppliedTag,
    rti1516e::OrderType sentOrder,
    rti1516e::TransportationType theType,
    rti1516e::SupplementalReflectInfo theReflectInfo)
    RTI_THROW((rti1516e::FederateInternalError))
{
    // Ignore updates from unknown aircraft
    // (can happen if discoverObjectInstance was not called yet)
    if (_aircraftMap.find(theObject) == _aircraftMap.end())
        return;

    // Get a reference to this specific aircraft state
    AircraftState &state = _aircraftMap[theObject];

    // Deserialize each received attribute value
    for (auto &pair : theAttributeValues)
    {
        rti1516e::HLAfloat64BE value;
        value.decode(pair.second);

        if (pair.first == _latitudeHandle)
            state.latitude = value;
        else if (pair.first == _longitudeHandle)
            state.longitude = value;
        else if (pair.first == _altitudeHandle)
            state.altitude = value;
    }

    // Print the updated position for this specific aircraft
    std::wcout << L"[Monitor] Aircraft update"
               << L" | Instance: " << _aircraftNames[theObject]
               << L" | Lat: " << state.latitude
               << L" | Lon: " << state.longitude
               << L" | Alt: " << state.altitude
               << std::endl;
}

// ------------------------------------------------------------
// shutdown: resign from the federation
// ------------------------------------------------------------

void MonitorFederate::shutdown()
{
    _running = false;

    _rtiAmbassador->resignFederationExecution(rti1516e::NO_ACTION);
    std::wcout << L"[Monitor] Resigned from federation." << std::endl;

    try
    {
        _rtiAmbassador->destroyFederationExecution(_federationName);
        std::wcout << L"[Monitor] Federation destroyed." << std::endl;
    }
    catch (const rti1516e::FederatesCurrentlyJoined &)
    {
        std::wcout << L"[Monitor] Other federates still joined, skipping destroy." << std::endl;
    }
    catch (const rti1516e::FederationExecutionDoesNotExist &)
    {
        // Another federate already destroyed the federation, that is fine
    }
}