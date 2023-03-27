#!/bin/bash

# Library to handle errors beautifly.

_SCRIPT_BASENAME=$(basename -s .sh "$0")

# Print the provided string and die.
# This function should be used when its the user's error.
function die()
{
	# String to print to stdout.
	str=$1

	echo "[$_SCRIPT_BASENAME] fatal: $str"
	echo "run $_SCRIPT_BASENAME --help for usage."

	exit 1
}

# Print the provided string and die.
# This function should be used when its an unexpected error.
function diei()
{
	# String to print to stdout.
	str=$1

	echo "[$_SCRIPT_BASENAME] internal: $str"
	echo "run $_SCRIPT_BASENAME --help for usage."

	exit 1
}
