#!/bin/bash

#collect data and transform into standard code format
./collect.py > log/collect.log;

#transform into data
./store >> log/collect.log;

#back up data and replace 
tar -zpcf backup/data-`date +%Y-%m-%d-%H-%M.tar.gz` data;
cp temp_data/* data;
rm -rf temp_data;
rm -rf tmp/*;
#killall searchd;
#./searchd;


