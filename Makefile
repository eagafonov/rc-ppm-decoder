CXX:=$(V2R_SDK_ROOT)/codesourcery/arm-2013.05/bin/arm-none-linux-gnueabi-g++

CFLAGS:=\
	-I$(ARDUPILOT_ROOT)/libraries/GCS_MAVLink/include


all: upload

rc-decode: Makefile rc-decode.cpp
	$(CXX) $(CFLAGS) -Wall -Werror -O2 -o rc-decode rc-decode.cpp

clean:
	# rm -f gpio-event
	rm -f rc-decode

upload:  rc-decode
	scp rc-decode v2r: