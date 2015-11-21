#include <kernel.h>
#include "../include/kernel.h"

BOOL valid_cursor_x(WINDOW* wnd, int x) {
    return ((0 <= x) && (x < wnd->width));
}

BOOL valid_cursor_y(WINDOW* wnd, int y) {
    return ((0 <= y) && (y < wnd->height));
}

BOOL valid_cursor_position(WINDOW* wnd, int x, int y) {
    return (valid_cursor_x(wnd, x) && valid_cursor_y(wnd, y));
}

void move_cursor(WINDOW* wnd, int x, int y) {
    if(valid_cursor_position(wnd, x, y)) {
        wnd->cursor_x = x;
        wnd->cursor_y = y;
    }
}

MEM_ADDR get_cursor_address(WINDOW* wnd) {
    return (MEM_ADDR) (0xB8000 + (2 * (wnd->x + wnd->cursor_x)) + (160 * (wnd->y + wnd->cursor_y)));
}

MEM_ADDR get_row_address(WINDOW* wnd, int row) {
    return (MEM_ADDR) (0xB8000 + (2 * wnd->x) + (160 * (wnd->y + row)));
}

void cursor_display(WINDOW *wnd, unsigned char c) {
    MEM_ADDR cursor_addr = get_cursor_address(wnd);
    poke_b(cursor_addr, c);
    poke_b(cursor_addr + 1, 0x0F);
}

void remove_cursor(WINDOW* wnd) {
    cursor_display(wnd, ' ');
}

void show_cursor(WINDOW* wnd) {
    cursor_display(wnd, wnd->cursor_char);
}

void clear_window(WINDOW* wnd) {
    int row;
    for(row = 0; row < wnd->height; row++){
        MEM_ADDR current_row = get_row_address(wnd, row);
        memset_b(current_row, 0x00, (wnd->width * 2) - 1);
    }
    move_cursor(wnd, 0, 0);
}

BOOL can_cursor_new_line(WINDOW* wnd) {
    return (wnd->cursor_y + 1 < wnd->height);
}

BOOL can_cursor_advance(WINDOW* wnd) {
    return (wnd->cursor_x + 1 < wnd->width);
}

void advance_cursor(WINDOW* wnd) {
    move_cursor(wnd, wnd->cursor_x + 1, wnd->cursor_y);
}

void cursor_new_line(WINDOW* wnd) {
    move_cursor(wnd, 0, wnd->cursor_y + 1);
}

void clear_last_row(WINDOW* wnd) {
    MEM_ADDR last_row = get_row_address(wnd, wnd->height - 1);
    memset_b(last_row, 0x00, (wnd->width * 2) - 1);
}

void scroll_window(WINDOW* wnd) {
    int row;
    for(row = 0; row < (wnd->height - 1); row++){
        MEM_ADDR old_line = get_row_address(wnd, row);
        MEM_ADDR new_line = get_row_address(wnd, row + 1);
        k_memcpy((void*) old_line, (void*) new_line, (wnd->width) * 2);
    }

    clear_last_row(wnd);
    move_cursor(wnd, 0, wnd->cursor_y);
}

void output_char(WINDOW* wnd, unsigned char c) {
    MEM_ADDR cursor_addr = get_cursor_address(wnd);
    if(c == '\n') {
        poke_b(cursor_addr, 0x00);
        poke_b(cursor_addr + 1, 0x0F);
    } else {
        poke_b(cursor_addr, c);
        poke_b(cursor_addr + 1, 0x0F);
    }

    if(can_cursor_advance(wnd) && (c != '\n' )) {
        advance_cursor(wnd);
    } else if(can_cursor_new_line(wnd)) {
        cursor_new_line(wnd);
    } else {
        scroll_window(wnd);
    }
}

void output_string(WINDOW* wnd, const char *str) {
	volatile int lock;
    DISABLE_INTR(lock);
    int length = k_strlen(str);
    while(length > 0) {
        char c = *str;
        output_char(wnd, c);
        str++;

        length--;
    }
    ENABLE_INTR(lock);
}


/*
 * There is not need to make any changes to the code below,
 * however, you are encouraged to at least look at it!
 */
#define MAXBUF (sizeof(long int) * 8)		 /* enough for binary */

char *printnum(char *b, unsigned int u, int base,
	       BOOL negflag, int length, BOOL ladjust,
	       char padc, BOOL upcase)
{
    char	buf[MAXBUF];	/* build number here */
    char	*p = &buf[MAXBUF-1];
    int		size;
    char	*digs;
    static char up_digs[] = "0123456789ABCDEF";
    static char low_digs[] = "0123456789abcdef";
    
    digs = upcase ? up_digs : low_digs;
    do {
	*p-- = digs[ u % base ];
	u /= base;
    } while( u != 0 );
    
    if (negflag)
	*b++ = '-';
    
    size = &buf [MAXBUF - 1] - p;
    
    if (size < length && !ladjust) {
	while (length > size) {
	    *b++ = padc;
	    length--;
	}
    }
    
    while (++p != &buf [MAXBUF])
	*b++ = *p;
    
    if (size < length) {
	/* must be ladjust */
	while (length > size) {
	    *b++ = padc;
	    length--;
	}
    }
    return b;
}


