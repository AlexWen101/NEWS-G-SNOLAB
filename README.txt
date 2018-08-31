# Shield Simulation

################################################################################################

## Introduction

This code simulates the NEWS-G detector using Geant4. Currently, the following
parts of the detector are included:

* Thin copper sphere containing neon at 2 bar.
* Thick spherical lead shield around the copper sphere.
* Octagonal HDPE shield around the lead shield.
* Glovebox to the side with stainless steel, HDPE, and lead around it.
* Tube that passes through all layers of shielding from the edge of the copper
  sphere to the interior of the glovebox.
    - the tube configuration in this simulation is outdated. There is new design, with
    the tube entering the top of HDPE shielding. 

The simulation can be run multithreaded, and with/without a UI. 

################################################################################################

## Building

To build the project, run the following CMake commands in a new build folder:

    $ mkdir build
    $ cd build
    $ cmake -DSHIELD_VIS_ENABLE=ON ..
    $ make

The `SHIELD_VIS_ENABLE` option determines whether the visualization components
of the simulation will be included. For best performance this should be -DSHIELD_VIS_ENABLE=OFF, 
but for verifying the simulation geometry it can be useful to have visualization.

################################################################################################

## Configuring and running

Before running the simulation, it should be configured by modifying the
`build/configure_commands.mac` file. This file contains some lines to manipulate
the simulation parameters. 

NOTE that if visualization is OFF, configure_commands.mac and run_commands.mac will be executed. 
          if visualization is ON, configure_commands.mac and vis_commands.mac will be executed. 

In `build/configure_commands.mac`, there the following options: 

    Number of threads determines, well, the number of threads...
        for example, you can run on Sphere with 8 threads.

    Run type determines what will be simulated, and with what energies
        # Run Type Options: 
        # ambeneutron 
        # ddneutron
        # ambecapturegamma (4.4 MeV)
        # ambegamma (spectrum <1 MeV)
        # na22gamma 

    Source location is self-explanatory
        # Source Location Options (all without glovebox except the default position)
        # origin
        # location1
        # location2
        # location3
        # location4 (no glovebox)
        # location5 (at the end of the source deployment tube)
        # if nothing is specificed, it is location4 WITH the glovebox

    Cuts refer to geant4 production cuts. They determine how "detailed" the simulation is and 
    will drastically affect performance/number of steps printed.
        # Cuts: 
        # by default is 0.7 mm and 990 eV

    All the parameters can be changed, for example:
        /run/numberOfThreads 8
        /control/alias runType ambecapturegamma 
        /control/alias runName Run
        /control/alias runLocation location5
        /run/setCut 0.000001 mm 
        /cuts/setLowEdge 100 eV 

In 'build/run_commands.mac', there is the important command /run/beamOn which determines the 
number of particles simulated. 

    /run/beamOn 500000

After configuring, the simulation can be run by the command (in the build folder):

    $ ./ShieldSimulation

################################################################################################

## Simulation output
The simulations writes all of the results to a `csv` file. Every time any
particle enters the neon gas detector, it's position, momentum, energy, and many
other properties are recorded in the `csv` file. Each `csv` file is uniquely
indexed by the date & time it was created, the type of the run, the location of the run. 

################################################################################################

## Notes 

If there are any errors in running, and especially in visualization, make sure the following variables
are set while you build geant4: 

    GEANT4_BUILD_MULTITHREADED:BOOL=ON
    GEANT4_INSTALL_DATA:BOOL=ON
    GEANT4_USE_GDML:BOOL=ON
    GEANT4_USE_QT:BOOL=ON
    DGEANT4_USE_OPENGL_X11=ON
    DGEANT4_USE_RAYTRACER_X11=ON

Also, make sure your -DGeant4_DIR variable is set to the following G4 folder: 

    $PATHTO$/Geant4/geant4.10.04.p01-install/lib/Geant4-10.4.1
