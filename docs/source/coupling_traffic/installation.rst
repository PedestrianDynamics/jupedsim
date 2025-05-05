==========================
SUMO-JuPedSim Installation
==========================

For the latest **release version** of *SUMO* installer for Windows and macOS are available `here <https://sumo.dlr.de/docs/Installing/index.html>`__.

.. figure:: /_static/coupling/install/Installer.png
    :width: 80%
    :align: center
    :alt: Installer for *SUMO-JuPedSim*

    The installer helps you to get the latest version of SUMO running on your computer quite quickly. This may already be sufficient for a first simulation with *SUMO-JuPedSim*.

However, if you want to use the latest developments to benefit from recent changes and enhancements, we recommend installing the nightly snapshots of *SUMO*. 
The following explains how to install this **development version** on Windows and macOS.

Installing Development Version on Windows
=========================================

Go to this `website <https://sumo.dlr.de/docs/Downloads.php#nightly_snapshots>`__ and download the installer provided by *Windows installer with all extras*. 
Follow the instructions of the installer – you should be ready to get started with the tutorials.

.. figure:: /_static/coupling/install/Installer_nightly.png
    :width: 100%
    :align: center
    :alt: Installer for the development version

    An installer for the developement version is also available for Windows.



Installing Development Version on macOS
=======================================

For macOS there is no installer available for the nightly snapshots. 
You will therefore need to download the source code and build it yourself. 
To do this, perform the following steps.

Preparation
^^^^^^^^^^^

To run *JuPedSim* in *SUMO* Python is required.
Before doing anything first create a virtual environment and activate it:

.. code:: console

    python3 -m venv sumo_jupedsim_venv
    source sumo_jupedsim_venv/bin/activate

Make sure you have installed the prerequisites and dependencies listed `here <https://sumo.dlr.de/docs/Installing/MacOS_Build.html#the_homebrew_approach>`__.

Building JupedSim
^^^^^^^^^^^^^^^^^

Build *JuPedSim* (v 1.2.1) following these `instructions <https://sumo.dlr.de/docs/Installing/Linux_Build.html#how_to_build_jupedsim_and_then_build_sumo_with_jupedsim>`__.

To build and install jupedsim in a local directory type this instead:

.. code:: console

    cmake -B build -DCMAKE_INSTALL_PREFIX=jupedsim-install .
    cmake --build build
    cmake --install build

Now *JuPedSim* should be available and you can link the library when building *SUMO*.

Building SUMO
^^^^^^^^^^^^^

Download the sources from the `nightly snapshots <https://sumo.dlr.de/docs/Downloads.php#nightly_snapshots>`__.
Go to the *SUMO* repository and install the python requirements:

.. code:: console

    pip install -r ./tools/requirements.txt

Make sure that ``SUMO_HOME`` is exported otherwise you will get a segmentation fault.
Then you can build *SUMO*

.. code:: console

    export SUMO_HOME="$PWD"
    cmake -B build .

If you compiled and installed *JuPedSim* locally, say in a directory called ``jupedsim-install`` (on the samelevel as *SUMO*) then you should type instead:

.. code:: console

    cmake -B build -DJUPEDSIM_CUSTOMDIR=../jupedsim-install/ .
    cmake --build build

Check the terminal output. It provides information about missing libraries and whether *JuPedSim* could be found.

Starting sumo-gui and netedit
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Run *SUMO* from the ``/bin`` subfolder (``/bin/sumo-gui`` and ``/bin/netedit`` should be available). 

When starting ``sumo-gui`` under *Help > About* information on *JuPedSim* should be listed. If so, the installation was successful.

.. figure:: /_static/coupling/install/Netedit.png
    :width: 100%
    :align: center
    :alt: Netedit with information about *JuPedSim*

    Netedit lists *JuPedSim* in the information. *SUMO-JuPedSim* has been successfully installed.
