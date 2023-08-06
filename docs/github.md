# GitHub Workflow Builds

## Overview
### TeLink tlsr825x Builds
- Run on Windows (so no Docker container can be used :-( )
- Based on an old version of the [Eclipse] IDE platform
- Uses the `eclipsec`` command-line build tool
- Requires an [Eclipse] workspace for the project.

## Espressif ESP32 Builds
- Run on Linux so a [Ubuntu] [Docker] container can be used
  - Not clear how GitHub caches [Docker] containers but that's not a problem for now.
- ??? how create a project ???

## Current Builds
- Just an example build that:
  - Installs the Windows [TeLink] IDE and test builds the `sampleGW` project
  - Creates a [Ubuntu] [Docker] container with the [Espressif] IDE installed **but does not perform any builds**.

## Random Notes
### GitHub Workflow (Build) Triggers
- Pull request
- Push, optionally filtered by
- Of a branch
- Of tags
- Path(s) of files pushed
- Schedule; good for sanity checks.

### IDE Storage
- GitHub does not allow files >100MiB so the IDEs are too large.
- Let's not worry about this until the files are removed!

[Eclipse]: https://eclipseide.org/
[Ubuntu]: https://ubuntu.com/
[Docker]: https://www.docker.com/
]Telink]: http://wiki.telink-semi.cn/wiki/IDE-and-Tools/IDE-for-TLSR8-Chips/
[Espressif]: https://idf.espressif.com/