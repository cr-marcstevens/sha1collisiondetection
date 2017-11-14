#!/bin/sh

if test e98a60b463a6868a6ce351ab0166c0af0c8c4721 != `bin/sha1dcsum test/sha1_reducedsha_coll.bin | cut -d' ' -f1` || (echo "\nError: Compiled for incorrect endianness" && false); then : ; else echo "ERROR 1"; exit 1; fi
if test a56374e1cf4c3746499bc7c0acb39498ad2ee185 = `bin/sha1dcsum test/sha1_reducedsha_coll.bin | cut -d' ' -f1`; then : ; else echo "ERROR 2"; exit 1; fi
if test 16e96b70000dd1e7c85b8368ee197754400e58ec = `bin/sha1dcsum test/shattered-1.pdf | cut -d' ' -f1`; then : ; else echo "ERROR 3"; exit 1; fi
if test e1761773e6a35916d99f891b77663e6405313587 = `bin/sha1dcsum test/shattered-2.pdf | cut -d' ' -f1`; then : ; else echo "ERROR 4"; exit 1; fi
if test dd39885a2a5d8f59030b451e00cb45da9f9d3828 = `bin/sha1dcsum_partialcoll test/sha1_reducedsha_coll.bin | cut -d' ' -f1` ; then : ; else echo "ERROR 5"; exit 1; fi
if test d3a1d09969c3b57113fd17b23e01dd3de74a99bb = `bin/sha1dcsum_partialcoll test/shattered-1.pdf | cut -d' ' -f1`; then : ; else echo "ERROR 6"; exit 1; fi
if test 92246b0b718f4c704d37bb025717cbc66babf102 = `bin/sha1dcsum_partialcoll test/shattered-2.pdf | cut -d' ' -f1`; then : ; else echo "ERROR 7"; exit 1; fi
if test 38762cf7f55934b34d179ae6a4c80cadccbb7f0a = `bin/sha1dcsum_simd test/shattered-1.pdf | cut -d' ' -f1`; then : ; else echo "ERROR 8"; exit 1; fi
if test 38762cf7f55934b34d179ae6a4c80cadccbb7f0a = `bin/sha1dcsum_simd test/shattered-2.pdf | cut -d' ' -f1`; then : ; else echo "ERROR 9"; exit 1; fi
if test a56374e1cf4c3746499bc7c0acb39498ad2ee185 = `bin/sha1dcsum test/sha1_reducedsha_coll.bin | cut -d' ' -f1`; then : ; else echo "ERROR 10"; exit 1; fi
echo "OK"
