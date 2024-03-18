#!/bin/bash

set -eux

script_dir=$(cd $(dirname $0); pwd)

debs=(
	python3.11
	sudo
	polkitd
	frr
	wireguard-tools
	python3-ifaddr
	python3-yaml
	python3-requests
	python3-git
	python3-jinja2
	libnss-myhostname
	iputils-ping
	net-tools
	tcpdump
	mtr-tiny
)

echo ${debs[@]}
aptcmd="apt-get update && apt-get install --no-install-recommends --download-only -y ${debs[@]}"

podman run --rm -it \
	-v ${script_dir}/extradebs/bookworm:/out \
	debian:12 \
	bash -c "$aptcmd && cp /var/cache/apt/archives/*.deb /out/"

