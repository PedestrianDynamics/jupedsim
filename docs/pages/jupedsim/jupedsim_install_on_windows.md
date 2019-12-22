---
title: Install JuPedSim on Windows
tags: [getting_started, troubleshooting]
summary: "Windows executables will be released. This page gives instructions how to compile the code on windows using Visual Studio"
keywords: installation
sidebar: jupedsim_sidebar
folder: jupedsim
permalink: jupedsim_install_on_windows.html
last_updated: Dec 20, 2019
---


## Get the code and create a VS-project

```bash
 git clone https://github.com/JuPedSim/jpscore.git
 cd jpscore
 cmake .
```

## Visual Studio: Preparation

Before you proceed make sure your Visual Studio has some features installed.

In our case we need `NuGet`. So if it's not already installed then click

`Tools --> Get Tools and Features`

![Visual studio installer]({{ site.baseurl }}/images/vs/VSInstaller.png)


This will start Visual Studio Installer and ask to make changes in Visual Studio.
Choose the appropriate packages and proceed.

![Visual Studio NuGet]({{ site.baseurl }}/images/vs/installerNuGet.png)

## Visual Studio: Compilation

With Visual Studio open the created JPScore solution (`JPScore.sln`)
and do the following

1. `Right click on the Solution --> Restore NuGet Packages`

    ![Restore NuGet packages]({{ site.baseurl }}/images/vs/restore.png)

    If Visual Studio offers you to restore the packages automatically, as shown in the following screenshot, click on `Restore` (and be patient ...)

    ![Default restore NuGet packages]({{ site.baseurl }}/images/vs/restore_default.png)
    
2. The project `jpscore` should be marked bold (important for running the code later on).
   If this is not the case, then `right click on jpscore --> Set as startup Project`

    ![Startup project]({{ site.baseurl }}/images/vs/startproject.png)

    After this step the project will appear bold  (**jpscore**).
3. `View --> Other Windows --> Open Package Manager Console`

    ![Package manager]({{ site.baseurl }}/images/vs/manager.png)

    and type the following

   ```bash
    Update-Package -reinstall -ProjectName core
   ```

   and

   ```bash
    Update-Package -reinstall -ProjectName jpscore
    ```

    ![Reinstall NuGet packages]({{ site.baseurl }}/images/vs/nuget.png)

4. `Build --> Build Solution`

    If this step is successful then proceed to the next step and run the code

5. Debug --> Start Without Debugging

       ![Run jpscore]({{ site.baseurl }}/images/vs/run.png)

    This will run jpscore in debug mode.


{% include note.html content="this step although it runs jpscore, the later will complain about a missing inifile" %}

6.  `Debug --> jpscore Properties --> Debugging --> Command Arguments`

    and type the full path to a demo file. For example:
    ```bash
     C:\Users\here full path\demos\scenario_1_corridor\corridor_ini.xml
    ```

    ![Run jpscore with demo file]({{ site.baseurl }}/images/vs/cmdarg.png)

## Result

![Result]({{ site.baseurl }}/images/vs/runjpscore.png)


## Install using package manager vcpkg

{%include warning.html content="This section may be incomplete and needs testing"%}

First install [vcpkg](https://github.com/Microsoft/vcpkg):

```bash
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
```

For `JuPedSim` following C++ libraries are needed:

- Qt5
- VTK
- Boost
- ZLIB

To install these libraries run the following:

```bash
 ./vcpkg.exe install qt5
 ./vcpkg.exe install vtk
 ./vcpkg.exe install boost-system boost-timer boost-test boost-filesystem
                     boost-chrono boost-geometry boost-graph
 ./vcpkg.exe install zlib:x86-windows-static
```

Now in order to compile a JuPedSim package (e.g. jpscore)

```bash
 cd jpscore
 mkdir build
 cd build
 cmake -DCMAKE_TOOLCHAIN_FILE="path\\to\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake" ..
```

If this runs without errors, it means all requirements are fulfilled and you can proceed with the compilation

```bash
 make
```

If you have a compiler (MinGW, or Cygwin).

Alternatively, in case you use Visual Studio you may need to
```bash
 File -> Open -> Project/Solution
```
and select the Visual-Studio solution produced in the prior step by cmake (sln-file).
After a while when VS finishes indexing the project click on

```bash
 Build -> Build Solution.
```

{% include links.html %}
