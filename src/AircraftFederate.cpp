#include "AircraftFederate.h"

#include <memory>
#include <RTI/encoding/BasicDataElements.h>
#include <iostream>
#include <thread>
#include <chrono>

// ------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------

AircraftFederate::AircraftFederate()
{
    // Initial state will be set by JSBSim
    _state.latitude = 0.0;
    _state.longitude = 0.0;
    _state.altitude = 0.0;
}

AircraftFederate::~AircraftFederate()
{
}

// ------------------------------------------------------------
// initializeJSBSim: load A320 model and set initial conditions
// ------------------------------------------------------------

void AircraftFederate::initializeJSBSim()
{
    _fdm = std::make_unique<JSBSim::FGFDMExec>();

    // Set paths to aircraft and engine model files
    _fdm->SetRootDir(SGPath("../simulation"));
    _fdm->SetAircraftPath(SGPath("aircraft"));
    _fdm->SetEnginePath(SGPath("engine"));

    // Load the A320 flight dynamics model
    if (!_fdm->LoadModel("A320"))
    {
        throw std::runtime_error("Failed to load A320 model");
    }

    std::wcout << L"[Aircraft] A320 model loaded successfully." << std::endl;

    // Set initial conditions for A320 final approach to Madrid-Barajas (LEMD)
    // Start ~65km north of radar, so aircraft enters 60km range during simulation
    _fdm->GetIC()->SetLatitudeDegIC(40.85);
_fdm->GetIC()->SetLongitudeDegIC(-3.5672);
    _fdm->GetIC()->SetAltitudeASLFtIC(6000.0); // 5000ft approach altitude
    _fdm->GetIC()->SetPsiDegIC(180.0);         // Heading south towards Madrid
    _fdm->GetIC()->SetVcalibratedKtsIC(200.0); // Typical A320 approach speed

    // Configure aircraft for approach
    _fdm->SetPropertyValue("gear/gear-cmd-norm", 0.0); // Gear up
    _fdm->SetPropertyValue("fcs/flap-cmd-norm", 0.25); // Flaps position 1

    // Set JSBSim timestep to match simulation frequency
    _fdm->Setdt(1.0 / SIMULATION_HZ);

    // Apply initial conditions
    _fdm->RunIC();

    // Trim for approach configuration
    _fdm->SetPropertyValue("simulation/do_simple_trim", 1);

    // Run one step so JSBSim calculates aerodynamic forces
    _fdm->Run();

    // Log trim results
    double throttle0 = _fdm->GetPropertyValue("fcs/throttle-cmd-norm[0]");
    double throttle1 = _fdm->GetPropertyValue("fcs/throttle-cmd-norm[1]");
    double elevator = _fdm->GetPropertyValue("fcs/elevator-cmd-norm");
    double pitch = _fdm->GetPropertyValue("attitude/pitch-rad");
    double vspeed = _fdm->GetPropertyValue("velocities/h-dot-fps");
    double lift = _fdm->GetPropertyValue("forces/flift-lbs");
    double weight = _fdm->GetPropertyValue("inertia/weight-lbs");

    std::wcout << L"[Aircraft] Trim results:" << std::endl;
    std::wcout << L"[Aircraft]   Throttle[0]:    " << throttle0 << std::endl;
    std::wcout << L"[Aircraft]   Throttle[1]:    " << throttle1 << std::endl;
    std::wcout << L"[Aircraft]   Elevator:       " << elevator << std::endl;
    std::wcout << L"[Aircraft]   Pitch:          " << pitch * 180.0 / 3.14159 << L" deg" << std::endl;
    std::wcout << L"[Aircraft]   Vertical speed: " << vspeed << L" ft/s" << std::endl;
    std::wcout << L"[Aircraft]   Lift:           " << lift << L" lbs" << std::endl;
    std::wcout << L"[Aircraft]   Weight:         " << weight << L" lbs" << std::endl;

    std::wcout << L"[Aircraft] Initial conditions set." << std::endl;
}
// ------------------------------------------------------------
// initialize: connect to RTI, create and join the federation
// ------------------------------------------------------------

