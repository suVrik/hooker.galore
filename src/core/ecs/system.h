#pragma once

namespace hg {

class World;

/** `System` is a base class for user defined systems. */
class System {
public:
    /** Construct system in specified world. */
    explicit System(World& world) noexcept;
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
    explicit NormalSystem(World& world) noexcept;
    ~NormalSystem() override;
};

/** `FixedSystem` is are executed every tick. */
class FixedSystem : public System {
public:
    /** Construct `FixedSystem` in specified world. */
    explicit FixedSystem(World& world) noexcept;
    ~FixedSystem() override;
};

} // namespace hg
