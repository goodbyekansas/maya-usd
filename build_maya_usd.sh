#!/bin/bash

scl enable devtoolset-6 bash
echo y | rm -rf /usr/local/workspace2019
echo "BUILDING maya-usd for maya 2019"
CMAKE=cmake3 sudo python build.py --maya-location /usr/autodesk/maya2019 --v 3 --pxrusd-location /usr/local/USD.19/USD-core /usr/local/workspace2019 --install /usr/local/USD.19/maya-usd-2019
echo y | rm -rf /usr/local/workspace2019
echo "BUILDING maya-usd for maya 2019 usd v20.0"
CMAKE=cmake3 sudo python build.py --maya-location /usr/autodesk/maya2019 --v 3 --pxrusd-location /usr/local/USD.20/USD-core /usr/local/workspace2019 --install /usr/local/USD.20/maya-usd-2019


echo "DONE!"
