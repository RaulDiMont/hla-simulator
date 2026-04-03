#ifndef RADAR_FEDERATE_H
#define RADAR_FEDERATE_H

#include <memory>
#include <vector>
#include <map>

#include <RTI/RTIambassador.h>
#include <RTI/RTIambassadorFactory.h>
#include <RTI/NullFederateAmbassador.h>

#include "AircraftFederate.h"

// Data structure representing a radar contact
struct RadarContact {
    double distance;   // Distance from radar to aircraft in km
    double bearing;    // Bearing angle in degrees from north
    bool   isInRange;  // True if aircraft is within radar range
};

// Radar federate: subscribes to Aircraft position and publishes RadarContact data
class RadarFederate : public rti1516e::NullFederateAmbassador {
public:
    RadarFederate();
    ~RadarFederate();

    // Connect to RTI and join the federation
    void initialize(const std::wstring& federationName,
                    const std::vector<std::wstring>& fomModules);

    // Main simulation loop
    void run();

    // Disconnect from the federation
    void shutdown();

protected:
    // Called by the RTI when a new aircraft instance is discovered
    void discoverObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::ObjectClassHandle    theObjectClass,
        std::wstring const&            theObjectInstanceName)
        RTI_THROW((rti1516e::FederateInternalError)) override;

    // Called by the RTI when aircraft attributes are updated
    void reflectAttributeValues(
        rti1516e::ObjectInstanceHandle           theObject,
        rti1516e::AttributeHandleValueMap const& theAttributeValues,
        rti1516e::VariableLengthData const&      theUserSuppliedTag,
        rti1516e::OrderType                      sentOrder,
        rti1516e::TransportationType             theType,
        rti1516e::SupplementalReflectInfo        theReflectInfo)
        RTI_THROW((rti1516e::FederateInternalError)) override;

    // Called by the RTI when an aircraft leaves the federation
    void removeObjectInstance(
        rti1516e::ObjectInstanceHandle      theObject,
        rti1516e::VariableLengthData const& theUserSuppliedTag,
        rti1516e::OrderType                 sentOrder,
        rti1516e::SupplementalRemoveInfo    theRemoveInfo)
        RTI_THROW((rti1516e::FederateInternalError)) override;

private:
    // Subscribe to Aircraft attributes
    void subscribeAircraft();

    // Publish RadarContact object class
    void publishRadarContact();

    // Calculate radar contact data from aircraft position
    RadarContact calculateContact(const AircraftState& state) const;

    // Send radar contact update to the RTI
    void updateRadarContact(rti1516e::ObjectInstanceHandle radarInstance,
                            const RadarContact& contact);

    // RTI ambassador
    std::unique_ptr<rti1516e::RTIambassador> _rtiAmbassador;

    // Aircraft FOM handles
    rti1516e::ObjectClassHandle _aircraftClassHandle;
    rti1516e::AttributeHandle   _latitudeHandle;
    rti1516e::AttributeHandle   _longitudeHandle;
    rti1516e::AttributeHandle   _altitudeHandle;

    // RadarContact FOM handles
    rti1516e::ObjectClassHandle _radarContactClassHandle;
    rti1516e::AttributeHandle   _distanceHandle;
    rti1516e::AttributeHandle   _bearingHandle;
    rti1516e::AttributeHandle   _isInRangeHandle;

    // Map of tracked aircraft: handle -> last known state
    std::map<rti1516e::ObjectInstanceHandle, AircraftState> _aircraftMap;

    // Map of radar contact instances: aircraft handle -> radar instance handle
    // One RadarContact instance is created per tracked aircraft
    std::map<rti1516e::ObjectInstanceHandle, rti1516e::ObjectInstanceHandle> _radarInstanceMap;

    // Radar position (fixed ground station in Madrid)
    double _radarLatitude;
    double _radarLongitude;

    // Maximum radar range in km
    double _radarRange;

    // Federation name
    std::wstring _federationName;

    // Flag to control the main loop
    bool _running;
};

#endif // RADAR_FEDERATE_H