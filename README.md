# Otter

Developed under the [ExCALIBUR task parallelism cross-cutting research theme](https://excalibur.ac.uk/projects/exposing-parallelism-task-parallelism/), Otter is a tool designed to facilitate data-driven parallelisation of serial code. Otter allows HPC developers to:

- Annotate, trace & visualise loop/task-based serial code as a directed graph;
- Recommend strategies for transforming serial code into effective task-based parallel code;
- Non-invasively trace & visualise loop/task-based OpenMP 5.x programs.

The [project wiki](https://github.com/Otter-Taskification/otter/wiki) introduces the Otter toolset and explains how to use the features above.

## Otter Toolset

The Otter toolset includes:

- [**Otter-Task-Graph**]([Otter-Task-Graph](https://github.com/Otter-Taskification/otter/wiki/Otter-Task-Graph)): an API and runtime library for annotating & tracing the task-graph of a (possibly parallel) target application.
<!-- - [**Otter-Serial**]([Otter-Serial](https://github.com/Otter-Taskification/otter/wiki/Otter-Serial)): an API and runtime library for annotating & tracing the structure of a serial target application. -->
<!-- -  in order to facilitate data-driven parallelisation of the target. -->
- [**Otter-OMPT**]([Otter-OMPT](https://github.com/Otter-Taskification/otter/wiki/Otter-OMPT)): an OMPT tool for non-invasive tracing of the loop/task-based structure of OpenMP 5.x programs.
<!-- - , allowing HPC developers to observe OpenMP program structure from the perspective of the OpenMP runtime. -->
- [**PyOtter**]([PyOtter](https://github.com/Otter-Taskification/otter/wiki/PyOtter)): The visualisation & reporting tool for use with Otter trace data.

## Getting Started

- [Installation guide](https://github.com/Otter-Taskification/otter/wiki#installation-guide)
- [Using Otter-Task-Graph](https://github.com/Otter-Taskification/otter/wiki/Otter-Task-Graph/#using-otter-task-graph)
- [Using Otter-OMPT](https://github.com/Otter-Taskification/otter/wiki/Otter-OMPT#getting-started)
- [Using PyOtter](https://github.com/Otter-Taskification/otter/wiki/PyOtter)

## Issues, Questions and Feature Requests

For **Otter-Task-Graph**, **Otter-Serial** or **Otter-OMPT**, please post [here](https://github.com/Otter-Taskification/otter/issues).

For **PyOtter**, please post [here](https://github.com/Otter-Taskification/pyotter/issues).

## Licensing

Otter is released under the BSD 3-clause license. See [LICENCE](LICENCE) for details.

Copyright (c) 2021, Adam Tuft
All rights reserved.

## Acknowledgements

Otter's development started as the subject of a final project and dissertation for the the [Scientific Computing and Data Analysis MSc](https://miscada.phyip3.dur.ac.uk/) (MISCADA) at Durham University, UK. The current research is supported by EPSRC's Excalibur programme through its cross-cutting project EX20-9 *Exposing Parallelism: Task Parallelism* (Grant ESA 10 CDEL).
