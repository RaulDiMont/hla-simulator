#include <iostream>
#include <JSBSim/FGFDMExec.h>
#include <JSBSim/initialization/FGInitialCondition.h>

int main()
{
    // Create the JSBSim flight dynamics model executor
    JSBSim::FGFDMExec fdm;

    // Set the root path where JSBSim will look for aircraft and engine files
    fdm.SetRootDir(SGPath("../simulation"));
    fdm.SetAircraftPath(SGPath("aircraft"));
    fdm.SetEnginePath(SGPath("engine"));

    // Load the A320 model
    if (!fdm.LoadModel("A320")) {
        std::cerr << "Failed to load A320 model!" << std::endl;
        return 1;
    }

    std::cout << "A320 model loaded successfully." << std::endl;

    // Set initial conditions: Madrid at 10000ft, heading north, 250 knots
    fdm.GetIC()->SetLatitudeDegIC(40.4168);
    fdm.GetIC()->SetLongitudeDegIC(-3.7038);
    fdm.GetIC()->SetAltitudeASLFtIC(10000.0);
    fdm.GetIC()->SetPsiDegIC(0.0);    // Heading north
    fdm.GetIC()->SetVcalibratedKtsIC(250.0);

    // Initialize the simulation
    fdm.RunIC();

    // Set throttle to 80%
    fdm.SetPropertyValue("fcs/throttle-cmd-norm[0]", 0.8);
    fdm.SetPropertyValue("fcs/throttle-cmd-norm[1]", 0.8);

    // Run 5 simulation steps and print position
    for (int i = 0; i < 5; ++i) {
        fdm.Run();

        double lat = fdm.GetPropertyValue("position/lat-geod-deg");
        double lon = fdm.GetPropertyValue("position/long-gc-deg");
        double alt = fdm.GetPropertyValue("position/h-sl-ft");
        double spd = fdm.GetPropertyValue("velocities/vtrue-kts");

        std::cout << "Step " << i + 1
                  << " | Lat: "  << lat
                  << " | Lon: "  << lon
                  << " | Alt: "  << alt << " ft"
                  << " | Spd: "  << spd << " kts"
                  << std::endl;
    }

    return 0;
}