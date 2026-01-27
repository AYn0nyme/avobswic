# avobswic

A Very Optimized Build "System" Written In C
---

## Why??
be*C*ause (funny right haha)
---
## Usage
Copy the build.c file into your project
Your structure **must** have a `src` and `include` folder and will output to a `bin` folder, creating it if needed

In every file on src, you must set the first line to:
```c
//header:m 
```
when using the `<math.h>` library
It should be like `-lGL` without the `-l` with a trailing whitespace at the end of the line, just one
---
## Roadmap
[x] Be able to compile something
[ ] Add support for `include` folders
[ ] Add static building
[ ] Add more customization per-file