void AircraftFederate::initialize(const std::wstring &federationName,
                                  const std::vector<std::wstring> &fomModules)
{
    _federationName = federationName;

    // Initialize JSBSim first
    initializeJSBSim();

    // Create the RTI ambassador
    rti1516e::RTIambassadorFactory factory;
    _rtiAmbassador = factory.createRTIambassador();

    // Connect to the RTI using thread mode
    _rtiAmbassador->connect(*this, rti1516e::HLA_EVOKED, L"rti://localhost");

    // Create the federation execution using all FOM modules
    try
    {
        _rtiAmbassador->createFederationExecution(federationName, fomModules);
        std::wcout << L"[Aircraft] Federation created." << std::endl;
    }
    catch (const rti1516e::FederationExecutionAlreadyExists &)
    {
        std::wcout << L"[Aircraft] Federation already exists, joining." << std::endl;
    }

    // Join the federation as "AircraftFederate"
    _rtiAmbassador->joinFederationExecution(L"AircraftFederate", federationName);
    std::wcout << L"[Aircraft] Joined federation." << std::endl;

    // Resolve FOM handles for Aircraft class and its attributes
    _aircraftClassHandle = _rtiAmbassador->getObjectClassHandle(L"HLAobjectRoot.Aircraft");
    _latitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Latitude");
    _longitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Longitude");
    _altitudeHandle = _rtiAmbassador->getAttributeHandle(_aircraftClassHandle, L"Altitude");

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
    rti1516e::AttributeHandleSet attributes;
    attributes.insert(_latitudeHandle);
    attributes.insert(_longitudeHandle);
    attributes.insert(_altitudeHandle);

    _rtiAmbassador->publishObjectClassAttributes(_aircraftClassHandle, attributes);
    std::wcout << L"[Aircraft] Publishing Aircraft attributes." << std::endl;
}

// ------------------------------------------------------------
// calculatePosition: run one JSBSim timestep and read position
// ------------------------------------------------------------

void AircraftFederate::calculatePosition()
{
    // Run one JSBSim simulation step
    _fdm->Run();

    // NOTE: The A320 BETA model does not maintain level flight due to
    // unvalidated aerodynamic coefficients. Altitude is fixed at 10000ft
    // for simulation purposes. A fully validated model would not require this.
    // _fdm->SetPropertyValue("position/h-sl-ft", 10000.0);

    // Read back position from JSBSim
    _state.latitude = _fdm->GetPropertyValue("position/lat-geod-deg");
    _state.longitude = _fdm->GetPropertyValue("position/long-gc-deg");
    _state.altitude = _fdm->GetPropertyValue("position/h-sl-ft");
}

// ------------------------------------------------------------
// run: main simulation loop
// ------------------------------------------------------------

void AircraftFederate::run()
{
    std::wcout << L"[Aircraft] Starting simulation loop." << std::endl;

    // We publish to RTI once per second

    for (int update = 0; update < 30; ++update)
    {
        // Run JSBSim for one second worth of simulation
        for (int step = 0; step < SIMULATION_HZ; ++step)
        {
            calculatePosition();
            // Sleep exactly one JSBSim timestep
            std::this_thread::sleep_for(
                std::chrono::microseconds(1000000 / SIMULATION_HZ));
        }

        // Publish updated position to the RTI
        updatePosition();

        std::wcout << L"[Aircraft] Update " << update + 1
                   << L" | Lat: " << _state.latitude
                   << L" | Lon: " << _state.longitude
                   << L" | Alt: " << _state.altitude << L" ft"
                   << std::endl;
    }
}

// ------------------------------------------------------------
// updatePosition: serialize and send position to the RTI
// ------------------------------------------------------------

void AircraftFederate::updatePosition()
{
    rti1516e::AttributeHandleValueMap attributes;
    attributes[_latitudeHandle] = rti1516e::HLAfloat64BE(_state.latitude).encode();
    attributes[_longitudeHandle] = rti1516e::HLAfloat64BE(_state.longitude).encode();
    attributes[_altitudeHandle] = rti1516e::HLAfloat64BE(_state.altitude).encode();

    rti1516e::VariableLengthData tag;
    _rtiAmbassador->updateAttributeValues(_aircraftInstance, attributes, tag);
}

// ------------------------------------------------------------
// shutdown: resign and destroy the federation
// ------------------------------------------------------------

void AircraftFederate::shutdown()
{
    _rtiAmbassador->resignFederationExecution(rti1516e::NO_ACTION);
    std::wcout << L"[Aircraft] Resigned from federation." << std::endl;

    try
    {
        _rtiAmbassador->destroyFederationExecution(_federationName);
        std::wcout << L"[Aircraft] Federation destroyed." << std::endl;
    }
    catch (const rti1516e::FederatesCurrentlyJoined &)
    {
        std::wcout << L"[Aircraft] Other federates still joined, skipping destroy." << std::endl;
    }
    catch (const rti1516e::FederationExecutionDoesNotExist &)
    {
        // Another federate already destroyed the federation, that is fine
    }
}