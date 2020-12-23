.. _scripting:

Scripting
=========

IOR can use an input script with the command line using the ``-f`` option.
**Any options on the command line set before the '-f' option is given will be
considered the default settings for running the script.**  For example, ::

    mpirun ./ior -W -f script

will run all tests in the script with an implicit ``-W``.  The script itself can
override these settings and may be set to run many different tests of IOR under
a single execution, and it is important to note that **any command-line options
specified after ``-f`` will not be applied to the runs dictated by the script.**
For example, ::

    mpirun ./ior -f script -W

will *not* run any tests with the implicit ``-W`` since that argument does not
get applied until after the ``-f`` option (and its constituent runs) are complete.

Input scripts are specified using the long-form option names that correspond to
each command-line option described in the :ref:`Options` page.  In addition to
long-form options,

    * ``IOR START`` and ``IOR END`` mark the beginning and end of the script
    * ``RUN`` dispatches the test using all of the options specified before it
    * All previous set parameter stay set for the next test. They are not reset
      to the default! For default the must be rest manually.
    * White space is ignored in script, as are comments starting with ``#``.
    * Not all test parameters need be set.

An example of a script: ::

  IOR START
      api=posix
      testFile=testFile
      hintsFileName=hintsFile
      repetitions=8
      interTestDelay=5
      readFile=1
      writeFile=1
      filePerProc=0
      checkWrite=0
      checkRead=0
      quitOnError=0
      segmentCount=1
      blockSize=32k
      verbose=0
      numTasks=32
      fsync=1
      posix.odirect=0
      showHints=0
      showHelp=0
  RUN
  # additional tests are optional
      transferSize=64
      blockSize=64k
      segmentcount=2
  RUN
      transferSize=4K
      blockSize=1M
      segmentcount=1024
  RUN
  IOR STOP
