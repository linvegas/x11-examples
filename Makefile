all: 00-basic 01-poll-events 02-drawing 03-gl-context

00-basic: 00-basic.c
	cc -ggdb -Wall -Wextra 00-basic.c -o 00-basic -lX11

01-poll-events: 01-poll-events.c
	cc -ggdb -Wall -Wextra 01-poll-events.c -o 01-poll-events -lX11

02-drawing: 02-drawing.c
	cc -ggdb -Wall -Wextra 02-drawing.c -o 02-drawing -lX11 -lm

03-gl-context: 03-gl-context.c
	cc -ggdb -Wall -Wextra 03-gl-context.c -o 03-gl-context -lX11 -lGL
