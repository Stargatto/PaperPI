# PaperPI

A fantastic tool to transform a Waveshare e-Paper Display + a Raspberry Pi Zero W in a 10.3inch e-Book Reader.
Proudly made during 2020 lockdown.

## Description
PaperPI works as a PDF viewer for Waveshare e-Paper Display (controlled by the IT8951 board):

1. It scans the directory (specified as argument) for PDF files.
1. Once selected, the PDF is shown on the e-ink screen. The pages are scaled to maximize the area.
1. The user can turn the pages backward and forward as a normal e-book reader.
1. After reading, the user can go to the main screen and then exit.

All PDF pages are converted in PBM images using Poppler and then rendered on the IT8951 framebuffer.
The code is optimized in order to:
* cache the next and previous pages in memory just after a page is turn;
* minimize the throughput to the IT8951 using only 4-bit for a single pixel.

The user can control PaperPI using three pysichal buttons connected with RaspberryPI GPIOs 8, 10, 12.

The main screen is a bit bulky. It shows a list of the PDF files ordered from _1_ to _n_.
The selection of the _n_ th file is made pressing the forward button _n_ times.

For my prototype I used a Raspberry Pi Zero W board with Arch Linux ARM.

### User Inputs

Being in pull-up configuration, when the button is pressed the signal goes from HIGH to LOW.
The output is set to 1 when a transition 0->1 occours:
        
        signal: 11111111111000011111111111111111
        output: 00000000000000010000000000000000
                               ^

| pin2 | pin1 | pin0 | Usage | # |
|------|------|------|-------|---|
|0|0|0| None | 0 |
|0|0|1| Forward | 1 |
|0|1|0| Select/Exit from page | 2 |
|0|1|1| Forward + 10 pages | 3 |
|1|0|0| Backward | 4 |
|1|0|1| Rotate | 5 |
|1|1|0| Backward + 10 | 6 |
|1|1|1| Exit from PaperPI | 7 |

## Getting Started

### Dependencies

* [Poppler](https://poppler.freedesktop.org/)
* [bcm2835 C library](https://www.airspayce.com/mikem/bcm2835/index.html)

### Compiling

You have to set the correnct _VCOM_ parameter in IT8951.h before compiling PaperPI.

`$ make`

### Executing program

`$ ./paperpi DIRECTORY`

## Caveats
I'm not a professional programmer, so there may be bugs or memory holes.
But it works...

## Authors

* [Stargatto](https://github.com/stargatto)

## Credits
* Waveshare
	* [IT8951](https://github.com/waveshare/IT8951)
* aligrudi
	* [fbpdf](https://github.com/aligrudi/fbpdf)

Thanks to [naluhh](https://github.com/naluhh) for some interesting suggestions and improvements to IT8951 libraries.

## Version History

* v0.7
    * Finally on GitHub

## License

This project is licensed under the GNU GPL v2.0