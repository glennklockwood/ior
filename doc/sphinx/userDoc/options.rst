.. _options:

Options
================================================================================

IOR's runtime behavior can be configured entirely using the command line or
using a command file.  All supported options can be viewed by running IOR with
the ``--help`` command-line argument (MPI is not required), and all options are
either flags (which take a true or false value) or an argument (which take a
string or numeric argument).

* For transfer and block sizes, the case-insensitive K, M, and G suffices are
  recognized.  I.e., '4k' or '4K' is accepted as 4096.
* To specify a flag in an IOR command file, you must specify values; [1]=true,
  [0]=false.
* All directives are case-insensitive.
* Long args are supported on the command line using double dashes.  For example,
  using ``-c`` is equivalent to ``--collective``.  Module-specific options are
  the same (e.g., ``--posix.odirect``).

For more information on using IOR with a command file, see :ref:`scripting`.


Supported flags
--------------------------------------------------------------------------------

=========  ====================   ==============================================================================
Short arg  Long arg               Description
=========  ====================   ==============================================================================
  -c       collective             collective I/O
  -C       reorderTasks           changes task ordering for readback (useful to avoid client cache)
  -e       fsync                  perform a fsync() operation at the end of each read/write phase
  -E       useExistingTestFile    do not remove test file before write access
  -F       filePerProc            file-per-process
  -g       intraTestBarriers      use barriers between open, write/read, and close
  -H       showHints              show hints
  -I       individualDataSets     datasets not shared by all procs [not working]
  -k       keepFile               don't remove the test file(s) on program exit
  -K       keepFileWithError      keep error-filled file(s) after data-checking
  -m       multiFile              use number of reps (-i) for multiple file count
  -n       noFill                 no fill in HDF5 file creation
  -p       preallocate            preallocate file size
  -P       useSharedFilePointer   use shared file pointer [not working]
  -q       quitOnError            during file error-checking, abort on error
  -r       readFile               read existing file
  -R       checkRead              verify that the output of read matches the expected signature (used with -G)
  -S       useStridedDatatype     put strided access into datatype [not working]
  -u       uniqueDir              use unique directory name for each file-per-process
  -v       verbose                output information (repeating flag increases level)
  -V       useFileView            use MPI_File_set_view
  -w       writeFile              write file
  -W       checkWrite             check read after write
  -x       singleXferAttempt      do not retry transfer if incomplete
  -Y       fsyncPerWrite          perform sync operation after every write operation
  -z       randomOffset           access is to random, not sequential, offsets within a file
  -Z       reorderTasksRandom     changes task ordering to random ordering for readback
  N/A      dryRun                 do not perform any I/Os just run evtl. inputs print dummy output
=========  ====================   ==============================================================================




Supported arguments
--------------------------------------------------------------------------------

=========  =============================  =======  ===========
Short arg  Long arg                       Type     Description
=========  =============================  =======  ===========
  -a       api                            string   which backend API to use (POSIX, MPIIO, etc)
  -A       refNum                         integer  user supplied reference number to include in the summary
  -b       blockSize                      integer  contiguous bytes to write per task  (e.g.: 8, 4k, 2m, 1g)
  -d       interTestDelay                 integer  delay between reps in seconds
  -D       deadlineForStonewalling        integer  seconds before stopping write or read phase
  N/A      stoneWallingWearOut            integer  once the stonewalling timout is over, all process finish to access the amount of data
  N/A      stoneWallingWearOutIterations  integer  stop after processing this number of iterations, needed for reading data back written with stoneWallingWearOut
  N/A      stoneWallingStatusFile         file     this file keeps the number of iterations from stonewalling during write and allows to use them for read
  -f       scriptFile                     file     test script name
  -G       setTimeStampSignature          integer  set value for time stamp signature/random seed
  -i       repetitions                    integer  number of repetitions of test
  -j       outlierThreshold               integer  warn on outlier N seconds from mean
  -J       setAlignment                   size     HDF5 alignment in bytes (e.g.: 8, 4k, 2m, 1g)
  -l       datapacket type                string   type of packet that will be created [offset|incompressible|timestamp|o|i|t]
  -M       memoryPerNode                  string   how much memory to hog on the node  (e.g.: 2g, 75%)
  -N       numTasks                       integer  number of tasks that are participating in the test (overrides MPI)
  -o       testFile                       file     full name for file to which I/O should be performed
  -O       N/A                            string   string of IOR directives (e.g. -O checkRead=1,lustreStripeCount=32)
  -Q       taskPerNodeOffset              integer  for read tests use with -C and -Z options (-C constant N, -Z at least N)
  -s       segmentCount                   integer  number of segments
  -t       transferSize                   size     size of transfer in bytes (e.g.: 8, 4k, 2m, 1g)
  -T       maxTimeDuration                integer  max time in minutes executing repeated test; it aborts only between iterations and not within a test!
  -U       hintsFileName                  file     full name for hints file
  -X       reorderTasksRandomSeed         integer  random seed for -Z option
  N/A      summaryFile                    file     store result data into this file
  N/A      summaryFormat                  string   format for outputing the summary (default, json, csv)
