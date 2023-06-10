CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lX11 -lXext -lXfixes -lXrandr

kiosk-wm: kiosk-wm.o
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

kiosk-wm.o: kiosk-wm.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f kiosk-wm kiosk-wm.o
