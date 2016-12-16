#!/bin/csh -f

set pwd = `pwd`;
set dn = `dirname $0`;
set bn = `basename ${pwd}`;

set GENERATE_COMMAND = "${dn}/chk_itu_mc.csh -v -e ${USER}@rit.edu";

echo "This can take up to 1 hour, you will recieve an email when completed."
${GENERATE_COMMAND};
