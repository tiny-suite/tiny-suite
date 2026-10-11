SUBDIRS = tinytile tinyctl tinyfetch tinytop
CTL = tinyctl/tinyctl

all:
	@fail=0; for d in $(SUBDIRS); do \
		echo "== $$d =="; \
		$(MAKE) -C $$d || fail=1; \
	done; exit $$fail

clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done

install:
	@fail=0; for d in $(SUBDIRS); do \
		echo "== $$d =="; \
		$(MAKE) -C $$d install || fail=1; \
	done; exit $$fail

list: $(CTL)
	@$(CTL) list

tui: $(CTL)
	@$(CTL) tui

$(CTL):
	@$(MAKE) -C tinyctl

.PHONY: all clean install list tui
