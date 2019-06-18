#pragma once

namespace hg {

class World;

/** System is a base class for user defined systems. */
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

/** Normal systems are executed every frame. */
class NormalSystem : public System {
public:
    /** Construct normal system in specified world. */
    explicit NormalSystem(World& world) noexcept;
    ~NormalSystem() override;
};

/** Fixed systems are executed every tick. */
class FixedSystem : public System {
public:
    /** Construct fixed system in specified world. */
    explicit FixedSystem(World& world) noexcept;
    ~FixedSystem() override;
};

} // namespace hg
