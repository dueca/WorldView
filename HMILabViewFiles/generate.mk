# Where are we??
CNFPATH := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

MASKFILES = hmilabmaskleft-blur.png hmilabmaskright-blur.png hmilabmaskfront-blur.png

ifeq (, $(shell which magick))
ifeq (, $(shell which convert))
$(error "Missing magick and convert install one of these")
else
CONVERT := $(shell which convert)
endif
else
CONVERT := $(shell which magick)
endif

MASKOPTS = --export-area-page

# default target
all: ${MASKFILES}

clean:
	rm -f ${MASKFILES}

%-blur.png: %-sharp.png
	${CONVERT} ${<} -channel RGBA -blur 0x5 -gamma 0.45 png32:${@}

%-sharp.png:
	inkscape --export-filename=${@} \
		${MASKOPTS} ${CNFPATH}/${@:-sharp.png=.svg}