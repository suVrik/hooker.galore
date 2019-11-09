#pragma once

namespace hg {

class World;

/** `System` is a base class for user defined systems. */
class System {
public:
    /** Construct `System` in specified world. */
    explicit System(World& world);
    virtual ~System() = 0;

    /** Execute system. */
    virtual void update(float elapsed_time) = 0;

protected:
    World& world;
};

/** `NormalSystem` is executed every frame. */
class NormalSystem : public System {
public:
    /** Construct `NormalSystem` in specified world. */
    explicit NormalSystem(World& world);
    ~NormalSystem() override;
};

/** `FixedSystem` is executed every tick. */
class FixedSystem : public System {
public:
    /** Construct `FixedSystem` in specified world. */
    explicit FixedSystem(World& world);
    ~FixedSystem() override;
};

} // namespace hg
