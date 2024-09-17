# F3::GameDialogueInterface

[Return to `F3`](/docs/f3.md)

## C++

- [`GameDialogueInterface.hpp`](/src/f3/GameDialogueInterface.hpp)
- [`GameDialogueInterface.cpp`](/src/f3/GameDialogueInterface.cpp)

## References

- [`JE::HeapTracker`](https://github.com/OpenJE/openje/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](https://github.com/OpenJE/openje/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](https://github.com/OpenJE/openje/docs/je/CEngineInterface.md)
- [`JE::DialogueInterface`](https://github.com/OpenJE/openje/docs/je/DialogueInterface.md)
- [`F3::CGameInterface`](/docs/f3/CGameInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::DialogueInterface
    JE::CEngineInterface --> F3::CGameInterface
    JE::DialogueInterface --> F3::GameCSSkillMenuInterface
    F3::CGameInterface --> F3::GameCSSkillMenuInterface
```

[Return to `F3`](/docs/f3.md)