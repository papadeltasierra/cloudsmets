# Component Creation etc.
```mermaid

sequenceDiagram;

    participant m as main;

    create participant cl as cloud;
    m->>cl: Create

    create participant az as Azure;
    cl->>az: Create
    az->>cl: Created
```
