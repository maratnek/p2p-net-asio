#/bin/bash
# echo "current directory"
# pwd
echo "config net"
# tc qdisc show dev eth0
# tc qdisc add dev eth0 root tbf rate 80kbit burst 32kb latency 10ms # this 25ms delay
# tc qdisc add dev eth0 root tbf rate 1mbit burst 10kb latency 25ms
# tc qdisc add dev eth0 root netem delay 25ms
echo "start p2p net server"
./app/p2p-test ./app/config.json