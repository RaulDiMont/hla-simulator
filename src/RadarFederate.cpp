#include "RadarFederate.h"

#include <memory>
#include <RTI/encoding/BasicDataElements.h>
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>

// Earth radius in km
static const double EARTH_RADIUS = 6371.0;
static const double PI = 3.14159265358979323846;

// ------------------------------------------------------------
// Helper: convert degrees to radians
// ------------------------------------------------------------

static double toRadians(double degrees)
{
    return degrees * PI / 180.0;
}

// ------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------

RadarFederate::RadarFederate() :
    _radarLatitude(40.4939),   // Madrid-Barajas airport (LEMD) - ICAO verified
_radarLongitude(-3.5672),
    _radarRange(60.0),         // 60km range — covers the approach corridor
    _running(false)
{
}

RadarFederate::~RadarFederate()
{
}

// ------------------------------------------------------------
// initialize: connect to RTI and join the federation
// ------------------------------------------------------------

void RadarFederate::initialize(const std::wstring &federationName,
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
        std::wcout << L"[Radar] Federation created." << std::endl;
    }
    catch (const rti1516e::FederationExecutionAlreadyExists &)
    {
        std::wcout << L"[Radar] Federation already exists, joining." << std::endl;
    }

    // Join the federation as "RadarFederate"
    _rtiAmbassador->joinFederationExecution(L"RadarFederate", federationName);
    std::wcout << L"[Radar] Joined federation." << std::endl;

    // Resolve Aircraft FOM handles
    _aircraftClassHandle = _rtiAmbassador->getObjectClassHandle(L"HLAobjectRoot.Aircraft");
    _latitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Latitude");
    _longitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Longitude");
    _altitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Altitude");

    // Resolve RadarContact FOM handles
    _radarContactClassHandle = _rtiAmbassador->getObjectClassHandle(L"HLAobjectRoot.RadarContact");
    _distanceHandle = _rtiAmbassador->getAttributeHandle(_radarContactClassHandle, L"Distance");
    _bearingHandle = _rtiAmbassador->getAttributeHandle(_radarContactClassHandle, L"Bearing");
    _isInRangeHandle = _rtiAmbassador->getAttributeHandle(_radarContactClassHandle, L"IsInRange");

    // Subscribe to Aircraft and publish RadarContact
    subscribeAircraft();
    publishRadarContact();
}

// ------------------------------------------------------------
// subscribeAircraft: declare to RTI that we want Aircraft updates
// ------------------------------------------------------------

void RadarFederate::subscribeAircraft()
{
    rti1516e::AttributeHandleSet attributes;
    attributes.insert(_latitudeHandle);
    attributes.insert(_longitudeHandle);
    attributes.insert(_altitudeHandle);

    _rtiAmbassador->subscribeObjectClassAttributes(_aircraftClassHandle, attributes);
    std::wcout << L"[Radar] Subscribed to Aircraft attributes." << std::endl;
}

// ------------------------------------------------------------
// publishRadarContact: declare to RTI that we will publish RadarContact
// ------------------------------------------------------------

void RadarFederate::publishRadarContact()
{
    rti1516e::AttributeHandleSet attributes;
    attributes.insert(_distanceHandle);
    attributes.insert(_bearingHandle);
    attributes.insert(_isInRangeHandle);

    _rtiAmbassador->publishObjectClassAttributes(_radarContactClassHandle, attributes);
    std::wcout << L"[Radar] Publishing RadarContact attributes." << std::endl;
}

// ------------------------------------------------------------
// calculateContact: compute radar data from aircraft position
// Uses Haversine formula for distance and spherical trigonometry for bearing
// ------------------------------------------------------------

RadarContact RadarFederate::calculateContact(const AircraftState &state) const
{
    RadarContact contact;

    double lat1 = toRadians(_radarLatitude);
    double lat2 = toRadians(state.latitude);
    double dLat = toRadians(state.latitude - _radarLatitude);
    double dLon = toRadians(state.longitude - _radarLongitude);

    // Haversine formula for distance
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1) * std::cos(lat2) *
                   std::sin(dLon / 2) * std::sin(dLon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    contact.distance = EARTH_RADIUS * c;

    // Spherical trigonometry formula for bearing
    double y = std::sin(toRadians(state.longitude - _radarLongitude)) * std::cos(lat2);
    double x = std::cos(lat1) * std::sin(lat2) -
               std::sin(lat1) * std::cos(lat2) *
                   std::cos(toRadians(state.longitude - _radarLongitude));
    double bearing = std::atan2(y, x) * 180.0 / PI;

    // Normalize bearing to 0-360 degrees
    contact.bearing = std::fmod(bearing + 360.0, 360.0);

    // Check if aircraft is within radar range
    contact.isInRange = contact.distance <= _radarRange;

    return contact;
}

// ------------------------------------------------------------
// updateRadarContact: serialize and send radar contact to the RTI
// ------------------------------------------------------------

