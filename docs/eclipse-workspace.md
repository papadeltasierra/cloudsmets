# Notes on creating the tlsr8258 Eclipse Workspace

## Preparation
- Install the [Telink IDE] and [Telink Zigbee SDK]
  - Try to install these in the same locations as you will use for your [GitHub] CI process.  They don't have to be the same but it helps if they are.
- From the `Control Panel`, `System`, `Advanced` settings, create three environment variables that indicate the root directory where you installed your [Telink IDE] and [Telink Zigbee SDK] as per examples below:

|Variable|Purpose|Example|
|-|-|-|
|TELINK_IDE|Root for the IDE|c:\telink_zigbee_sdk\tl_zigbee_sdk|
|TELINK_ZIGBEE_SDK|Root of the Zigbee SDK|c:\telink_ide|
|GITHUB_WORKSPACE|Root of your Git project<br/>(GitHub will set this in the CI)|c:\git\cloudsmets|

## Installing Eclipse Helios (E)Git Support
> ** DO NOT DO THIS - it doesn't work! **

## Create Empty Workspace and Project
- Replicate the directory structure used by the standard [Telink IDE] workspace but...
- Only copy the directories where the `workspace` and `project` live and the source directories for the application that you will copy so..

|Directory|Purpose|
|-|-|
|tlsr8258|(New!) The root for our build environment|
|tlsr8258\build|The directory where the `workspace` will be created|
|tlsr8258\build\tlsr_tc32|The directory where the `project` will be created|

> Do not create anything else at this time.

- Start the [Telink IDE] and create an empty workspace
  - At this point you can open the existing [Telink Zigbee SDK] workspace, or try creating yours immediately by providing the full path to your workspace directory e.g. `c:\git\<myproject>\tlsr8258\build`
  - If you opened the [Telink Zigbee SDK], now use the `File`, `Switch workspace` menu to create a new workspace, entering the directory `c:\git\<myproject>\tlsr8258\build`
- Now create an empty project using the `File`, `New...` menu item
  - The project must be a `C` (not `C++`) project and select the `TC32 Cross Target Application` option.

## Importing Some Source Code
> Note that we only want to import a starting application here.  **DO NOT** import everything yet.

- Right-click on the project (`tlsr_tc32`) and select `Import`
- Select `General`, `File system` and then enter the directory of the `apps` directory in the installed [Telink Zigbee SDK]
- You should see "`[ ] Apps`" as and option and you can expand this and **only select** the sample application that you will base your project on e.g. `sampleLight_8258`

> If you have done this properly, you will be able to expand the `tlsr_tc32` project and see just the `apps/sampleLight_8258` directory and the **source code** below it.  If you see more, you did it wrongly - just select and delete `apps` and do it again.

## Linking Required Build Source
### Path Variables
[Eclipse] uses `Path Variables` to refer to linked resource.  `Path Variables` **are not** the same as environment variables and you cannot use environment variables to refer to linked resources.  This means that if the linked resources (i.e. the ZigBee SDK) is installed in a different location locally to the [GitHub] CI,one of the build potentially won't work!

However can use the followng tricks to work around this:
- `Path Variables` can be sert at both Workspace and Project scope.  As you will see below, we **do not** code manage the workspace, which is a part of our local build, so any changes we make there **do not** end up in the [Github] CI environment
- `Path Variables` **can** be defined in terms of other variables so...
- Define variable `TELINK_ZIGBEE_SDK_WS` at Workspace scope (via `Windows`, `Preferences`, `General`, `Workspace`, `Linked Resources`)
- Define variable `TELINK_ZIGBEE_SDK` at Project scope (via `Project explorer` tab, `tlsr_tc32`, right-click, `Properties`, `Resource`, `Linked Resources`
  - At this point, the local build will work but in the [GitHub] CI, there is no workspace, `TELINK_ZIGBEE_SDK_WS` is not defined and the build will fail
- Use a Pwershell regex to **patch the `.project` file** during the [GitHub] CI build; the patch does the following:
  - Replaces the references to the `TELINK_ZIGBEE_SDK_WS` variable with the correct path to the [Tlink ZigBee SDK] as installed indie the [GitHub] CI.

We can now *import as links* those parts of the [Telink Zigbee SDK] that we need to build our application but we are **not** going to actually copy it.  The linked code will **not** appear as part of our Git repo.  We import relative to the `TELINK_ZIGBEE_SDK` `Path Variable` which means that paths are corrected in both local and [GitHub] CI build environments.

- By looking at the [Telink Zigbee SDK] workspace and the resources in the project there, identify which parts of the [Telink Zigbee SDK] you need to link to your application's project
- Use the same `Import` mechanism as before, select the resources but use the `Advanced` button to select links only, including creating the directory structure.

> The imported resources should now appear in the project but should **NOT** appear in your Git repo.

## Duplicating Build Parameters
Put simply, open your workspace and the [Telink Zigbee SDK] workspaces side-by-side and duplicate all the settings from the [Telink Zigbee SDK] workspace into your workspace.  But here are the interesting bits ;-)

