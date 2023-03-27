#!/bin/bash

# Find a unique filename for a specified prefix and suffix.
# To achieve that an increasing number is picked from 1 till bash biggest int.
# The returned file name is guaranteed to exist
function find_unique_filename()
{
	# The unique filename requested.
	local -n unique_filename=$1
	filename=$unique_filename

	# The number to make the file unique.
	file_version=1

	candidate=$filename-$file_version

	while ls "$candidate"* > /dev/null 2>&1
	do
		((++file_version))
		candidate=$filename-$file_version
	done

	unique_filename=$candidate
}