void RadarFederate::updateRadarContact(rti1516e::ObjectInstanceHandle radarInstance,
                                       const RadarContact &contact)
{
    rti1516e::AttributeHandleValueMap attributes;
    attributes[_distanceHandle] = rti1516e::HLAfloat64BE(contact.distance).encode();
    attributes[_bearingHandle] = rti1516e::HLAfloat64BE(contact.bearing).encode();
    attributes[_isInRangeHandle] = rti1516e::HLAboolean(contact.isInRange).encode();

    rti1516e::VariableLengthData tag;
    _rtiAmbassador->updateAttributeValues(radarInstance, attributes, tag);
}

// ------------------------------------------------------------
// run: main simulation loop
// ------------------------------------------------------------

void RadarFederate::run()
{
    std::wcout << L"[Radar] Waiting for aircraft..." << std::endl;

    _running = true;
    while (_running)
    {
        // Process incoming RTI callbacks
        _rtiAmbassador->evokeMultipleCallbacks(0.1, 1.0);
    }
}

// ------------------------------------------------------------
// discoverObjectInstance: called by RTI when a new aircraft appears
// ------------------------------------------------------------

void RadarFederate::discoverObjectInstance(
    rti1516e::ObjectInstanceHandle theObject,
    rti1516e::ObjectClassHandle theObjectClass,
    std::wstring const &theObjectInstanceName)
    RTI_THROW((rti1516e::FederateInternalError))
{
    // Register the new aircraft with a default state
    _aircraftMap[theObject] = AircraftState{0.0, 0.0, 0.0};

    // Create a RadarContact instance for this aircraft
    rti1516e::ObjectInstanceHandle radarInstance =
        _rtiAmbassador->registerObjectInstance(_radarContactClassHandle);
    _radarInstanceMap[theObject] = radarInstance;

    std::wcout << L"[Radar] New aircraft detected: " << theObjectInstanceName
               << L" — RadarContact created." << std::endl;
}

// ------------------------------------------------------------
// reflectAttributeValues: called by RTI when aircraft position updates
// ------------------------------------------------------------

void RadarFederate::reflectAttributeValues(
    rti1516e::ObjectInstanceHandle theObject,
    rti1516e::AttributeHandleValueMap const &theAttributeValues,
    rti1516e::VariableLengthData const &theUserSuppliedTag,
    rti1516e::OrderType sentOrder,
    rti1516e::TransportationType theType,
    rti1516e::SupplementalReflectInfo theReflectInfo)
    RTI_THROW((rti1516e::FederateInternalError))
{
    if (_aircraftMap.find(theObject) == _aircraftMap.end())
        return;

    // Update the aircraft state
    AircraftState &state = _aircraftMap[theObject];
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

    // Calculate and publish radar contact data
    RadarContact contact = calculateContact(state);
    updateRadarContact(_radarInstanceMap[theObject], contact);

    std::wcout << L"[Radar] Contact update"
               << L" | Distance: " << contact.distance << L" km"
               << L" | Bearing: " << contact.bearing << L" deg"
               << L" | InRange: " << (contact.isInRange ? L"YES" : L"NO")
               << std::endl;
}

// ------------------------------------------------------------
// removeObjectInstance: called by RTI when an aircraft leaves
// ------------------------------------------------------------

void RadarFederate::removeObjectInstance(
    rti1516e::ObjectInstanceHandle theObject,
    rti1516e::VariableLengthData const &theUserSuppliedTag,
    rti1516e::OrderType sentOrder,
    rti1516e::SupplementalRemoveInfo theRemoveInfo)
    RTI_THROW((rti1516e::FederateInternalError))
{
    auto it = _aircraftMap.find(theObject);
    if (it == _aircraftMap.end())
        return;

    std::wcout << L"[Radar] Aircraft lost. Removing RadarContact." << std::endl;

    // Delete the RadarContact instance from the federation
    rti1516e::VariableLengthData tag;
    _rtiAmbassador->deleteObjectInstance(_radarInstanceMap[theObject], tag);

    // Remove from both maps
    _aircraftMap.erase(it);
    _radarInstanceMap.erase(theObject);

    // Stop if no more aircraft are being tracked
    if (_aircraftMap.empty())
    {
        std::wcout << L"[Radar] No more aircraft. Shutting down." << std::endl;
        _running = false;
    }
}

// ------------------------------------------------------------
// shutdown: resign from the federation
// ------------------------------------------------------------

void RadarFederate::shutdown()
{
    _rtiAmbassador->resignFederationExecution(rti1516e::NO_ACTION);
    std::wcout << L"[Radar] Resigned from federation." << std::endl;

    try
    {
        _rtiAmbassador->destroyFederationExecution(_federationName);
        std::wcout << L"[Radar] Federation destroyed." << std::endl;
    }
    catch (const rti1516e::FederatesCurrentlyJoined &)
    {
        std::wcout << L"[Radar] Other federates still joined, skipping destroy." << std::endl;
    }
    catch (const rti1516e::FederationExecutionDoesNotExist &)
    {
        // Another federate already destroyed it
    }
}