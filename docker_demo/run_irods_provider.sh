#! /bin/bash


## ----      Wait on database           ----
## -----------------------------------------

while [ $((++x)) -lt 10 ] && ! psql -U postgres -h dbhost -c '\l' >/dev/null 2>&1
do
  echo >&2 ... waiting on db server ... "($x)"
  sleep 1
done


## ---- Set up db for ICAT 
## -----------------------------------------

psql -U postgres -h dbhost -f /db_irods.txt


## ---- Configure (or Run) irods server ----
## -----------------------------------------

if ! id -u irods >/dev/null 2>&1
then

  echo >&2 "Configuring and starting iRODS ..."
  python /var/lib/irods/scripts/setup_irods.py < /dbhost_setup_postgres.input

else

  echo >&2 "Starting iRODS ..."
  service irods start

fi && echo >&2  $'\n\t --> IRODS Server is running \n'

while true
do
  sleep 1
done

