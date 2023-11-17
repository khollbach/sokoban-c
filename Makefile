start_addr := 2000

default: dsk

build: clean
	mkdir -p out/
	cl65 -t apple2 --start-addr 0x$(start_addr) -Wl -D__EXEHDR__=0 main.c -o out/main

wav: build
	c2t -bc8 out/main,$(start_addr) out/main.wav

dsk: build
	cp "disk-images/ProDOS 8.dsk" out/main.dsk
	for f in launcher sysutil fastcopy basic; do ac -d out/main.dsk $$f.system; done
	ac -p out/main.dsk main.system sys 0x$(start_addr) < out/main

clean:
	rm -rf out/
