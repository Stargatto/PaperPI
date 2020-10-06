#!/bin/sh

cp paperpi.service /etc/systemd/system/
chmod 644 /etc/systemd/system/paperpi.service
systemctl enable paperpi


