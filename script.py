#!/bin/python

import os 

cmd = "/usr/local/common/gcc-7.3.0/bin/g++ proj4.cpp -o proj4 -lm -fopenmp"
os.system(cmd)
cmd = "./proj4 >> output.txt"
os.system(cmd)
	