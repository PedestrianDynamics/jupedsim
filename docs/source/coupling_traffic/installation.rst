==========================
SUMO-JuPedSim Installation
==========================

For the latest release verion of *SUMO* installer for Windows and macOS are available `here <https://sumo.dlr.de/docs/Installing/index.html>`__.
However, we recommend installing the latest development version (nightly snapshots) of *SUMO* so you can benefit from the most recent developments.
The necessary steps for Windows and macOS are explained below.

Windows Installer
=================

Go to this `website <https://sumo.dlr.de/docs/Downloads.php#nightly_snapshots>`__ and download the installer provided by *Windows installer with all extras*. 
Follow the instructions of the installer – you should be ready to get started with the tutorials.


Build on macOS
===============

**Preparation**

To run *JuPedSim* in *SUMO* Python is required.
Before doing anything first create a virtual environment and activate it:

.. code:: shell
    python3 -m venv sumo_jupedsim_venv
    source sumo_jupedsim_venv/bin/activate

Make sure you have installed the prerequisites and dependencies listed `here <https://sumo.dlr.de/docs/Installing/MacOS_Build.html#the_homebrew_approach>`__.

**Building JupedSim**

Build *JuPedSim* (v 1.2.1) following these `instructions <https://sumo.dlr.de/docs/Installing/Linux_Build.html#how_to_build_jupedsim_and_then_build_sumo_with_jupedsim>`__.

To build and install jupedsim in a local directory type this instead:

.. code:: shell
    cmake -B build -DCMAKE_INSTALL_PREFIX=jupedsim-install .
    cmake --build build
    cmake --install build

Now *JuPedSim* should be available and you can link the library when building *SUMO*.

**Building SUMO**

Download the sources from the `nightly snapshots <https://sumo.dlr.de/docs/Downloads.php#nightly_snapshots>`__.
Go to the *SUMO* repository and install the python requirements:

.. code:: shell
    pip install -r ./tools/requirements.txt

Make sure that ``SUMO_HOME`` is exported otherwise you will get a segmentation fault.
Then you can build *SUMO*

.. code:: shell
    export SUMO_HOME="$PWD"
    cmake -B build .

If you compiled and installed *JuPedSim* locally, say in a directory called ``jupedsim-install`` (on the samelevel as *SUMO*) then you should type instead:

.. code:: shell
    cmake -B build -DJUPEDSIM_CUSTOMDIR=../jupedsim-install/ .
    cmake --build build

Check the terminal output. It provides information about missing libraries and whether *JuPedSim* could be found.

**Starting sumo-gui and netedit**

Run *SUMO* from the ``/bin`` subfolder (``bin/sumo-gui`` and ``bin/netedit`` should be available). 

When starting ``sumo-gui`` under *Help > About* information on *JuPedSim* should be listed. If so, the installation was successful.
