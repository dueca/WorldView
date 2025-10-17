# Where are we??
CNFPATH := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

MASKFILES = hmilabmaskleft-blur.png hmilabmaskright-blur.png hmilabmaskfront-blur.png
CONVERT := /usr/bin/magick
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