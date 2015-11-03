#!/bin/sh

old_version='v0.7'
new_version='v0.8'


#old_version='http://134.94.2.137/jps_ini_core.xsd'
#new_version='http://xsd.jupedsim.org/0.6/jps_ini_core.xsd'

#for f in $(find ../ -name '*.cpp' -or -name '*.h' -or -name '*.xml' -or -name '*.py');
for f in $(find ../ -name '*.cpp' -or -name '*.h' );  
do 

#file uptodate
if grep -R -q "$new_version" $f; then
	#echo "File $f is uptodate"
	continue
fi

if grep -R -q "$old_version" $f; then
	echo "Processing $f  $old_version ---> $new_version"
#	sed -i "s/${old_version}/${new_version}/g" $f
else
	echo "File $f does not contain a version number"
fi
done