- When you have to define something that is in the [Telink Zigbee SDK] (include paths, paths to pre and post build tools etc), enter them using:<br/>
  `"${TELINK_ZIGBEE_SDK}/<rest-of-the-path>"`<br/>
  This means that when you use your workspace in the [GitHub] CI, providing you have first set the environment variables via your CI pipeline, then the build will find the tools and files it needs and still work!
- Enter paths to your source code using `"${GITHUB_WORKSPACE}/<rest-of-the-path>"` so that it can be found locally and in the [GitHub] CI pipeline.

## Code-managing the Workspace
Don't!  Just code-manage the project and settings as identified by these files/directories:
- `tlsr8258/build/tlsr_tc32.settings`
- `tlsr8258/build/tlsr_tc32/.project`
- `tlsr8258/build/tlsr_tc32/.cproject`

## Things That Can Go wrong
There will be other ways to break the build - it appears to be quite tricky to set-up correctly at first.

### Wrong/missing definitions
If you fail to define the MCU type, you can end up building for the wrong target processor and then your build either fails or you get an image that won't work.  Check the workspace set up because this is something set in the workspace settings.

### Missing Files
Means that either:
- You are missing some path definitions in the workspace or
- You set the environment variables pointing to your repo (`GITHUB_WORKSPACE`) and/or [Telink Zigbee SDK] (`TELINK_ZIGBEE_SDK`) incorrectly.

### Wrong File Included
- You have all the paths, but perhaps not in the correct order to ensure the build finds the correct instance of required included files or
- You linked in the wrong directories from the [Telink Zigbee SDK].

### Builds Locally, but not in [GitHub] CI
The ability to build both locally and on the [GitHub] CI depends on removing all hard-coded paths and replacing them with environment based alternatives as well as using import/link to bring in the [TeLink ZigBee SDK] components.  Get any of this wrong and the build will fail so track down where these hard-coded paths remain and correct them.

### Assembler (*.S) File Do Not Compile
The assembler files use a lot of relative paths such as `../common/comm_cfg.h` to find header files etc but this will fail in our _perosonal and IDE/ZigBee_ set-up.  The first is to identify the directories where files should be found (`apps/common` in this case) and add the _environment variabled_ version to the TC32 CC/Assembler `Paths`` setting.

### Multiple Definitions Identified During Link
For example
```bash
C:/Users/PDS/git/telink_zigbee_sdk/tl_zigbee_sdk/platform/chip_826x/flash.c:102: multiple definition of `flash_mspi_read_ram'
./platform/chip_8278/flash.o:C:/Users/PDS/git/telink_zigbee_sdk/tl_zigbee_sdk/platform/chip_8278/flash.c:101: first defined here
```
This probably iomplies that we have linked in _the same_ files for multiple target platforms, as you can see from the `chip_8278` in this example.

Delete these links from the project.

### Undefined Reference at Link Time
For example
```bash
C:/Users/PDS/git/telink_zigbee_sdk/tl_zigbee_sdk/zigbee/mac/mac_phy.c:205: undefined reference to `__divsi3'
```
This is typically a library or objects missing, in this case `div_mod.S`.  Import this into the project.

Note that the imported location might be different to that in the [Telink Zigbee SDK] workspace.  I believe this is because if you import _with copy_ instead of link, you can specify where the item(s) will appear in the project.


[Telink IDE]: http://wiki.telink-semi.cn/wiki/IDE-and-Tools/IDE-for-TLSR8-Chips/
[Telink Zigbee SDK]: http://wiki.telink-semi.cn/tools_and_sdk/Zigbee/Zigbee_SDK.zip
[GitHub]: https://github.com
[Eclipse]: https://www.eclipse.org/downloads/packages/release/helios/r
