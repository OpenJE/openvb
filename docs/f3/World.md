# f3::World

[Return to `f3`](/docs/f3.md)

## C++

- [`World.hpp`](/c++/include/World.hpp)
- [`World.cpp`](/c++/source/World.cpp)

## References

- [`f3::IWorld`](/docs/f3/IWorld.md)
- [`f3::IComponent`](/docs/f3/IComponent.md)
- [`f3::IWorldClient`](/docs/f3/IWorldClient.md)
- [`f3::IWorldServer`](/docs/f3/IWorldServer.md)

## Inheritance

```mermaid
graph TD;
    f3::IWorld --> f3::IWorldClient
    f3::IWorld --> f3::IWorldServer
    f3::IComponent --> f3::World
    f3::IWorldClient --> f3::World
    f3::IWorldServer --> f3::World
```

[Return to `f3`](/docs/f3.md)