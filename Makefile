# Top level makefile for Gammu
#  - wraps CMake build as used by configure
#  - provides some operations which can be done on source

BUILD_DIR=build-configure

.PHONY: all clean package package_source apidoc interndoc translations gammu install distclean test NightlyMemoryCheck Nightly

all: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR)

clean: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) clean

package: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) package

package_source: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) package_source

apidoc: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) apidoc

manual-html: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) manual-html

manual-ps: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) manual-ps

manual-latex: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) manual-latex

manual-man: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) manual-man

manual-htmlhelp: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) manual-htmlhelp

manual-pdf: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) manual-pdf

cppcheck: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) cppcheck

interndoc: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) interndoc

translations: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) translations

gammu: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) gammu

install: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) install

test: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) test ARGS=--output-on-failure

Nightly: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) Nightly

NightlyMemoryCheck: $(BUILD_DIR)
	$(MAKE) -C $(BUILD_DIR) NightlyMemoryCheck

TAGFILESFIND = find gammu libgammu include smsd helper tests gammu-detect -name '*.[ch]' -o -name '*.cpp'

tags: $(shell $(TAGFILESFIND))
	@echo Generating tags file...
	@ctags $(shell $(TAGFILESFIND))

distclean:
	rm -rf build*
	rm -f tags

$(BUILD_DIR):
	@echo 'You should run configure before running make!'
	@exit 1

# Developer commands

update-man: $(BUILD_DIR)
	@make -C $(BUILD_DIR) manual-man
	@cp $(BUILD_DIR)/docs/manual/man/*.* docs/man/

check-man:
	@for f in docs/man/*.[0-9] ; do echo $$f: ; LANG=en_US.UTF-8 MANWIDTH=80 man --warnings -E UTF-8 -l $$f >/dev/null ; done
