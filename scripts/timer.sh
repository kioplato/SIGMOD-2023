#!/bin/bash

# A simple library for bash timers. The timer calls a specified function as
# callback when the timer expires. This timer was created to cutoff a function
# or a program after a specified time. To do this you should wrap the function
# or program you want to cutoff with the start-timer and end-timer functions.
#
# To trigger the timer USR1 signal is used. Therefore, between the start-timer
# and end-timer you should not trap or untrap the USR1 signal.
#
# When the start-timer or end-timer is running the 3 fd will be used.

function start-timer
{
	# The timer's length in seconds.
	timer_seconds=$1
	# The callback function, with its arguments.
	callback_fn=$2

	# The pid of this process.
	# This process receives the USR1 signal.
	receiving_pid=$$

	trap '$callback_fn' USR1

	# We need a unique file that will serve the purpose of indicating that
	# the timer hasn't been triggered and it's still running. Also, this
	# file will store the timer's pid for stop-timer to know its pid in
	# order to kill it. We also have a lockfile such that the timer isn't
	# triggered at about the same time that's being deleted, for safety.
	timer_file="/tmp/$$-timer.txt"
	lock_file="/tmp/$$-timer.lock"

	# Start the timer.
	{
		echo "timer: sleeping..."
		# Do the actual waiting.
		sleep "$timer_seconds"

		echo "timer: woke up!"

		echo "timer: locking lockfile $lock_file"
		# Trigger the timer. Acquire the lock.
		exec 3<> $lock_file
		flock 3
		echo "timer: locked lockfile $lock_file"

		rm -f $timer_file
		kill -USR1 $receiving_pid

		echo "timer: sent USR1 signal"

		# Release the lock.
		exec 3>&-

		echo "timer: released the lockfile $lock_file"
	} &

	echo "start-timer: started timer"

	# Write the timer's pid in the timer file.
	echo "$!" > $timer_file
}

function stop-timer
{
	# The two files storing the necessary information for the timer.
	timer_file="/tmp/$$-timer.txt"
	lock_file="/tmp/$$-timer.lock"

	# Acquire the lock.
	exec 3<> $lock_file
	flock 3

	echo "stop-timer: locked lockfile $lock_file"

	# Reset the USR1 signal handling to default.
	trap - USR1

	echo "stop-timer: reseted the trap"

	# If the @timer_file still exists it means the timer hasn't gone off.
	if [ -e $timer_file ]
	then
		echo "stop-timer: $timer_file exists."

		# Get the timer's pid from its file.
		timer_pid="$(cat $timer_file)"
		# Kill the timer's only child process: the `sleep` command.
		timer_sleep_pid="$(ps -o pid= --ppid "$timer_pid")"
		if [ -z "$timer_sleep_pid" ]
		then
			echo "stop-timer: timer has sleep worker"
			kill "$timer_sleep_pid"
		fi
		# Kill the timer itself.
		kill "$timer_pid"

		echo "stop-timer: killed timer"

		# Cleanup the timer file.
		rm -f $timer_file
	fi

	# Release the lock.
	exec 3>&-

	echo "stop-timer: unlocked lockfile $lock_file"

	# The timer no longer exists, we can safely delete the lockfile.
	rm -f $lock_file
}
