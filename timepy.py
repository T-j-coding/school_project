import os
import sys
import shlex
import subprocess
import time
import datetime

command = './shutter-gk  > /dev/null &'

args = shlex.split(command)
process = subprocess.Popen(args)

f = open('a.out', 'w')

while True:
	time.sleep(1)
	process.poll()
	if process.returncode is not None:
		break
	f.write(str(datetime.datetime.now()) + '\n')
	f.flush()