=========  =============================  =======  ===========




Module-specific options
--------------------------------------------------------------------------------

+------------+-------------------------+------------------------------------------------------+
| Module     | Long arg                | Description                                          |
+============+=========================+======================================================+
| POSIX      | posix.odirect           | open files with ``O_DIRECT``                         |
+------------+-------------------------+------------------------------------------------------+
| DUMMY      | dummy.delay-only-rank0  | Delay only rank 0                                    |
+            +-------------------------+------------------------------------------------------+
|            | dummy.delay-create=N    | Delay per create in usec                             |
+            +-------------------------+------------------------------------------------------+
|            | dummy.delay-xfer=N      | Delay per xfer in usec                               |
+------------+-------------------------+------------------------------------------------------+
| HDF5       | hdf5.collectivemetadata | Use collectiveMetadata (available since HDF5-1.10.0) |
+------------+-------------------------+------------------------------------------------------+
| MMAP       | mmap.madv_dont_need     | Use advise don't need                                |
|            | mmap.madv_pattern       | Use advise to indicate the pattern random/sequential |
+------------+-------------------------+------------------------------------------------------+




Flags and Arguments
--------------------------------------------------------------------------------

refNum
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
User-supplied reference number to be included in long summary.
(default: 0)

api
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Must be set to one of the supported IOR backends (e.g., POSIX,
MPIIO, HDF5, etc) (default: ``POSIX``)

testFile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Name of the output file [testFile].  With ``filePerProc`` set,
the tasks can round robin across multiple file names via ``-o S@S@S``.
If only a single file name is specified in this case, IOR appends the MPI
rank to the end of each file generated (e.g., ``testFile.00000059``)
(default: ``testFile``)

hintsFileName
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Name of the hints file (default: none)

repetitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Number of times to run each test (default: 1)

multiFile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Creates multiple files for single-shared-file or
file-per-process modes for each iteration (default: 0)

reorderTasksConstant
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Reorders tasks by a constant node offset for
writing/reading neighbor's data from different nodes (default: 0)

taskPerNodeOffset
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
For read tests. Use with ``-C`` and ``-Z`` options.
With ``reorderTasks``, constant N. With ``reordertasksrandom``, >= N
(default: 1)

reorderTasksRandom
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Reorders tasks to random ordering for read tests
(default: 0)

reorderTasksRandomSeed
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Random seed for ``reordertasksrandom`` option. (default: 0)

    * When > 0, use the same seed for all iterations
    * When < 0, different seed for each iteration

quitOnError
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Upon error encountered on ``checkWrite`` or ``checkRead``,
display current error and then stop execution.  Otherwise, count errors and
continue (default: 0)

numTasks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Number of tasks that should participate in the test.  0
denotes all tasks.  (default: 0)

interTestDelay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Time (in seconds) to delay before beginning a write or
read phase in a series of tests This does not delay before check-write or
check-read phases.  (default: 0)

outlierThreshold
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Gives warning if any task is more than this number of
seconds from the mean of all participating tasks.  The warning includes the
offending task, its timers (start, elapsed create, elapsed transfer, elapsed
close, end), and the mean and standard deviation for all tasks.  When zero,
disable this feature. (default: 0)

intraTestBarriers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use barrier between open, write/read, and close
phases (default: 0)

uniqueDir
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Create and use unique directory for each file-per-process
(default: 0)

