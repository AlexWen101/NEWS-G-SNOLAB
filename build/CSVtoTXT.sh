#!/bin/bash

# This macro finds all files with a .csv extension, replaces all commas with spaces, deletes the first lines that don't contain data, and outputs .txt. 
# Written to convert the default .csv output from ShieldSimulation into a format that can be more easily taken in by ROOT macros and QUADIS. 
# NOTE: THIS MACRO IS NOT EXPECTED TO WORK ON GNU (Some funny business with sed)

for f in *.csv; do
    sed -i '' -e 's/,/ /g' $f    # replace all commas with spaces
    sed -i '' -e '1,35d' $f   # deletes first 35 lines (may change depending on output)
    rename 's/csv$/txt/' $f  # replaces .csv with .txt in file extension
done

