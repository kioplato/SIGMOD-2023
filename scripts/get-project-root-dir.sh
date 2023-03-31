#!/bin/bash

# Get a relative path of the calling script's directory.
function get_project_root_dir
{
	# The path of the calling script.
	local script_path=$1
	# The relative path to compute, relative to the script's directory.
	local relative_path=$2
	# Where to save the path.
	local -n result_path=$3

	# Dereference any links in the path.
	local script_abs_path
	script_abs_path=$(readlink -f "$script_path")
	# Get the directory part of @script_abs_path.
	local script_dir
	script_dir=$(dirname "$script_abs_path")
	# Get the absolute path to the target path. But first make sure that
	# the @relative_path does not have a trailing or leading slash.
	relative_path=$(echo "$relative_path" | sed 's/^\///')
	relative_path=$(echo "$relative_path" | sed 's/\/$//')
	# Now store the resulting path to the refname variable.
	result_path=$(readlink -f "$script_dir/$relative_path")
}