writeFile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Write file(s), first deleting any existing file.
The defaults for ``writeFile`` and ``readFile`` are set such that if there
is not at least one of ``-w``, ``-r``, ``-W``, or ``-R``, ``-w`` and ``-r``
are enabled.  If either ``writeFile`` or ``readFile`` are explicitly
enabled, though, its complement is *not* also implicitly enabled.

readFile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Reads existing file(s) as specified by the ``testFile``
option.  The defaults for ``writeFile`` and ``readFile`` are set such that
if there is not at least one of ``-w``, ``-r``, ``-W``, or ``-R``, ``-w``
and ``-r`` are enabled.  If either ``writeFile`` or ``readFile`` are
explicitly enabled, though, its complement is *not* also implicitly enabled.

filePerProc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Have each MPI process perform I/O to a unique file
(default: 0)

checkWrite
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Read data back and check for errors against known pattern.
Can be used independently of ``writeFile``.  Data checking is not timed and
does not affect other performance timings.  All errors detected are tallied
and returned as the program exit code unless ``quitOnError`` is set.
(default: 0)

checkRead
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Re-read data and check for errors between reads.  Can be
used independently of ``readFile``.  Data checking is not timed and does not
affect other performance timings.  All errors detected are tallied and
returned as the program exit code unless ``quitOnError`` is set.
(default: 0)

keepFile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Do not remove test file(s) on program exit (default: 0)

keepFileWithError
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Do not delete any files containing errors if
detected during read-check or write-check phases. (default: 0)

useExistingTestFile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Do not remove test file(s) before write phase
(default: 0)

segmentCount
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Number of segments in file, where a segment is a
contiguous chunk of data accessed by multiple clients each writing/reading
their own contiguous data (blocks).  The exact semantics of segments
depend on the API used; for example, HDF5 repeats the pattern of an entire
shared dataset. (default: 1)

blockSize
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Size (in bytes) of a contiguous chunk of data accessed by a
single client.  It is comprised of one or more transfers (default: 1048576)

transferSize
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Size (in bytes) of a single data buffer to be transferred
in a single I/O call (default: 262144)

verbose
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Output more information about what IOR is doing.  Can be set
to levels 0-5; repeating the -v flag will increase verbosity level.
(default: 0)

The information shown for different verbosity levels is as follows:

======  ===================================
Level   Behavior
======  ===================================
  0     default; only bare essentials shown
  1     max clock deviation, participating tasks, free space, access pattern, commence/verify access notification with time
  2     rank/hostname, machine name, timer used, individual repetition performance results, timestamp used for data signature
  3     full test details, transfer block/offset compared, individual data checking errors, environment variables, task writing/reading file name, all test operation times
  4     task id and offset for each transfer
  5     each 8-byte data signature comparison (WARNING: more data to STDOUT than stored in file, use carefully)
======  ===================================

setTimeStampSignature
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Value to use for the time stamp signature.  Used
to rerun tests with the exact data pattern by setting data signature to
contain positive integer value as timestamp to be written in data file; if
set to 0, is disabled (default: 0)

showHelp
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Display options and help (default: 0)

storeFileOffset
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use file offset as stored signature when writing file.
This will affect performance measurements (default: 0)

memoryPerNode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Allocate memory on each node to simulate real
application memory usage or restrict page cache size.  Accepts a percentage
of node memory (e.g. ``50%``) on systems that support
``sysconf(_SC_PHYS_PAGES)`` or a size.  Allocation will be split between
tasks that share the node. (default: 0)

memoryPerTask
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Allocate specified amount of memory (in bytes) per task
to simulate real application memory usage. (default: 0)

maxTimeDuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Max time (in minutes) to run all tests.  Any current
read/write phase is not interrupted; only future I/O phases are cancelled
once this time is exceeded.  Value of zero unsets disables. (default: 0)

deadlineForStonewalling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Seconds before stopping write or read phase.
Used for measuring the amount of data moved in a fixed time.  After the
barrier, each task starts its own timer, begins moving data, and the stops
moving data at a pre-arranged time.  Instead of measuring the amount of time
to move a fixed amount of data, this option measures the amount of data
moved in a fixed amount of time.  The objective is to prevent straggling
tasks slow from skewing the performance.  This option is incompatible with
read-check and write-check modes.  Value of zero unsets this option.
(default: 0)

