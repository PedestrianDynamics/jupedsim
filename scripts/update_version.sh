#!/bin/sh

#replace in the source files
#grep --color -R "v0.5" ./*.cpp
#replace in the header files

old_version='v0.5'
new_version='v0.6'

for f in $(find ./ -name '*.cpp' -or -name '*.h'); 
do 

#file uptodate
if grep -R -q "$new_version" $f; then
	echo "File $f is uptodate"
	continue
fi

if grep -R -q "$old_version" $f; then
	echo "Processing $f  $old_version ---> $new_version"
	sed -i "s/${old_version}/${new_version}/g" $f
else
	echo "File $f does not contain a version"
fi
done

#for f in *.cpp
#do
# echo "Processing $f"
 # do something on $f
#done
#find . -type f -print0 | xargs -0 sed -i 's/subdomainA.example.com/subdomainB.example.com/g'
#find . -type f -name "*.cpp" -print0 | xargs -0 sed -i'' -e 's/$old_version/$new_version/g'
