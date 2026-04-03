#include <memory>  // Must be included before any OpenRTI headers
#ifndef MONITOR_FEDERATE_H
#define MONITOR_FEDERATE_H

#include <RTI/RTIambassador.h>
#include <RTI/RTIambassadorFactory.h>
#include <RTI/NullFederateAmbassador.h>

#include <map>

#include "AircraftFederate.h"

// Monitor federate: subscribes to aircraft position and prints it to console
class MonitorFederate : public rti1516e::NullFederateAmbassador
{
public:
    MonitorFederate();
    ~MonitorFederate();

    // Connect to RTI and join the federation
    void initialize(const std::wstring &federationName,
                    const std::wstring &fomPath);

    // Main loop: process incoming callbacks from the RTI
    void run();

    // Disconnect from the federation
    void shutdown();

protected:
    // Called by the RTI when a new object instance is discovered
    void discoverObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::ObjectClassHandle theObjectClass,
        std::wstring const &theObjectInstanceName)
        RTI_THROW((rti1516e::FederateInternalError)) override;

    // Called by the RTI when attribute values are updated by a publisher
    void reflectAttributeValues(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::AttributeHandleValueMap const &theAttributeValues,
        rti1516e::VariableLengthData const &theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::TransportationType theType,
        rti1516e::SupplementalReflectInfo theReflectInfo)
        RTI_THROW((rti1516e::FederateInternalError)) override;

    // Called by the RTI when an aircraft leaves the federation
    void removeObjectInstance(
        rti1516e::ObjectInstanceHandle theObject,
        rti1516e::VariableLengthData const &theUserSuppliedTag,
        rti1516e::OrderType sentOrder,
        rti1516e::SupplementalRemoveInfo theRemoveInfo)
        RTI_THROW((rti1516e::FederateInternalError)) override;

private:
    // Subscribe to Aircraft object class attributes
    void subscribeAircraft();

    // RTI ambassador: our interface to communicate with the RTI
    std::unique_ptr<rti1516e::RTIambassador> _rtiAmbassador;

    // Handles to identify objects and attributes in the FOM
    rti1516e::ObjectClassHandle _aircraftClassHandle;
    rti1516e::AttributeHandle _latitudeHandle;
    rti1516e::AttributeHandle _longitudeHandle;
    rti1516e::AttributeHandle _altitudeHandle;

    // Map of all known aircraft instances and their last known state
    // Key: unique object instance handle assigned by the RTI
    // Value: last received position data for that aircraft
    std::map<rti1516e::ObjectInstanceHandle, AircraftState> _aircraftMap;
    // Map of object instance handles to their human-readable names
    std::map<rti1516e::ObjectInstanceHandle, std::wstring> _aircraftNames;

    // Federation name
    std::wstring _federationName;

    // Flag to control the main loop
    bool _running;
};

#endif // MONITOR_FEDERATE_H