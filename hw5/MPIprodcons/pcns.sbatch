#!/bin/bash
#SBATCH -A TG-SEE120004
#SBATCH -n 16
#SBATCH -J prodConsNoSleep
#SBATCH -o pcns.o%j
#SBATCH -p normal
#SBATCH -t 00:15:00
cdw
ibrun prod_cons.exe 1000 100 14
