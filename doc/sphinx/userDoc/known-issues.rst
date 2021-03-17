.. _issues:

Known Issues
============

IOR 3.3.0
---------

If attempting to read past the end of a file, IOR will return the following::

    ior ERROR: read(37, 0x2aaac5419000, 33554432) returned EOF prematurely

which will be followed by a spurious errno and strerror.  See `#346 <https://github.com/hpc/ior/issues/346>`_.
