#! /usr/bin/nash
# Otter run-time environment variables

# in case it was previously set
unset OTTER_APPEND_HOSTNAMEy

# If defined, append hostname to all output files
# export OTTER_APPEND_HOSTNAME=

# OTF2 trace directory
export OTTER_OTF2_TRACE_PATH="/home/adam/git/otter/scratch/trace"

# OTF2 trace name
export OTTER_OTF2_TRACE_NAME="Otter-OTF2-Archive"

printf "%-25s %s\n" "OTTER_OTF2_TRACE:" $OTTER_OTF2_TRACE

if [ -z "${OTTER_APPEND_HOSTNAME+x}" ];
then
	printf "%-25s %s\n" "OTTER_APPEND_HOSTNAME:" "No"
else
	printf "%-25s %s\n" "OTTER_APPEND_HOSTNAME:" "Yes"
fi


