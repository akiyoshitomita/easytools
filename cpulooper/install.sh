#!/bin/bash

CL_SYSD=/etc/systemd/system  # systemd 
CL_SBIN=/usr/loca/sbin

cp ./src/cpulooper.service ${CL_SYSD} 
cp ./src/cpulooper_run.sh  ${CL_SBIN} 
cp ./src/cpulooper.pl      ${CL_SBIN} 

chmod 755 ${CL_SBIN}/cpulooper_run.sh
chmod 755 ${CL_SBIN}/cpulooper.pl
ln -s ${CL_SBIN}/cpulooper_run.sh ${CL_SBIN}/cpulooper
