# IOR and mdtest Documentation

## Building

To build the documentation in this directory,

    sphinx-build . _build_html

The output will be saved as html in `_build_html/`.

## Prerequisites

The simplest way to build these documents locally is using Anaconda.

    conda create -n sphinx sphinx graphviz sphinx_rtd_theme
    conda activate sphinx
    sphinx-build . _build_html

To build Doxygen documentation, Graphviz (the `dot` command) is required.
