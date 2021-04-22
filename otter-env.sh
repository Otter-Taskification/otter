#! /usr/bin/nash

# Run-time environment variables

# Graph output file format: dot/adj/edge
export OTTER_TASK_TREE_FORMAT=dot

# Graph output file name
export OTTER_TASK_TREE_OUTPUT='task-tree-graph'

# Task metadata output
# export OTTER_TASK_TREE_META_OUTPT=Otter-tasks.csv

# Run commands
# OMP_TOOL_LIBRARIES=lib/libotter.so ./omp-demo
# dot -Tsvg -o tasks.svg $OTTER_TASK_TREE_OUTPUT.gv

printf "OTTer environment variables:\n"
printf "%-25s %s\n" "OTTER_TASK_TREE_FORMAT" "$OTTER_TASK_TREE_FORMAT"
printf "%-25s %s\n" "OTTER_TASK_TREE_OUTPUT" "$OTTER_TASK_TREE_OUTPUT"
