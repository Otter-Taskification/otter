---
author:
- Adam Tuft
title: |
    OTTER:\
    OpenMP Task Tracer
---

Introduction
============

OTTER Core
==========

OTTER Core implements the OMPT callbacks defined by the OMP runtime. It
collects the data required to build the task graph and to record OTF2
locations, regions and events. OTTER Core defines various types used to
store data relevant to corresponding OMP constructs such as parallel
regions, threads, tasks etc.

OTTER Core defines `scope_type_t` which enumerates the OMP constructs
relevant to OTTER and the `scope_t` object which contains data used to
record entry into and exit from scopes. The scopes defined by OTTER are
associated with OMP constructs which emit begin and end execution model
events e.g. `scope_parallel` is associated with parallel regions and
their *parallel-begin* and *parallel-end* events.

The OTTER scopes corresponding to OMP constructs that can be nested
(e.g. parallel regions) are represented in the task graph by pairs of
nodes (scope entry and exit nodes) which allows the nesting of OMP
constructs to be reflected in the task graph. Those scopes representing
standalone constructs (such as implicit barriers) are represented by
single nodes generated during their *scope-end* events.

A `scope_t` stores references to the scope's begin and end nodes as well
as data used to identify the scope, a flag indicating whether the scope
has encountered its *scope-end* event and a stack storing references to
the nodes created while the scope is active. A mutex ensures thread-safe
updates of its stack. Scopes representing standalone constructs, such as
barrier regions, do not use a `scope_t` as they cannot be nested.
Instead, their task graph node is created by the master thread of the
team during their *scope-end* events and added to the thread's queue of
synchronisation nodes to be processed at the enclosing scope's
*scope-end* event.

A thread tracks the scope within which a callback occurs by maintaining
a stack of `scope_t` objects. When a *scope-begin* event is received, a
`scope_t` is created and pushed onto the encountering thread's stack,
which is then popped during the matching *scope-end* event. At
*scope-begin* the `scope_t` is added to a global queue for the initial
thread to process at *tool-finalise*. Between a scope's begin and end
events, a thread can inspect the innermost scope's data by examining the
struct at the top of its stack.

In order for the graph nodes created during a scope to be connected to
the scope's begin and end nodes, a scope records the nodes it encloses
in a stack. When a node is added to the task graph it is also added to
the innermost scope's node stack. During the *tool-finalise* event the
initial thread consumes the global scope queue, checking the nodes in
each scope's stack and adding an edge from the node to the scope's end
node for each node with no immediate descendants. This ensures that all
nodes that are descendants of the scope's begin node are also ancestors
of its end node.

For standalone constructs a node is added to the task graph during the
construct's *scope-end* event. This node is also added to the
encountering thread's synchronisation node queue but **not** the
enclosing thread's node stack. At the enclosing scope's *scope-end*
event the encountering thread consumes its queue of synchronisation
nodes; for each node in the queue, it consumes the scope's nodes stack
to connect all nodes with no immediate descendants to the
synchronisation node before pushing the synchronisation node onto the
scope's node stack.

*Parallel-begin* and *Implicit-task-begin* events
-------------------------------------------------

(Parallel scopes and implicit-task-begin events - only encountering
thread receices *parallel-begin* and *parallel-end* events but all
participating threads need reference to the same parallel scope object =
create scope in *parallel-begin*, store in parallel data and retrieve
during each thread's *implicit-task-begin* event.)

Connecting Consecutive Scopes
-----------------------------

At a *scope-begin* event a thread connects the new scope to the task
graph by checking the type of the encountering task:

-   for initial tasks, if a prior scope exists an edge is created
    between that scope and the new scope, otherwise an edge is created
    from the initial tasks's node to the new scope's begin node;

-   for implicit tasks, if the prior scope encloses the new scope the
    two scope-begin nodes are connected with an edge, otherwise the
    prior scope's end node is connected to the new scope's begin node;

-   for explitit and target tasks an edge is created from the task's
    node to the new scope's begin node.

Graph Data Structure
====================

OTTER uses a simple representation of a graph which allows addition of
arbitrary nodes and directed edges. The graph is represented as a pair
of queues; one for the nodes and another for the edges. A node is a
struct containing an id, a type indicator, a pointer to arbitrary data
and a flag to indicate whether it is the source of any edges. An edge is
a struct with pointers to its start and end nodes.

Note that OTTER's graph implementation is very basic as it is not
intended to be a general-purpose graph. For example, it does not support
deletion of nodes or edges. It does support iterating over its nodes and
edges with `graph_scan_nodes` and `graph_scan_edges` .

OTTER Task Graph
================

The task graph module manages the underlying graph data structure that
represents the task graph of the OMP program. It manages a single global
instance of a graph which is exposed to OTTER-core only through the
`task_graph_` functions. The task graph is initialised once when OTTER
starts (before the OMP program begins) and is destroyed when OTTER exits
(after the OMP program is finished). Any other initialisation or
destruction is an error. The task graph struct includes a mutex that is
locked when a node or edge is added to ensure thread-safe updates of the
task graph.

The module header defines the `task_graph_node_type_t` enum which lists
the kinds of nodes that can be represented in the task graph. This enum
is closely aligned to the `scope_t` enum defined in `otter-core/otter.h`
and is used to format the nodes when the task graph is saved.

Task graph output file name and format are controlled through
environment variables. OTTER can save the task graph as a dot-file (.gv)
or edge list (.csv).

OTTER Trace
===========

To do\...
