#!/bin/bash

interface="${1}"

seconds=1

tx_file="/sys/class/net/${interface}/statistics/tx_bytes"
rx_file="/sys/class/net/${interface}/statistics/rx_bytes"

old_tx=$(cat $tx_file)
old_rx=$(cat $rx_file)

format() {
	local bytes="${1}"
	local output=""
	local grenze=$((1024*1024*1024))

	if [[ "${bytes}" -gt $grenze ]]; then
		einheit="GiB/s"
	elif [[ "${bytes}" -gt $((grenze/=1024)) ]]; then
		einheit="MiB/s"
	#elif [[ "${bytes}" -gt $((grenze/=1024)) ]]; then
	else
		((grenze/=1024))
		einheit="KiB/s"
	fi

	printf "%3d %s" $((bytes/grenze)) $einheit
}


while true
do
	sleep $seconds
	new_tx="$(cat ${tx_file})"
	new_rx="$(cat ${rx_file})"

	bytes_tx=$(( new_tx-old_tx ))
	bytes_rx=$(( new_rx-old_rx ))

	echo -e -n "\e[2K\e[1G"
	echo -e -n "Tx: $(format $((bytes_tx/seconds))), Rx: $(format $((bytes_rx/seconds)))"

	old_tx="${new_tx}"
	old_rx="${new_rx}"
done
