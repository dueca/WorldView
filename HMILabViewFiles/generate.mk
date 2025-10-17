# Where are we??
CNFPATH := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))

MASKFILES = hmilabmaskleft-blur.png hmilabmaskright-blur.png hmilabmaskfront-blur.png
TESTFILES = hmilabmaskleft-test.png hmilabmaskright-test.png hmilabmaskfront-test.png

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
TESTVIEWACTIONS = select-clear; select-by-id:aligner; object-set-attribute:style, display:inline; \
									select-clear; select-by-id:testbar; object-set-attribute:style, display:inline


# default target
all: ${MASKFILES}
alltest: ${TESTFILES}

clean:
	rm -f ${MASKFILES} ${TESTFILES}

%-blur.png: %-sharp.png
	${CONVERT} ${<} -channel RGBA -blur 0x5 -gamma 0.45 png32:${@}

%-sharp.png:
	inkscape --export-filename=${@} \
		${MASKOPTS} ${CNFPATH}/${@:-sharp.png=.svg}

%-test.png:
	inkscape --export-filename=${@} \
		--actions "${TESTVIEWACTIONS}" ${MASKOPTS}  ${CNFPATH}/${@:-test.png=.svg}

%-blur.png: %-sharp.png
	${CONVERT} ${<} -channel RGBA -blur 0x5 -gamma 0.45 png32:${@}