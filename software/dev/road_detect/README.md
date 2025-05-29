# Basic Road Detection routine

```mermaid
flowchart LR
  A[start point]
  B[reset variables]
  C[read camera image (bottom-to-top, line-by-line]

  D[select widest continuous lines from scanline (index = 'y')]
  E[get their center-line's position relative to camera]
  F[add diff (divided by 'y' index) to steer value]
  G[increment 'y' index]
  H[at the top of frame?]

  I[driving logic]

  A --> B
  B --> C
  C --> D

  D --> E
  E --> F
  F --> G
  G --> H

  H -- no --> D
  H -- yes --> C

  F --> I
```
