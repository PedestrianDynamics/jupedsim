#!/bin/sh

old_version='0.5'
new_version='0.8'


#old_version='http://134.94.2.137/jps_ini_core.xsd'
#new_version='http://xsd.jupedsim.org/0.6/jps_ini_core.xsd'

#for f in $(find ../ -name '*.cpp' -or -name '*.h' -or -name '*.xml' -or -name '*.py');
for f in $(find ../Utest -name 'geometry*.xml' -or -name 'master*.xml' );  
do 


if grep -R -q "version=" $f; then
    echo "Processing $f"  
    sed -i -e "s/version=\"0.[0-9]\"/version=\"$new_version\"/g" $f
    d=`dirname $f`
    echo "remove $d/*.xml-e"
    rm $d/*.xml-e
    # read -p "Press [Enter] key to continue..."
else
	echo "WARNING: File $f does not contain a version number"
fi
done

