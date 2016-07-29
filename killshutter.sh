#!/bin/bash

ps aux | grep shutter-gk | grep -v grep | awk '{print $2}' | xargs -i kill -9 {}
ps aux | grep startshutter.sh |grep -v grep |awk '{print $2}' | xargs -i kill -9 {} 
