Installation
===================================================

Prerequisites
---------------------------------------------------

- CMake >= 3.23
- For the OMPT plugin, a C compiler supporting OpenMP >= 5.0
- `OTF2 <https://doi.org/10.5281/zenodo.1240851>`__ >= 2.3

To build and install OTF2 2.3 from source, run:

::

   wget https://zenodo.org/record/4682684/files/otf2-2.3.tar.gz
   tar -xzvf otf2-2.3.tar.gz && cd otf2-2.3
   ./configure [--prefix=/preferred/install/prefix]
   make
   make install

Alternatively, from spack:

::

    spack install otf2@2.3

If your machine has the `module <https://modules.readthedocs.io/en/latest/>`__ command, you may already
have otf2 available:

::

    module avail otf2

The CMake build system will search for OTF2 in the locations specified by the `OTF2_ROOT` CMake
variable and the `OTF2_ROOT` environment variable, in that order.

Building Otter
---------------------------------------------------

After the pre-requisites are installed, clone the repository:

::

   git clone https://github.com/Otter-Taskification/otter.git
   cd otter/

For a first build, Otter provides a number of presets:

::

    cmake --list-presets

Configure a preset with:

::

    cmake --preset <preset-name>

View the variables associated with the build:

::

    cmake -LA -N <buid-dir>

Then build it with:

::

    cmake --build --preset <preset-name>

Key CMake Options
---------------------------------------------------

This table describes the key options for building Otter:

+--------------------------------------------------------------------------------+---------------------------+---------------------+
| Option                                                                         | Description               |            Default  |
+================================================================================+===========================+=====================+
| ``-DCMAKE_C_COMPILER=[...]``                                                   | C compiler to use         |            System   |
|                                                                                |                           |            default  |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DCMAKE_INSTALL_PREFIX=[...]``                                               | Target installation path  |      ``/usr/local`` |
|                                                                                | for Otter                 |                     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DCMAKE_BUILD_TYPE=[Debug|Release]``                                         | Build type                |     ``Release``     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DOTF2_ROOT=[PATH]``                                                         | Location of OTF2          |                     |
|                                                                                | installation              |                     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DWITH_OMPT_PLUGIN=[ON|OFF]``                                                | Build the OMPT plugin     |            ``OFF``  |
|                                                                                |                           |                     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DWITH_EXAMPLES=[ON|OFF]``                                                   | Build Otter with examples |            ``OFF``  |
|                                                                                |                           |                     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DWITH_TESTS=[ON|OFF]``                                                      | Build tests               |            ``OFF``  |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DBUILD_SHARED_LIBS=[ON|OFF]``                                               | Build shared libraries    |            ``OFF``  |
+--------------------------------------------------------------------------------+---------------------------+---------------------+


Installing Otter
---------------------------------------------------

To install a build:

::

    cmake --install <buid-dir> [--prefix=...]

Choose a custom installation prefix with the ``--prefix`` option. The following
files (among others) will be installed:

-  ``include/otter/otter-task-graph-user.h`` (the main API for
   annotating application code)
-  ``include/otter/otter-task-graph-stub.h`` (for developers of software
   which may use Otter but who don't want to require it as a dependency
   for their users)
-  ``lib/libotter-task-graph.[a|so]``
-  ``lib/libotter-ompt.so`` (if requested)
-  ``lib/cmake/Otter/OtterConfig.cmake`` to enable ``find_package(Otter CONFIG)``
   in CMake projects which depend on Otter.
-  ``lib/cmake/Otter/FindOTF2.cmake`` to allow an Otter installation to find
   its OTF2 dependency on behalf of dependent projects.
-  ``etc/modulefiles/otter/otter`` (to provide ``module load otter`` on
   machines which make use of modulefiles)

Using Otter
---------------------------------------------------

.. note::

   It is **strognly recommended** that you install Otter before using it
   to annotate your application. This will ensure the necessary Otter
   headers and libraries are available when compiling your target
   application.

Using Otter with the ``module`` command (recommended)
"""""""""""""""""""""""""""""""""""""""""""""""""""""

Add your Otter installation to the ``MODULEPATH`` environment variable like so:

::

    module use --append <install-prefix>/etc/modulefiles/otter

Alternatively, create a link to the folder containing the installed modulefile in a location that is
already on your ``MODULEPATH`` like so:

::

    ln -s <install-prefix>/etc/modulefiles/otter ~/.config/modulefiles/otter

Using Otter in a CMake project
""""""""""""""""""""""""""""""

When Otter is installed, a CMake ``find_package()`` script is also installed that
imports the ``Otter::Otter`` target. To link to Otter in a CMake project, you
simply need to do the following:

::

    find_package(Otter CONFIG)
    target_link_libraries(targetApp INTERFACE Otter::Otter)

The ``Otter::Otter`` target manages discovery of and linking to the OTF2 dependency
on behalf of the calling project, so there is no need to also call
``find_package(OTF2)`` or ``target_link_libraries(... INTERFACE OTF2)``.

To make the Otter target visible to CMake, you can do any **one** of the following:

-  Use the modulefile (see above) to set up your environment to use Otter (**recommended**).
-  Pass the CMake variable ``-DOtter_ROOT=<install-prefix>``.
-  Define the environment variable ``Otter_ROOT=<install-prefix>`` to point to
   the root of your Otter installation.

Using Otter in a non-CMake project
""""""""""""""""""""""""""""""""""

If you do a basic ``cmake --install .`` from your build directory,
this should install Otter to a standard location on your system, and
the Otter headers and libraries should be visible to your compiler by
default.

If you choose a custom installation prefix with ``--prefix=<install-prefix>`` then
you will need to tell your compiler where you have installed Otter when using
it in your target application. Use ``-I<install-prefix>/include`` and
``-L<install-prefix>/lib`` arguments to add the relevant include and
library paths. **Note:** this is handled for you if you use the
modulefile provided with Otter (see above).

To link against Otter, use ``-lotter-task-graph -lstdc++``. You will likely also need to
pass ``-L<otf2-install-prefix>/lib -lotf2 -lm`` to link the OTF2 dependency. Note
that OTF2 comes with the helpful ``otf2-config`` utility to provide these arguments
programatically.


Installing PyOtter
---------------------------------------------------

The only non-Python dependency is the ``dot`` command, available as part of `graphviz <https://graphviz.org/>`__.
Full installation instructions are available `here <https://graphviz.org/download/>`__ but should be as
simple as:

::

   sudo apt install graphviz

The latest version of PyOtter is obtained by running:

::

   git clone -b dev https://github.com/Otter-Taskification/pyotter.git
   pip install ./pyotter/
