Otter Documentation
===================

Developed under the `ExCALIBUR task parallelism cross-cutting research
theme <https://excalibur.ac.uk/projects/exposing-parallelism-task-parallelism/>`__,
Otter is a tool designed to facilitate data-driven parallelisation of
serial code. Otter allows HPC developers to:

-  Annotate, trace & visualise loop/task-based serial code as a directed
   graph;
-  Recommend strategies for transforming serial code into effective
   task-based parallel code;
-  Non-invasively trace & visualise loop/task-based OpenMP 5.x programs.

Check out the :doc:`installation </pages/installation>` page for instructions on installing Otter.

.. note::

   This project is under active development.
   
.. toctree::
    :hidden:
    :maxdepth: 2

    Installation </pages/installation>
    Otter Task Graph </pages/otter-task-graph>
    Otter OMPT </pages/otter-ompt>
    PyOtter </pages/pyotter>

Otter Toolset
-------------

The Otter toolset includes:

-  :doc:`Otter task-graph </pages/otter-task-graph>`: an API and runtime library
   for annotating & tracing the task-graph of a (possibly parallel)
   target application.
-  :doc:`Otter OMPT </pages/otter-ompt>`: an OMPT plugin for non-invasive tracing of
   the loop/task-based structure of OpenMP 5.x programs.
-  :doc:`PyOtter </pages/pyotter>`: The visualisation & reporting tool for use
   with Otter trace data.


Issues, Questions and Feature Requests
--------------------------------------

For **Otter task-graph** and **Otter-OMPT**, please
post `here <https://github.com/Otter-Taskification/otter/issues>`__.

For **PyOtter**, please post
`here <https://github.com/Otter-Taskification/pyotter/issues>`__.

Licensing
---------

Otter is released under the BSD 3-clause license. See
`LICENCE <https://github.com/Otter-Taskification/otter/blob/dev/LICENSE>`__ for details.

Copyright (c) 2021, Adam Tuft All rights reserved.

Acknowledgements
----------------

Otter's development started as the subject of a final project and
dissertation for the `Scientific Computing and Data Analysis
MSc <https://miscada.phyip3.dur.ac.uk/>`__ (MISCADA) at Durham
University, UK. The current research is supported by EPSRC's Excalibur
programme through its cross-cutting project EX20-9 *Exposing
Parallelism: Task Parallelism* (Grant ESA 10 CDEL).

.. image:: images/ExCALIBUR_colour.png
    :align: center
    :width: 250px
    :target: https://excalibur.ac.uk/projects/exposing-parallelism-task-parallelism/
    :alt: images/ExCALIBUR_colour.png

.. image:: /images/ExCALIBUR_colour.png
    :align: center
    :width: 250px
    :target: https://excalibur.ac.uk/projects/exposing-parallelism-task-parallelism/
    :alt: /images/ExCALIBUR_colour.png

.. image:: source/images/ExCALIBUR_colour.png
    :align: center
    :width: 250px
    :target: https://excalibur.ac.uk/projects/exposing-parallelism-task-parallelism/
    :alt: source/images/ExCALIBUR_colour.png

.. image:: /source/images/ExCALIBUR_colour.png
    :align: center
    :width: 250px
    :target: https://excalibur.ac.uk/projects/exposing-parallelism-task-parallelism/
    :alt: /source/images/ExCALIBUR_colour.png
