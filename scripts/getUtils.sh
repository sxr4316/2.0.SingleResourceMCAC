#!/bin/bash

# Downloads some useful utilities

function addPath() {
    if [ -d "$1" ] && [[ ":$PATH:" != *":$1:"* ]]; then
        PATH="${PATH:+"$PATH:"}$1"
        echo "" >> ~/.bashrc
        echo "PATH=\"\$PATH:$1\"" >> ~/.bashrc
    fi
}

mkdir -p ~/bin
addPath "~/bin"

if [ ! -f ~/bin/dos2unix ]; then
	cd ~/Downloads/
	wget http://heanet.dl.sourceforge.net/project/dos2unix/dos2unix/7.3.3/dos2unix-7.3.3.tar.gz
	tar -xzf dos2unix-7.3.3.tar.gz
	rm dos2unix-7.3.3.*
	cd dos2unix-7.3.3
	make
	mv dos2unix ~/bin/
	cd ..
	rm -rf dos2unix-7.3.3/
fi

if [ ! -f ~/bin/subl ]; then
	cd ~/Downloads/
	wget https://download.sublimetext.com/sublime_text_3_build_3103_x64.tar.bz2
	bzip2 -df sublime_text_3_build_3103_x64.tar.bz2
	tar -xf sublime_text_3_build_3103_x64.tar
	rm sublime_text_3_build_3103_x64.tar
	mkdir -p ~/Applications
	mv sublime_text_3 ~/Applications/
	cd ~/bin
	if [ -e ~/bin/subl ]; then
		rm -f ~/bin/subl
	fi
	ln -s ~/Applications/sublime_text_3/sublime_text subl
fi