/*
 *  This version implements therefore following printf features:
 *
 *	%d	decimal conversion
 *	%u	unsigned conversion
 *	%x	hexadecimal conversion
 *	%X	hexadecimal conversion with capital letters
 *	%o	octal conversion
 *	%c	character
 *	%s	string
 *	%m.n	field width, precision
 *	%-m.n	left adjustment
 *	%0m.n	zero-padding
 *	%*.*	width and precision taken from arguments
 *
 *  This version does not implement %f, %e, or %g.  It accepts, but
 *  ignores, an `l' as in %ld, %lo, %lx, and %lu, and therefore will not
 *  work correctly on machines for which sizeof(long) != sizeof(int).
 *  It does not even parse %D, %O, or %U; you should be using %ld, %o and
 *  %lu if you mean long conversion.
 *
 *  This version implements the following nonstandard features:
 *
 *	%b	binary conversion
 *
 */


#define isdigit(d) ((d) >= '0' && (d) <= '9')
#define ctod(c) ((c) - '0')


void vsprintf(char *buf, const char *fmt, va_list argp)
{
    char		*p;
    char		*p2;
    int			length;
    int			prec;
    int			ladjust;
    char		padc;
    int			n;
    unsigned int        u;
    int			negflag;
    char		c;
    
    while (*fmt != '\0') {
	if (*fmt != '%') {
	    *buf++ = *fmt++;
	    continue;
	}
	fmt++;
	if (*fmt == 'l')
	    fmt++;	     /* need to use it if sizeof(int) < sizeof(long) */
	
	length = 0;
	prec = -1;
	ladjust = FALSE;
	padc = ' ';
	
	if (*fmt == '-') {
	    ladjust = TRUE;
	    fmt++;
	}
	
	if (*fmt == '0') {
	    padc = '0';
	    fmt++;
	}
	
	if (isdigit (*fmt)) {
	    while (isdigit (*fmt))
		length = 10 * length + ctod (*fmt++);
	}
	else if (*fmt == '*') {
	    length = va_arg (argp, int);
	    fmt++;
	    if (length < 0) {
		ladjust = !ladjust;
		length = -length;
	    }
	}
	
	if (*fmt == '.') {
	    fmt++;
	    if (isdigit (*fmt)) {
		prec = 0;
		while (isdigit (*fmt))
		    prec = 10 * prec + ctod (*fmt++);
	    } else if (*fmt == '*') {
		prec = va_arg (argp, int);
		fmt++;
	    }
	}
	
	negflag = FALSE;
	
	switch(*fmt) {
	case 'b':
	case 'B':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 2, FALSE, length, ladjust, padc, 0);
	    break;
	    
	case 'c':
	    c = va_arg (argp, int);
	    *buf++ = c;
	    break;
	    
	case 'd':
	case 'D':
	    n = va_arg (argp, int);
	    if (n >= 0)
		u = n;
	    else {
		u = -n;
		negflag = TRUE;
	    }
	    buf = printnum (buf, u, 10, negflag, length, ladjust, padc, 0);
	    break;
	    
	case 'o':
	case 'O':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 8, FALSE, length, ladjust, padc, 0);
	    break;
	    
	case 's':
	    p = va_arg (argp, char *);
	    if (p == (char *)0)
		p = "(NULL)";
	    if (length > 0 && !ladjust) {
		n = 0;
		p2 = p;
		for (; *p != '\0' && (prec == -1 || n < prec); p++)
		    n++;
		p = p2;
		while (n < length) {
		    *buf++ = ' ';
		    n++;
		}
	    }
	    n = 0;
	    while (*p != '\0') {
		if (++n > prec && prec != -1)
		    break;
		*buf++ = *p++;
	    }
	    if (n < length && ladjust) {
		while (n < length) {
		    *buf++ = ' ';
		    n++;
		}
	    }
	    break;
	    
	case 'u':
	case 'U':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 10, FALSE, length, ladjust, padc, 0);
	    break;
	    
	case 'x':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 16, FALSE, length, ladjust, padc, 0);
	    break;
	    
	case 'X':
	    u = va_arg (argp, unsigned int);
	    buf = printnum (buf, u, 16, FALSE, length, ladjust, padc, 1);
	    break;
	    
	case '\0':
	    fmt--;
	    break;
	    
	default:
	    *buf++ = *fmt;
	}
	fmt++;
    }
    *buf = '\0';
}



void wprintf(WINDOW* wnd, const char *fmt, ...)
{
    va_list	argp;
    char	buf[160];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    output_string(wnd, buf);
    va_end(argp);
}




static WINDOW kernel_window_def = {0, 0, 80, 25, 0, 0, ' '};
WINDOW* kernel_window = &kernel_window_def;


void kprintf(const char *fmt, ...)
{
    va_list	  argp;
    char	  buf[160];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    output_string(kernel_window, buf);
    va_end(argp);
}


