#! /usr/bin/nash
# Otter run-time environment variables

# in case it was previously set
unset OTTER_APPEND_HOSTNAME

# Graph output file name
export OTTER_TASK_TREE_OUTPUT="${HOME}/Desktop/task-tree-graph-${HOSTNAME}-`date +%H:%M:%s`"

# Graph output file format: dot/adj/edge
export OTTER_TASK_TREE_FORMAT=dot

# If defined, append hostname to all output files
# export OTTER_APPEND_HOSTNAME=

# Task metadata output
export OTTER_TASK_TREE_NODEATTR=Otter-tasks.csv

# Run commands
# OMP_TOOL_LIBRARIES=lib/libotter.so ./omp-demo
# dot -Tsvg -o tasks.svg $OTTER_TASK_TREE_OUTPUT.gv

printf "OTTER environment variables:\n"
printf "%-25s %s\n" "OTTER_TASK_TREE_OUTPUT" "$OTTER_TASK_TREE_OUTPUT"
printf "%-25s %s\n" "OTTER_TASK_TREE_FORMAT" "$OTTER_TASK_TREE_FORMAT"

if [ -z "${OTTER_APPEND_HOSTNAME+x}" ];
then
	printf "%-25s %s\n" "OTTER_APPEND_HOSTNAME" "No"
else
	printf "%-25s %s\n" "OTTER_APPEND_HOSTNAME" "Yes"
fi

printf "%-25s %s\n" "OTTER_TASK_TREE_NODEATTR" "$OTTER_TASK_TREE_NODEATTR"
