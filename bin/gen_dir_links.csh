#!/bin/csh -f

set dir_list = ./etc/DIRS.txt 
set root = `pwd`

echo "========="
echo ${root}
echo "========="

cd ${root}

set dirs = (`cat ${dir_list}`)

foreach dir (${dirs})
	echo "========="
	echo ${dir}

	if ((! -e ${dir}) && (! -d ${dir})) then
		echo "  Linking ${dir}"
		ln -s ../mcac_s2015/${dir}
	else
		echo "  Skipping ${dir}"
	endif
	echo "========="
end
