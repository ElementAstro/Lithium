# TODO List for Project Lithium

## High Priority

- [ ] Complete the development of the OATPP server

  - [ ] Complete the definition of HTTP and WS interfaces
  - [ ] Implement all interfaces of q-box
  - [ ] Complete user login verification

- [ ] Complete the internal core of the server
  - [ ] Implement internal message bus
  - [ ] Implement internal task allocation mechanism

## Medium Priority

## Low Priority

## Future Enhancements

- [ ] Add support for additional languages
  - [ ] Localization for Spanish and French
  - [ ] Implement language selection feature

## Completed Tasks

- [x] Setup project structure
- [x] Configure build system with CMake

## Notes

- Ensure all code follows the project's coding standards and guidelines.
- Use the issue tracker for reporting bugs and tracking progress on larger tasks.
- Regularly review and update this TODO list to reflect the current state of the project.

## Daily Schedule

### 2024-08-23

- [ ] Complete INDI server management and device connection under Linux
  - [ ] Get a list of all available drivers
  - [ ] Start and stop the server according to the configuration
  - [ ] Get all connected devices
  - [ ] Connect and easily operate specific devices
  - [ ] Complete the corresponding required server interface

- [ ] Complete PHD2 navigation star interface
  - [ ] Analyze all ASIAIR navigation interfaces and supplement them as needed
  - [ ] Support managing PHD2 processes, including starting, stopping, and retrieving locations
  - [ ] PHD2 module is encapsulated as a module and supports enabling automatic loading
