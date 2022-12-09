#!/usr/bin/env bash

# Generate html docs using pdoc.
pdoc3 --html --force --output-dir docs lion
mv ./docs/lion/* ./docs/
rmdir ./docs/lion
