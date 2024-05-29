#!/bin/bash

# 创建虚拟串口对
socat -d -d pty,link=/dev/ttyV0,raw,echo=0 pty,link=/dev/ttyV1,raw,echo=0 &
