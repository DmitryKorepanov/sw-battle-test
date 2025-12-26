# Known Issues

This document lists known limitations and potential improvements that were not implemented due to time constraints or specific requirements of the task (KISS).

## Testing
- **No Automated Tests:** The project currently lacks a unit testing framework (like GTest). Verification relies on `commands_example.txt` and manual inspection.
- **Limited Scenario Coverage:** Only basic scenarios (spawn, march, attack) are verified. Edge cases like map boundaries, full encirclement, or complex collisions are not exhaustively tested.

## AI & Pathfinding
- **Greedy Pathfinding:** Units use a simple greedy algorithm (move directly towards target). They do not avoid obstacles or find optimal paths around blocking units.
- **No Collision Avoidance:** If a target cell is blocked, the unit simply waits. It does not try to step aside or find an alternative route.

## Architecture
- **TypeRegistry Implementation:** The `TypeRegistry` uses `std::shared_ptr<void>` for type erasure. While simple, it incurs atomic reference counting overhead. A `std::unique_ptr`-based solution with custom deleters would be more efficient but more complex to implement.
- **Shared_ptr for Units:** Units are stored as `shared_ptr` in both the turn order list and the spatial grid. This is acceptable for small simulations but may impact cache locality and performance at scale compared to contiguous memory storage (SOA/ECS).
- **Double Position Storage:** Unit position is stored both in the `Unit` object and implicitly in the `GameWorld` grid index. While `GameWorld::moveUnit` ensures consistency, this denormalization is a trade-off for O(1) access speed.

## Features
- **Damage Encapsulation:** Damage logic (`dealDamage`) directly modifies component data (`HealthComponent`). A more encapsulated approach (e.g., event-based damage) was avoided to keep `Core` unaware of `Features` specifics.
