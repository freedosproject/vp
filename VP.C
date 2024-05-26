#include <stdio.h>
#include <stdlib.h>
#include <conio.h> /* getch */
#include <graph.h> /* graphics */
#include <i86.h> /* delay */

typedef unsigned char byte_t;

typedef enum {false,true} bool;

/* global structure to define the printer */

struct {
    unsigned long pxwidth;
    unsigned long pxheight;

    unsigned long xpos;
    unsigned long ypos;

    byte_t *ascii;

    bool is_underline;
    bool is_graphics;
    bool is_unix;
} Printer;

/* uses OpenWatcom C */

int pause(void)
{
    int key;

    /* draw a line to alert the user */

    _setcolor(7); /* white */
    _moveto(Printer.pxwidth-1, 0);
    _lineto(Printer.pxwidth-1, Printer.pxheight-1);

    /* read keystroke */

    key = getch();
    if (key == 0) {
	/* ext key .. read getch again for ext value */
	getch(); /* ignore ext value */
    }

    /* returning 0 for ext key is okay here */
    return key;
}

void new_page(void)
{
    /* clear the screen */

    _setbkcolor(0); /* black */
    _clearscreen(_GCLEARSCREEN);

    /* draw an empty page */

    _setcolor(15); /* br white */
    _rectangle(_GFILLINTERIOR, 0,0, Printer.pxwidth-1, Printer.pxheight-1);

    /* set the "print head" to "home" */

    Printer.xpos = 0;
    Printer.ypos = 0;
}

void new_line(void)
{
    /* new line is 8px in graphics mode, 10px in text mode */
    if (Printer.is_graphics == true) {
	Printer.ypos += 8;
    }
    else {
	Printer.ypos += 10;
    }

    /* add CR if Unix mode */
    if (Printer.is_unix == true) {
	Printer.xpos = 0;
    }

    /* if we reach the end of the page, start a new page */

    if (Printer.pxheight - Printer.ypos < 10) {
	pause();
	new_page();
    }
}

void print_byte(byte_t b)
{
    int off;

    /* print a vertical bit pattern at the current xpos. the "1"
	bit is on bottom .. the "128" bit is on top. */
    /* .. but we don't need to do this if printing "0" (empty) */

    if (b) {
	_setcolor(1); /* blue */

	for (off = 0; off < 8; off++) {
	    if (b & (1<<off)) {
		/* ypos describes the "top" of the "print head" so we
		    need to do some math to print pixels correctly */
		_setpixel(Printer.xpos, Printer.ypos + (7-off));
	    }
	}

	/* if you want to artificially slow down the program, do that
	    here after printing something: */

	delay(1); /* milliseconds .. see i86.h */
    }

    /* only if text mode, add underline at 9th line */
    if ((Printer.is_graphics == false) && (Printer.is_underline == true)) {
	_setcolor(2); /* green */
	_setpixel(Printer.xpos, Printer.ypos+8);
    }

    /* advance the print head by 1px */
    /* if we reached the end of the line, wrap to the next line */

    if (++Printer.xpos > Printer.pxwidth) {
	new_line();
	Printer.xpos = 0;
    }
}

void print_char(int ch)
{
    /* print a character by referencing the 8x5 bit pattern
	from firmware. every char is defined by 5 columns
	of bit patterns. use print_byte to print it. */

    int i;

    /* element 0 in the Printer.ascii array is ascii 32, and
	the printable ascii elements are 32 .. 127 */

    if (ch < 32) {
	/* control codes */
	switch(ch) {
	case '\r': /* CR */
	    Printer.xpos = 0;
	    break;
	case '\n': /* NL */
	    new_line();
	    break;
	/* add others here */
	}
    }
    else { /* 32 or greater */
	if (ch > 127) {
	    /* extended ascii .. these are 8x6 chars, no space */
	    /* fake it .. print a 6-px box */

	    for (i = 0; i < 6; i++) {
		print_byte(255); /* filled */
	    }
	}
	else { /* 32 to 127 */
	    /* printable ascii .. these are 8x5 + 1px space*/
	    /* retrieve the bit-patterns from "firmware" */

	    for (i = 0; i < 5; i++) {
		print_byte(Printer.ascii[((ch-32)*5)+i]);
	    }

	    /* add a space between letters */
	    print_byte(0); /* space */
	}
    }
}

void print_file(FILE *in)
{
    int ch;

    while ((ch = fgetc(in)) != EOF) {
	print_char(ch);
    }
}

void test_printer(void)
{
    /* generate a printer test */

    int i;

    for (i = 32; i <= 127; i++) {
	print_char(i);
    }
}

int init_printer(void)
{
    int asc, line;

    /* define US Letter .. if 60 dpi, then:
	8 1/2 wide x 60 dpi = 510
	11 tall x 60 dpi = 660

	.. so US Letter is 510x660
    */

    Printer.pxwidth = 510;
    /* Printer.pxheight = 480; /* short page .. for 640x480 mode */
    Printer.pxheight = 660; /* for 1024x768 */

    /* set home position for first page */

    Printer.xpos = 0;
    Printer.ypos = 0;

    /* set printer defaults */

    Printer.is_underline = false;
    Printer.is_graphics = false;
    Printer.is_unix = false;

    /* allocate memory for firmware */
    /* low ascii is 32 to 127 .. that's (127-32)+1 = 96 chars
	.. and each is defined by 5 8-bit patterns */

    Printer.ascii = malloc(sizeof(byte_t) * 96 * 5);

    if (Printer.ascii == NULL) {
	/* fail */
	return 0;
    }

    /* load ascii chars into firmware .. if we had a char set
	defined, we'd load it here. since we don't, we'll fake
	it by loading bit patterns into each */

    for (asc = 32; asc <= 127; asc++) {
	for (line = 0; line < 5; line++) {
	    if (asc == 32) {
		Printer.ascii[line] = 0; /* space */
	    }
	    else {
		Printer.ascii[((asc-32)*5)+line] = asc;
	    }
	}
    }

    return (asc-32); /* number of ascii values entered */
}

void end_printer(void)
{
    /* free memory */
    free(Printer.ascii);
}

int main(int argc, char **argv)
{
    int i;
    FILE *in;

    /* init printer */

    if (init_printer() < 1) {
	puts("cannot load firmware");
	return 1;
    }

    /* set video */

    /* if (_setvideomode(_VRES16COLOR) == 0) { /* 640x480 */
    if (_setvideomode(_XRES256COLOR) == 0) { /* 1024x768 */
	puts("cannot set video mode");
	end_printer();
	return 2;
    }

    /* print files */

    new_page();

/*
    test_printer();
    pause();
*/

    for (i = 1; i < argc; i++) {
	in = fopen(argv[i], "rb");

	if (in) {
	    print_file(in);
	    fclose(in);
	}
    }

    /* done */

    pause();

    _setvideomode(_DEFAULTMODE);
    end_printer();

    return 0;
}
