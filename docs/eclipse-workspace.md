# Notes on creating the tlsr8258 Eclipse Workspace

## Preparation
- Install the [Telink IDE] and [Telink Zigbee SDK]
  - Try to install these in the same locations as you will use for your [GitHub] CI process.  They don't have to be the same but it helps if they are.
- From the `Control Panel`, `System`, `Advanced` settings, create two environment variables that indicate the roo directory where you installed your [Telink IDE] and [Telink Zigbee SDK] as per examples below:

|Variable|Purpose|Example|
|-|-|-|
|TELINK_IDE|Root for the IDE|c:\telink_zigbee_sdk\tl_zigbee_sdk|
|TELINK_ZIGBEE_SDK|Root of the Zigbee SDK|c:\telink_ide|

## Installing Eclipse Helios Git Support
- Instead of trying to archive ... install [Eclipse EGit]
- `Window`, `Preferences`, search for `Git`,
- Set HOME environment variable, HOMEDRIVE,, HOMEPATH
- Right click tlsr_tc32 project, Team, Export, `Share Project...`, Git
- Right-click `tlsr_tc32`` project, `Export...`, `Team`, `Team project set`


## Create Empty Workspace and Project
- Replicate the directory structure used by the standard [Telink IDE] workspace but...
- Only copy the directories where the `workspace` and `project` live and the source directories for the application that you will copy so..

|Directory|Purpose|
|-|-|
|tlsr8258|(New!) The root for our build environment|
|tlsr8258\tlsr_tc32|The directory where the `workspace` will be created|
|tlsr8258\tlsr_tc32\build|The directory where the `project` will be created|

> Do not create anything else at this time.

- Start the [Telink IDE] and create an empty workspace
  - At this point you can open the existing [Telink Zigbee SDK] workspace, or try creating yours immediately by providing the full path to your workspace directory e.g. `c:\git\<myproject>\tlsr8258\tlsr_tc32`
  - If you opened the [Telink Zigbee SDK], noy use the `File`, `Switch workspace` menu to create a new workspace, entering the directory `c:\git\<myproject>\tlsr8258\tlsr_tc32`
- Now create an empty project using the `File`, `New...` menu item
  - The project must be a C (not C++) project and select the ???? tc32 application ???? option.

## Importing Some Source Code
> Note that we only want to import a starting application here.  **DO NOT** import everything yet.

- Right-click on the project (`tlsr_tc32`) and select `Import`
- Select `General`, `File system` and then enter the directory of the `apps` directory in the installed [Telink Zigbee SDK]
- You should see "`[ ] Apps`" as and option and you can expand this and **only select** the sample application that you will base your project on e.g. `sampleLight_8258`

> If you have done this properly, yuo will be able to expand the `tlsr_tc32` project and see just the `apps/sampleLight_8258` directory and the **source code** below it.  If you see more, you did it wrongly - just sleect and delete `apps` and do it again.

## Linking Required Build Source
> We are going to *import as links* those parts of the [Telink Zigbee SDK] that we need to build our application but we are **not** going to actually copy it.  The linked code will **not** appear as part of our Git repo.
>
> We will be using the environment variables created earlier to ensure that both locally, and in the [GitHub] CI, we can *find* this code and build our application.

- By looking at the [Telink Zigbee SDK] workspace and the resources in the project there, identify which parts of the [Telink Zigbee SDK] you need to link to your application's project
- Use the same `Import` mechanism as before, select the resources but use the `Advanced` button to select links only, including creating the directory structure.

> The imported resources should now appear in the project but should **NOT** appear in your Git repo.

## Duplicating Build Parameters
Put simply, open your workspace and the [Telink Zigbee SDK] workspaces side-by-side and duplicate all the settings from the [Telink Zigbee SDK] workspace into your workspace.  But here are the interesting bits ;-)

> When you have to define something that is in the [Telink Zigbee SDK] (include paths, paths to pre and post build tools etc), enter them using:
>
> `"${env:TELINK_ZIGBEE_SDK}/<rest-of-the-path>"`
>
> This means that when you use your workspace in the [GitHub] CI, providing you have first set the environment variables via your CI pipeline, then the build will find the tools and files it needs and still work!

## Things That Can Go wrong
There will be other ways to break the build - it appears to be quite tricky to set-up correctly at first.

### Wrong/missing definitions
If you fail to define the MCU type, you can end up building for the wrong target processor and then your build either fails or you get an image that won't work.  Check the workspace set up because this is something set in the workspace settings.

### Missing Files
Means that either:
- You are missing some path definitions in the workspace or
- You set the environment variables pointing to the [Telink IDE] and/or [Telink Zigbee SDK] incorrectly.

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
[Eclipse EGit]: https://archive.eclipse.org/egit/updates-1.0/