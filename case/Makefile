# Build the case STLs and preview images.
#
#   make            render every STL (both display sizes)
#   make png        render preview images (needs OpenGL; CI uses xvfb)
#   make clean
#
# Override the binary with:  make OPENSCAD=/path/to/openscad

SHELL := /bin/bash

ifndef OPENSCAD
OPENSCAD := $(shell command -v openscad 2>/dev/null)
ifeq ($(OPENSCAD),)
OPENSCAD := /Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD
endif
endif

DISPLAYS := 3.5 4.0
ORIENTS  := portrait landscape
PARTS    := shell bezel stand
VARIANTS := $(foreach o,$(ORIENTS),$(foreach d,$(DISPLAYS),$(o)-$(d)))
STLS     := $(foreach v,$(VARIANTS),$(foreach p,$(PARTS),stl/$(p)-$(v).stl))
PNGS     := $(foreach v,$(VARIANTS),png/front-$(v).png png/exploded-$(v).png png/stand-$(v).png)
SOURCES  := $(wildcard *.scad)

# Headless OpenGL for the PNGs when there is no display (CI).
XVFB := $(shell if [ -z "$$DISPLAY" ] && command -v xvfb-run >/dev/null 2>&1; then echo "xvfb-run -a"; fi)

.PHONY: all stl png clean
all: stl
stl: $(STLS)
png: $(PNGS)

# stl/<part>-<orientation>-<display>.stl
# Fails if OpenSCAD reports any warning (bad geometry, empty result, ...)
# or if the STL contains no triangles.
stl/%.stl: $(SOURCES)
	@mkdir -p stl
	@set -e; part=$(word 1,$(subst -, ,$*)); orient=$(word 2,$(subst -, ,$*)); disp=$(word 3,$(subst -, ,$*)); \
	echo "==> $@"; \
	$(OPENSCAD) -o $@ -D "part=\"$$part\"" -D "orientation=\"$$orient\"" -D "display=\"$$disp\"" case.scad > $@.log 2>&1 \
	  || { cat $@.log; exit 1; }; \
	if grep -qiE 'warning|error' $@.log; then cat $@.log; echo "OpenSCAD warnings in $@"; exit 1; fi; \
	if ! grep -q facet $@; then echo "$@ is empty"; exit 1; fi; \
	rm -f $@.log

png/front-%.png: $(SOURCES)
	@mkdir -p png
	$(XVFB) $(OPENSCAD) -o $@ --imgsize=1100,1100 --viewall --autocenter --projection=o \
	  --camera=0,0,0,0,0,0,0 -D 'part="assembly"' -D 'orientation="$(word 1,$(subst -, ,$*))"' -D 'display="$(word 2,$(subst -, ,$*))"' case.scad

png/exploded-%.png: $(SOURCES)
	@mkdir -p png
	$(XVFB) $(OPENSCAD) -o $@ --imgsize=1400,1300 --viewall --autocenter \
	  --camera=0,0,0,70,0,320,0 -D 'part="assembly"' -D 'orientation="$(word 1,$(subst -, ,$*))"' -D 'display="$(word 2,$(subst -, ,$*))"' -D 'exploded=60' case.scad

png/stand-%.png: $(SOURCES)
	@mkdir -p png
	$(XVFB) $(OPENSCAD) -o $@ --imgsize=1100,1100 --viewall --autocenter \
	  --camera=0,0,0,70,0,330,0 -D 'part="assembly"' -D 'orientation="$(word 1,$(subst -, ,$*))"' -D 'display="$(word 2,$(subst -, ,$*))"' -D 'with_stand=true' case.scad

clean:
	rm -rf stl png
