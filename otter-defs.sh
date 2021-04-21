#! /usr/bin/bash

export DEBUG_OTTER=3
export DEBUG_OTT=3
export DEBUG_ODT=3

export OTTER_DEFAULT_TASK_CHILDREN=1000
export OTT_DEFAULT_ROOT_CHILDREN=1000
export ODT_DEFAULT_ARRAY_LENGTH=100
export ODT_DEFAULT_ARRAY_INCREMENT=500

# Run-time environment variables
export OTTER_TASK_TREE_FORMAT=dot  # dot / edge / adj
export OTTER_TASK_TREE_OUTPUT='task-tree-graph'
# export OTTER_TASK_TREE_META_OUTPT=Otter-tasks.csv

# OMP_TOOL_LIBRARIES=lib/libotter.so ./omp-demo

# dot -Tsvg -o tasks.svg $OTTER_TASK_TREE_OUTPUT.dot
