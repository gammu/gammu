all: setup.py
	python setup.py build

clean: setup.py
	python setup.py clean

install: setup.py
	python setup.py install

dist: setup.py
	python setup.py sdist --formats=gztar,bztar,zip

.PHONY: all clean install dist
