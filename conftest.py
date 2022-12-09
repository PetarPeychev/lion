"""Pytest configuration."""

import logging


def pytest_configure(config):  # pylint: disable=unused-argument
    logging.getLogger("flake8").setLevel(logging.ERROR)
