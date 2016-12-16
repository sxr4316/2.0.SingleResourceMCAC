#!/bin/bash

function process {
    skip=1
    product=1
    while read line; do
        if [ "$line" = "" ]; then
            if [ $skip != 1 ]; then
                (( sum=sum+product ))
            fi
            product=1
            skip=1
        else
            skip=0
            (( value=$line ))
            (( product=product*value ))
        fi
    done < $1
    rm -rf $1
    if [ $skip != 1 ]; then
        (( sum=sum+product ))
    fi
}

# Change directory to "source"
pushd source > /dev/null

# Check for "array[]" in tables.h and show it to the user if present
gcc -DWMOPS=1 -E g711llc/tables.h | grep "extern.*const.*\[.*\]" | grep '\[[:space:]*\]' 1>&2

# Calculate sum
sum=0
gcc -DWMOPS=1 -E g711llc/tables.h | sed 's!/\*.*\*/!!g' > check_rom_tmp.\$\$\$

# Include all "extern...const..." lines, except for "extern...const...Word8..."
grep "extern.*const.*\[.*\]" check_rom_tmp.\$\$\$ | grep -v "extern.*const.*Word8.*\[.*\]" | tr '[' '\n' | grep ']' | tr ';' '\n' | sed 's/\].*$//' > check_rom_tmp_w16.\$\$\$
# Include all "extern...const...Word8*..." lines
grep "extern.*const.*Word8.*\*.*\[.*\]" check_rom_tmp.\$\$\$ | tr '[' '\n' | grep ']' | tr ';' '\n' | sed 's/\].*$//' >> check_rom_tmp_w16.\$\$\$
process check_rom_tmp_w16.\$\$\$
(( sum=2*sum ))

# Include all "extern...const...Word8..." lines but exclude all "extern...const...Word8*..." lines
grep "extern.*const.*Word8.*\[.*\]" check_rom_tmp.\$\$\$ | grep -v "extern.*const.*Word8.*\*.*\[.*\]" | tr '[' '\n' | grep ']' | tr ';' '\n' | sed 's/\].*$//' > check_rom_tmp_w8.\$\$\$
process check_rom_tmp_w8.\$\$\$

rm -rf check_rom_tmp.\$\$\$

echo "$sum octets"

# Change directory back to the original
popd > /dev/null
