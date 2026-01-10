# Known Issues and Limitations

## Architecture
- **Collision Logic Delegation:** `GameWorld` supports multiple units per cell and does not enforce collision rules. Blocking logic relies entirely on `BlockerComponent` checks within `MoveBehavior` and command handlers. Missing checks in new behaviors could lead to unintended unit stacking.
- **Implicit Targeting Logic:** Target selection currently relies on the presence of `HealthComponent` (see `features::utils::hasHealth`). This means any unit with health is automatically a valid target. Future extensions (Tower, Mine, etc.) likely require an explicit `AttackableComponent` / tags to distinguish "destructible" vs "valid AI target".
- **Event Emission Placement:** Event emission is split between `Behaviors` (attack, move, march-ended) and the orchestration layer (`main.cpp`) for unit death (after cleanup). This is consistent with "dead units disappear before the next turn", but it scatters responsibility for event emission. A future refinement could introduce a dedicated tick layer that owns both state transitions and event emission.
- **Concrete World Dependency in Runner:** The simulation runner in `main.cpp` uses `GameWorld` concrete methods (`addUnit`, `forEachUnit`, `removeDeadUnits`, etc.). `IGameWorld` exists and includes `getUnitById`, but orchestration is not yet fully expressed via interfaces.

## Implementation Details
- **Randomness:** The `Random` class uses a global generator (`static std::mt19937`). For deterministic replayability or better testing, this should be replaced with an injected RNG service (e.g., passed via simulation context/world/services) with an explicit seed.
- **Movement Can Get Stuck:** The current marching logic does not attempt to route around blocking units. If the "next step" toward the target is occupied, the unit stops and may remain stuck forever. This is an intentional simplification right now, but it should be addressed (e.g., by trying alternative neighboring steps, or by introducing a simple pathing/avoidance rule).
