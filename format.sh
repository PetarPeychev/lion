#!/usr/bin/env bash

# Format code with yapf.
# yapf -i -r -p -vv setup.py conftest.py sageleaf/ test/

# Format code with black.
black .

# Format docstrings with docformatter.
docformatter -i -r setup.py conftest.py lion/ test/