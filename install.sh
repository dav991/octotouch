#!/bin/sh


mkdir -p /opt/octotouch
mkdir -p /opt/octotouch/css
mkdir -p /opt/octotouch/glade
if [ ! -f /opt/octotouch/octotouch.cfg ]; then
	cp build/octotouch.cfg /opt/octotouch/
fi
cp build/octotouch /opt/octotouch/
cp build/css/* /opt/octotouch/css
cp build/glade/* /opt/octotouch/glade
cp octotouch.service /etc/systemd/system/
systemctl enable octotouch
