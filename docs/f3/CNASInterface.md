# f3::CNASInterface

[Return to `f3`](/docs/f3.md)

## C++

- [`CNASInterface.hpp`](/c++/include/CNASInterface.hpp)
- [`CNASInterface.cpp`](/c++/source/CNASInterface.cpp)

## References

- [`f3::HeapTracker`](/docs/f3/HeapTracker.md)
- [`f3::GUI::Interface`](/docs/f3/GUI/Interface.md)
- [`f3::CEngineInterface`](/docs/f3/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    f3::HeapTracker --> f3::GUI::Interface
    f3::GUI::Interface --> f3::CEngineInterface
    f3::CEngineInterface --> f3::CNASInterface
```

[Return to `f3`](/docs/f3.md)