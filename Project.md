# HLA Simulator - Project Notes

## Federate Roles

| | AircraftFederate | MonitorFederate |
|--|-----------------|-----------------|
| Role | Publisher | Subscriber |
| Overridden callbacks | None | `discoverObjectInstance` + `reflectAttributeValues` |
| Key method | `publishAircraft()` | `subscribeAircraft()` |
| Loop | Fixed 20 iterations | Continuous until `_running = false` |