randomOffset
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Randomize access offsets within test file(s).  Currently
incompatible with ``checkRead``, ``storeFileOffset``, MPIIO ``collective``
and ``useFileView``, and HDF5 and NCMPI APIs. (default: 0)

summaryAlways
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Always print the long summary for each test even if the job is interrupted. (default: 0)




POSIX-only options
--------------------------------------------------------------------------------

posix.odirect
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use direct I/O for POSIX, bypassing I/O buffers.  This option was formerly
known as ``-B`` or ``useO_DIRECT``.  (default: 0)

singleXferAttempt
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Do not continue to retry transfer entire buffer
until it is transferred.  When performing a write() or read() in POSIX,
there is no guarantee that the entire requested size of the buffer will be
transferred; this flag keeps the retrying a single transfer until it
completes or returns an error (default: 0)

fsyncPerWrite
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Perform fsync after each POSIX write (default: 0)

fsync
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Perform fsync after POSIX file close (default: 0)




MPIIO-only options
--------------------------------------------------------------------------------

preallocate
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Preallocate the entire file before writing (default: 0)

useFileView
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use an MPI datatype for setting the file view option to
use individual file pointer.  Default IOR uses explicit file pointers.
(default: 0)

useSharedFilePointer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use a shared file pointer.  Default IOR uses
explicit file pointers. (default: 0)

useStridedDatatype
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Create a datatype (max=2GB) for strided access;
akin to ``MULTIBLOCK_REGION_SIZE`` (default: 0)




HDF5-only options
--------------------------------------------------------------------------------

individualDataSets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Within a single file, each task will access its own
dataset.  Default IOR creates a dataset the size of ``numTasks * blockSize``
to be accessed by all tasks (default: 0)

noFill
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Do not pre-fill data in HDF5 file creation (default: 0)

setAlignment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Set the HDF5 alignment in bytes (e.g.: 8, 4k, 2m, 1g) (default: 1)

hdf5.collectiveMetadata
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Enable HDF5 collective metadata (available since HDF5-1.10.0)




MPIIO-, HDF5-, and NCMPI-only options
--------------------------------------------------------------------------------

collective
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Uses collective operations for access (default: 0)

showHints
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Show hint/value pairs attached to open file.  Not available
for NCMPI. (default: 0)




Lustre-specific options
--------------------------------------------------------------------------------

lustreStripeCount
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Set the Lustre stripe count for the test file(s) (default: 0)

lustreStripeSize
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Set the Lustre stripe size for the test file(s) (default: 0)

lustreStartOST
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Set the starting OST for the test file(s) (default: -1)

lustreIgnoreLocks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Disable Lustre range locking (default: 0)




GPFS-specific options
--------------------------------------------------------------------------------

gpfsHintAccess
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use ``gpfs_fcntl`` hints to pre-declare accesses (default: 0)

gpfsReleaseToken
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Release all locks immediately after opening or
creating file.  Might help mitigate lock-revocation traffic when many
processes write/read to same file. (default: 0)




BeeGFS-specific options
--------------------------------------------------------------------------------

beegfsNumTargets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Define the stripe width for file(s) on BeeGFS.  Must be greater than zero.

beegfsChunkSize 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Define the stripe chunk size for the file(s) on BeeGFS.  Must be a power of two
and larger than 64K.




Notes on compressibility
--------------------------------------------------------------------------------
Please note that incompressibility is a factor of how large a block compression
algorithm uses.  The incompressible buffer is filled only once before write
times, so if the compression algorithm takes in blocks larger than the transfer
size, there will be compression.  Below are some baselines for zip, gzip, and
bzip.

1)  zip:  For zipped files, a transfer size of 1k is sufficient.

2)  gzip: For gzipped files, a transfer size of 1k is sufficient.

3)  bzip2: For bziped files a transfer size of 1k is insufficient (~50% compressed).
    To avoid compression a transfer size of greater than the bzip block size is required
    (default = 900KB). I suggest a transfer size of greather than 1MB to avoid bzip2 compression.

Be aware of the block size your compression algorithm will look at, and adjust
the transfer size accordingly.
