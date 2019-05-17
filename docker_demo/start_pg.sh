#!/bin/bash

su - postgres -c '/usr/bin/pg_ctl -D /var/lib/pgsql/data -l logfile start'

# prevent exit

while : 
do 
  echo '*'; sleep 5; 
done >/tmp/log_output.$$  
