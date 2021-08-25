#! /usr/bin/nash
# Otter run-time environment variables

# in case it was previously set
unset OTTER_APPEND_HOSTNAME

# If defined, append hostname to all output files
# export OTTER_APPEND_HOSTNAME=

# OTF2 trace directory
export OTTER_TRACE_PATH="scratch/trace"

# OTF2 trace name
export OTTER_TRACE_NAME="archive"

printf "%-25s %s\n" "OTTER_TRACE_PATH:" $OTTER_TRACE_PATH
printf "%-25s %s\n" "OTTER_TRACE_NAME:" $OTTER_TRACE_NAME

if [ -z "${OTTER_APPEND_HOSTNAME+x}" ];
then
	printf "%-25s %s\n" "OTTER_APPEND_HOSTNAME:" "No"
else
	printf "%-25s %s\n" "OTTER_APPEND_HOSTNAME:" "Yes"
fi
