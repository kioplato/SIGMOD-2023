#!/bin/bash

# Library to log events to specified log file path.

__LOG_FILE=""

# Set the log file's path.
function set_log_file_path
{
	# The path to the log file.
	__LOG_FILE=$1
}

function log
{
	# The string to write to the log file.
	str=$1

	current_time=$(date +"[%d/%m/%y %H:%M:%S]")

	# Write the current time and the provided string to the log file.
	echo "$current_time: $str" >> "$__LOG_FILE"
}
