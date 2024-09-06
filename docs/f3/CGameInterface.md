# f3::CGameInterface

[Return to `f3`](/docs/f3.md)

## C++

- [`CGameInterface.hpp`](/src/f3/CGameInterface.hpp)
- [`CGameInterface.cpp`](/src/f3/CGameInterface.cpp)

## References

- [`f3::HeapTracker`](/docs/f3/HeapTracker.md)
- [`f3::GUI::Interface`](/docs/f3/GUI/Interface.md)
- [`f3::CEngineInterface`](/docs/f3/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    f3::HeapTracker --> f3::GUI::Interface
    f3::GUI::Interface --> f3::CEngineInterface
    f3::CEngineInterface --> f3::CGameInterface
```

[Return to `f3`](/docs/f3.md)