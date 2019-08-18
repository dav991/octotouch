#!/bin/sh


rm -rf /opt/octotouch/
mkdir -p /opt/octotouch
mkdir -p /opt/octotouch/css
mkdir -p /opt/octotouch/glade
cp build/octotouch.cfg /opt/octotouch/
cp build/octotouch /opt/octotouch/
cp build/css/* /opt/octotouch/css
cp build/glade/* /opt/octotouch/glade
cp octotouch.service /etc/systemd/system/
systemctl enable octotouch