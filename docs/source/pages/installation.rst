Installation
===================================================

This page explains how to build and install Otter.

Otter
---------------------------------------------------

Pre-requisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Otter requires a reasonably recent version of CMake (>=3.21). To build Otter's
OMPT plugin you must have a C compiler which supports OpenMP 5.0 and the OMPT
interface (see `here <https://www.openmp.org/resources/openmp-compilers-tools/>`__
for a list of compilers).

Otter requires the OTF2 tracing library. We recommend installing
`OTF v2.3 <https://zenodo.org/record/4682684>`__ as this is the only version
Otter has been tested with. Full installation instructions for OTF2 are included
with the OTF2 source. To download, build and install OTF2, run:

::

   wget https://zenodo.org/record/4682684/files/otf2-2.3.tar.gz
   tar -xzvf otf2-2.3.tar.gz && cd otf2-2.3
   ./configure
   make
   make install

The default installation location of ``/opt/otf2`` can be overridden with the ``--prefix`` option to ``./configure``.

Building Otter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To build Otter, first obtain the latest version of the Otter repository:

::

   git clone -b dev https://github.com/Otter-Taskification/otter.git
   cd otter/

Otter uses CMake to perform an out-of-source build. For a first-time build, we
recommend using a preset:

::

    cmake --preset default
    cmake --build --preset default

Otter checks for an OTF2 installation under the default location of ``/opt/otf2``.
If you installed OTF2 somewhere else, use ``-DOTF2_INSTALL_DIR=<install-path>``
to tell Otter where to look.

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
| ``-DCMAKE_BUILD_TYPE=[Debug\|Release]``                                        | Build type                |     ``Release``     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DOTF2_INSTALL_DIR=[PATH]``                                                  | Location of OTF2          |    ``/opt/otf2``    |
|                                                                                | installation              |                     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DWITH_OMPT_PLUGIN=[ON\|OFF]``                                               | Build the OMPT plugin     |            ``OFF``  |
|                                                                                |                           |                     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DWITH_EXAMPLES=[ON\|OFF]``                                                  | Build Otter with examples |            ``OFF``  |
|                                                                                |                           |                     |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DWITH_TESTS=[ON\|OFF]``                                                     | Build tests               |            ``OFF``  |
+--------------------------------------------------------------------------------+---------------------------+---------------------+
| ``-DBUILD_SHARED_LIBS=[ON\|OFF]``                                              | Build shared libraries    |            ``OFF``  |
+--------------------------------------------------------------------------------+---------------------------+---------------------+


Installing Otter
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To install the ``default`` preset:

::

    cmake --install build/default/ [--prefix=...]

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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This section contains generic advice to help you set up your environment
to use Otter.

.. note::

   It is **strognly recommended** that you install Otter before using it
   to annotate your application. This will ensure the necessary Otter
   headers and libraries are available when compiling your target
   application.

Using Otter with the ``module`` command
"""""""""""""""""""""""""""""""""""""""

The Otter modulefile is the recommended method for setting up your environment
to use Otter.

If you have the `module <https://modules.readthedocs.io/en/latest/index.html>`__ command on your machine, the Otter modulefile is the easiest
way to set up your environment to use Otter. To enable the modulefile, add your
Otter installation to the ``MODULEPATH`` environment variable, for example:

::

    module use --append <installdir>/etc/modulefiles/otter
    
You may wish to persist this in your ``~/.bashrc`` or similar.

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

-  (**recommended**) Use the modulefile (see above) to set up your environment to use Otter.
-  Define the environment variable ``Otter_ROOT=<install-prefix>`` to point to
   the root of your Otter installation.
-  Pass the CMake variable ``-DOtter_ROOT=<install-prefix>``.

Using Otter in a non-CMake project
""""""""""""""""""""""""""""""""""

If you do a basic ``cmake --install .`` from your build directory,
this should install Otter to a standard location on your system, and
the Otter headers and libraries should be visible to your compiler by
default.

If you choose a custom installation prefix with ``--prefix=<installdir>`` then
you will need to tell your compiler where you have installed Otter when using
it in your target application. Use ``-I<installdir>/include`` and
``-L<installdir>/lib`` arguments to add the relevant include and
library paths. **Note:** this is handled for you if you use the
modulefile provided with Otter (see above).

To link against Otter, use ``-lotter-task-graph``. You will likely also need to
pass ``-L<otf2-install-prefix>/lib -lotf2 -lm`` to link the OTF2 dependency. Note
that OTF2 comes with the helpful ``otf2-config`` utility to provide these arguments
programatically.


PyOtter
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
