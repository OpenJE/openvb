# f3::VideoInterface

[Return to `f3`](/docs/f3.md)

## C++

- [`VideoInterface.hpp`](/src/f3/VideoInterface.hpp)
- [`VideoInterface.cpp`](/src/f3/VideoInterface.cpp)

## References

- [`f3::HeapTracker`](/docs/f3/HeapTracker.md)
- [`f3::GUI::Interface`](/docs/f3/GUI/Interface.md)
- [`f3::CEngineInterface`](/docs/f3/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    f3::HeapTracker --> f3::GUI::Interface
    f3::GUI::Interface --> f3::CEngineInterface
    f3::CEngineInterface --> f3::VideoInterface
```

[Return to `f3`](/docs/f3.md)