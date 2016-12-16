#!/bin/csh -f

#
# for u-Law
#

set root = `pwd`
set root_in = test_vectors/itu/g726/software/G726ap2/G726ap2e/DISK1
set root_out = test_vectors/itu_ulaw

foreach dir (`cd ${root_out} ; find . -type d -print`)
	foreach file (`find ${root_in}/${dir} -maxdepth 1 -type f -print`)
		#echo ${file:t}
		if ("${file:t}" != "READ.ME") then
			./vec_strip -i ${file} -o ${root_out}/${dir}/${file:t}
		endif
	end
end

set root_in = test_vectors/itu/g726/software/G726ap2/G726ap2e/DISK2
set root_out = test_vectors/itu_alaw

foreach dir (`cd ${root_out} ; find . -type d -print`)
	foreach file (`find ${root_in}/${dir} -maxdepth 1 -type f -print`)
		#echo ${file:t}
		if ("${file:t}" != "READ.ME") then
			./vec_strip -i ${file} -o ${root_out}/${dir}/${file:t}
		endif
	end
end
