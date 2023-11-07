#/bin/bash
ls -la
tc qdisc add dev eth0 root tbf rate 1mbit burst 10kb latency 5000ms
./app/p2p-test ./app/config.json