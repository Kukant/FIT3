#!/bin/bash

# spustit ve slozce s binarkou pgrep

if [ ! -d "IPS-projekt1" ]; then
	wget -q "www.stud.fit.vutbr.cz/~xsedla0v/IPS-projekt1.tar"
	tar -xf "IPS-projekt1.tar"
	rm "IPS-projekt1.tar"
fi

cd "IPS-projekt1"

test() {
	out1=$(eval "./pgrep $1")
	#echo -e "evaluating: ./pgrep $1"
	out2=$(eval "../pgrep $1")
	echo -ne "\033[1;33m"
	diff <(echo "$out1") <(echo "$out2")
	if [ $? == 1 ]; then
		echo -e "\033[0;31m[ERROR]\033[0m ./pgrep $1"
	else
	echo -e "\033[0;32m[OK]\033[0m ./pgrep $1"
	fi
}

test '1 ".*wheel.*" 1 < sudoers'
test '0 ".*wheel.*" -1 < sudoers'
test '1 ".*wheel.*" -1 ".* ALL.*" 1 < sudoers'
test '1 ".*wheel.*" -1 ".* ALL.*" 1 ".*cd.*" 1 < sudoers'
test '2 ".*wheel.*" -1 ".* ALL.*" 1 ".*cd.*" 1 < sudoers'
test '2 ".*wheel.*" -1 ".* ALL.*" 1 ".*cd.*" 1 ".*root.*" 1  < sudoers'
test '3 ".*wheel.*" -1 ".* ALL.*" 1 ".*cd.*" 1 ".*root.*" 1  < sudoers'
test '3 ".*wheel.*" 1 ".* ALL.*" 2 ".*cd.*" 1 ".*root.*" 1  < sudoers'
test '3 ".*wheel.*" 1 ".* ALL.*" 2 ".*cd.*" 1 ".*root.*" 1  ".*NOPASSWD.*" -5 < sudoers'
test '7 ".*NETWORKING,.*" 1 ".*SOFTWARE,.*" 1 ".*SERVICES,.*" 1 ".*STORAGE,.*" 1 ".*DELEGATING,.*" 1 "PROCESSES,.*" 1 ".*LOCATE,.*" 1 ".*DRIVERS.*" 1 < sudoers'
test '2 "[^:]*-[^:]*:.*" 1 ".*:4.*" 1 < group'
test '1 "[^:]*-[^:]*:.*" 1 ".*:4.*" 1 < group'
test '2 "[^:]*-[^:]*:.*" 1 ".*:4.*" 1 ".*journal.*" -5 < group'
test '2 "[^:]*-[^:]*:.*" 1 ".*:4.*" 1 ".*journal.*" -5 "root.*" 5 < group'
test '3 "Ahoj.*" 1 "[0-9][0-9]* .*" 2 "[^0-9][^0-9]* .*" 3 < example'
test '2 ".*pgrep.*" 1 ".*pthread" 1 < zadani.txt'
test '2 ".*pgrep.*" 1 ".*pthread.*" 1 < zadani.txt'
test '1 ".*pgrep.*" 1 ".*pthread.*" -1 < zadani.txt'
