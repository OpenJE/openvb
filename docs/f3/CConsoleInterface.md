# f3::CConsoleInterface

[Return to `f3`](/docs/f3.md)

## C++

- [`CConsoleInterface.hpp`](/src/f3/CConsoleInterface.hpp)
- [`CConsoleInterface.cpp`](/src/f3/CConsoleInterface.cpp)

## References

- [`f3::HeapTracker`](/docs/f3/HeapTracker.md)
- [`f3::GUI::Interface`](/docs/f3/GUI/Interface.md)
- [`f3::CEngineInterface`](/docs/f3/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    f3::HeapTracker --> f3::GUI::Interface
    f3::GUI::Interface --> f3::CEngineInterface
    f3::CEngineInterface --> f3::CConsoleInterface
```

[Return to `f3`](/docs/f3.md)