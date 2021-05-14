/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 */
/******************************************************************************\
*                                                                              *
*        Copyright (c) 2003, The Regents of the University of California       *
*      See the file COPYRIGHT for a complete copyright notice and license.     *
*                                                                              *
********************************************************************************
*
* Implement of abstract I/O interface for POSIX.
*
\******************************************************************************/
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#  include <sys/ioctl.h>          /* necessary for: */
#  define __USE_GNU               /* O_DIRECT and */
#  include <fcntl.h>              /* IO operations */
#  undef __USE_GNU
#endif                          /* __linux__ */

#include <errno.h>
#include <fcntl.h>              /* IO operations */
#include <sys/stat.h>
#include <assert.h>


#if defined(HAVE_LUSTRE_LUSTREAPI)
#  include <string.h>
#  if defined(HAVE_LUSTRE_LUSTREAPI_H)
#    include <lustre/lustreapi.h>
#  elif defined(HAVE_LINUX_LUSTRE_LUSTRE_USER_H)
#    include <linux/lustre/lustre_user.h>
#  elif defined(HAVE_LUSTRE_LUSTRE_USER_H)
#    include <lustre/lustre_user.h>
#  endif
#endif
#ifdef HAVE_GPFS_H
#  include <gpfs.h>
#endif
#ifdef HAVE_GPFS_FCNTL_H
#  include <gpfs_fcntl.h>
#endif

#ifdef HAVE_BEEGFS_BEEGFS_H
#include <beegfs/beegfs.h>
#include <dirent.h>
#include <libgen.h>
#endif

#include "ior.h"
#include "aiori.h"
#include "iordef.h"
#include "utilities.h"

#ifndef   open64                /* necessary for TRU64 -- */
#  define open64  open            /* unlikely, but may pose */
#endif  /* not open64 */                        /* conflicting prototypes */

#ifndef   lseek64               /* necessary for TRU64 -- */
#  define lseek64 lseek           /* unlikely, but may pose */
#endif  /* not lseek64 */                        /* conflicting prototypes */

#ifndef   O_BINARY              /* Required on Windows    */
#  define O_BINARY 0
#endif

/**************************** P R O T O T Y P E S *****************************/
static IOR_offset_t POSIX_Xfer(int, void *, IOR_size_t *,
                               IOR_offset_t, IOR_param_t *);
static void POSIX_Fsync(void *, IOR_param_t *);
static void POSIX_Sync(IOR_param_t * );

/************************** O P T I O N S *****************************/
typedef struct{
  /* in case of a change, please update depending MMAP module too */
  int direct_io;
} posix_options_t;


option_help * POSIX_options(void ** init_backend_options, void * init_values){
  posix_options_t * o = malloc(sizeof(posix_options_t));

  if (init_values != NULL){
    memcpy(o, init_values, sizeof(posix_options_t));
  }else{
    o->direct_io = 0;
  }

  *init_backend_options = o;

  option_help h [] = {
    {0, "posix.odirect", "Direct I/O Mode", OPTION_FLAG, 'd', & o->direct_io},
    LAST_OPTION
  };
  option_help * help = malloc(sizeof(h));
  memcpy(help, h, sizeof(h));
  return help;
}


/************************** D E C L A R A T I O N S ***************************/


ior_aiori_t posix_aiori = {
        .name = "POSIX",
        .name_legacy = NULL,
        .create = POSIX_Create,
        .mknod = POSIX_Mknod,
        .open = POSIX_Open,
        .xfer = POSIX_Xfer,
        .close = POSIX_Close,
        .delete = POSIX_Delete,
        .get_version = aiori_get_version,
        .fsync = POSIX_Fsync,
        .get_file_size = POSIX_GetFileSize,
        .statfs = aiori_posix_statfs,
        .mkdir = aiori_posix_mkdir,
        .rmdir = aiori_posix_rmdir,
        .access = aiori_posix_access,
        .stat = aiori_posix_stat,
        .get_options = POSIX_options,
        .enable_mdtest = true,
        .sync = POSIX_Sync
};

/***************************** F U N C T I O N S ******************************/


#ifdef HAVE_GPFS_FCNTL_H
void gpfs_free_all_locks(int fd)
{
        int rc;
        struct {
                gpfsFcntlHeader_t header;
                gpfsFreeRange_t release;
        } release_all;
        release_all.header.totalLength = sizeof(release_all);
        release_all.header.fcntlVersion = GPFS_FCNTL_CURRENT_VERSION;
        release_all.header.fcntlReserved = 0;

        release_all.release.structLen = sizeof(release_all.release);
        release_all.release.structType = GPFS_FREE_RANGE;
        release_all.release.start = 0;
        release_all.release.length = 0;

        rc = gpfs_fcntl(fd, &release_all);
        if (verbose >= VERBOSE_0 && rc != 0) {
                EWARNF("gpfs_fcntl(%d, ...) release all locks hint failed.", fd);
        }
}
void gpfs_access_start(int fd, IOR_offset_t length, IOR_param_t *param, int access)
{
        int rc;
        struct {
                gpfsFcntlHeader_t header;
                gpfsAccessRange_t access;
        } take_locks;

        take_locks.header.totalLength = sizeof(take_locks);
        take_locks.header.fcntlVersion = GPFS_FCNTL_CURRENT_VERSION;
        take_locks.header.fcntlReserved = 0;

        take_locks.access.structLen = sizeof(take_locks.access);
        take_locks.access.structType = GPFS_ACCESS_RANGE;
        take_locks.access.start = param->offset;
        take_locks.access.length = length;
        take_locks.access.isWrite = (access == WRITE);

        rc = gpfs_fcntl(fd, &take_locks);
        if (verbose >= VERBOSE_2 && rc != 0) {
                EWARNF("gpfs_fcntl(%d, ...) access range hint failed.", fd);
        }
}

void gpfs_access_end(int fd, IOR_offset_t length, IOR_param_t *param, int access)
{
        int rc;
        struct {
                gpfsFcntlHeader_t header;
                gpfsFreeRange_t free;
        } free_locks;


        free_locks.header.totalLength = sizeof(free_locks);
        free_locks.header.fcntlVersion = GPFS_FCNTL_CURRENT_VERSION;
        free_locks.header.fcntlReserved = 0;

        free_locks.free.structLen = sizeof(free_locks.free);
        free_locks.free.structType = GPFS_FREE_RANGE;
        free_locks.free.start = param->offset;
        free_locks.free.length = length;

        rc = gpfs_fcntl(fd, &free_locks);
        if (verbose >= VERBOSE_2 && rc != 0) {
                EWARNF("gpfs_fcntl(%d, ...) free range hint failed.", fd);
        }
}

#endif

#ifdef HAVE_BEEGFS_BEEGFS_H

int mkTempInDir(char* dirPath)
{
   unsigned long len = strlen(dirPath) + 8;
   char* tmpfilename = (char*)malloc(sizeof (char)*len+1);
   snprintf(tmpfilename, len, "%s/XXXXXX", dirPath);

   int fd = mkstemp(tmpfilename);
   unlink(tmpfilename);
   free(tmpfilename);

   return fd;
}

bool beegfs_getStriping(char* dirPath, u_int16_t* numTargetsOut, unsigned* chunkSizeOut)
{
   bool retVal = false;

   int fd = mkTempInDir(dirPath);
   if (fd) {
      unsigned stripePattern = 0;
      retVal = beegfs_getStripeInfo(fd, &stripePattern, chunkSizeOut, numTargetsOut);
      close(fd);
   }

   return retVal;
}

bool beegfs_isOptionSet(int opt) {
   return opt != -1;
}

bool beegfs_compatibleFileExists(char* filepath, int numTargets, int chunkSize)
{
      int fd = open(filepath, O_RDWR);

      if (fd == -1)
         return false;

      unsigned read_stripePattern = 0;
      u_int16_t read_numTargets = 0;
      int read_chunkSize = 0;

      bool retVal = beegfs_getStripeInfo(fd, &read_stripePattern, &read_chunkSize, &read_numTargets);

      close(fd);

      return retVal && read_numTargets == numTargets && read_chunkSize == chunkSize;
}

/*
 * Create a file on a BeeGFS file system with striping parameters
 */
bool beegfs_createFilePath(char* filepath, mode_t mode, int numTargets, int chunkSize)
{
        bool retVal = false;
        char* dirTmp = strdup(filepath);
        char* dir = dirname(dirTmp);
        DIR* parentDirS = opendir(dir);
        if (!parentDirS) {
                ERRF("Failed to get directory: %s", dir);
        }
        else
        {
                int parentDirFd = dirfd(parentDirS);
                if (parentDirFd < 0)
                {
                        ERRF("Failed to get directory descriptor: %s", dir);
                }
                else
                {
                        bool isBeegfs = beegfs_testIsBeeGFS(parentDirFd);
                        if (!isBeegfs)
                        {
                                WARN("Not a BeeGFS file system");
                        }
                        else
                        {
                                if (   !beegfs_isOptionSet(numTargets)
                                    || !beegfs_isOptionSet(chunkSize)) {
                                        u_int16_t defaultNumTargets = 0;
                                        unsigned  defaultChunkSize  = 0;
                                        bool haveDefaults = beegfs_getStriping(dir,
                                                                               &defaultNumTargets,
                                                                               &defaultChunkSize);
                                        if (!haveDefaults)
                                                ERR("Failed to get default BeeGFS striping values");

                                        numTargets = beegfs_isOptionSet(numTargets) ?
                                                        numTargets : defaultNumTargets;
                                        chunkSize  = beegfs_isOptionSet(chunkSize) ?
                                                        chunkSize  : defaultChunkSize;
                                }

                                char* filenameTmp = strdup(filepath);
                                char* filename = basename(filepath);
                                bool isFileCreated =    beegfs_compatibleFileExists(filepath, numTargets, chunkSize)
                                                     || beegfs_createFile(parentDirFd, filename,
                                                                          mode, numTargets, chunkSize);
                                if (!isFileCreated)
                                        ERR("Could not create file");
                                retVal = true;
                                free(filenameTmp);
                        }
                }
                closedir(parentDirS);
        }
        free(dirTmp);
        return retVal;
}
#endif /* HAVE_BEEGFS_BEEGFS_H */

#ifdef HAVE_LUSTRE_LUSTREAPI

#ifndef LLAPI_LAYOUT_OVERSTRIPING
    #define LLAPI_LAYOUT_OVERSTRIPING 4ULL
#endif

#ifndef strchrnul
char *strchrnul(const char *s1, int i) {
    char *s = strchr(s1, i);
    return s ? s : (char *)s1 + strlen(s1);
}
#endif

static int lustre_parseTargets(uint32_t *tgts, int size, int offset, char *arg,
        unsigned long long *pattern)
{
    int rc;
    int nr = offset;
    int slots = size - offset;
    char *ptr = NULL;
    bool overstriped = false;
    bool end_of_loop;

    if (arg == NULL)
        return -EINVAL;

    end_of_loop = false;
    while (!end_of_loop) {
        int start_index = 0;
        int end_index = 0;
        int i;
        char *endptr = NULL;

        rc = -EINVAL;

        ptr = strchrnul(arg, ',');

        end_of_loop = *ptr == '\0';
        *ptr = '\0';

        start_index = strtol(arg, &endptr, 0);
        if (endptr == arg) /* no data at all */
            break;
        if (*endptr != '-' && *endptr != '\0') /* has invalid data */
            break;

        end_index = start_index;
        if (*endptr == '-') {
            end_index = strtol(endptr + 1, &endptr, 0);
            if (*endptr != '\0')
                break;
            if (end_index < start_index)
                break;
        }

        for (i = start_index; i <= end_index && slots > 0; i++) {
            int j;

            /* remove duplicate */
            for (j = 0; j < offset; j++) {
                if (tgts[j] == i
                && pattern
                && *pattern == LLAPI_LAYOUT_OVERSTRIPING)
                    overstriped = true;
                else if (tgts[j] == i)
                    return -EINVAL;
            }

            j = offset;

            if (j == offset) { /* check complete */
                tgts[nr++] = i;
                --slots;
            }
        }

        if (slots == 0 && i < end_index)
            break;

        *ptr = ',';
        arg = ++ptr;
        offset = nr;
        rc = 0;
    }
    if (!end_of_loop && ptr != NULL)
        *ptr = ',';

    if (!overstriped && pattern)
        *pattern = LLAPI_LAYOUT_DEFAULT;

    return rc < 0 ? rc : nr;
}

int lustre_createFilePath(char *filename, int open_flags, mode_t mode, char *layout_str, unsigned long long stripe_size, int stripe_width)
{
    uint32_t targets[LOV_MAX_STRIPE_COUNT] = {0};

    int num_osts = lustre_parseTargets(
        targets,
        sizeof(targets) / sizeof(uint32_t), /* should always be LOV_MAX_STRIPE_COUNT */
        0,
        layout_str,
        NULL);

    if (num_osts < 0)
    {
        EWARNF("invalid OST target(s): %s\n", layout_str);
        return EINVAL;
    }

    struct llapi_layout *layout = llapi_layout_alloc();
    if (!layout)
    {
        EWARN("llapi_layout_alloc() failed\n");
        return ENOMEM;
    }

    llapi_layout_stripe_count_set(layout, stripe_width > 0 ? stripe_width : num_osts);
    llapi_layout_stripe_size_set(layout, stripe_size);

    if (stripe_width > 0) {
        /* evenly distribute files across osts */
        int stride = num_osts / stripe_width;
        for (int i = 0; i < stripe_width; i++) {
            int ost_idx = (i * stride + rank) % num_osts;
            int rc = llapi_layout_ost_index_set(layout, i, targets[ost_idx]);
            if (rc != 0)
            {
                EWARNF("failed to set layout on %s", filename);
                break;
            }
        }
    }
    else {
        /* stripe every file across every ost */
        for (int i = 0; i < num_osts; i++) {
            int rc = llapi_layout_ost_index_set(layout, i, targets[i]);
            if (rc != 0)
            {
                EWARNF("failed to set layout on %s", filename);
                break;
            }
        }
    }

    int fd = llapi_layout_file_create(filename, open_flags|O_CREAT|O_EXCL, mode, layout);

    llapi_layout_free(layout);

    return fd;
}
#endif /* HAVE_LUSTRE_LUSTREAPI */

/*
 * Creat and open a file through the POSIX interface.
 */
void *POSIX_Create(char *testFileName, IOR_param_t * param)
{
        int fd_oflag = O_BINARY;
        int mode = 0664;
        int *fd;

        fd = (int *)malloc(sizeof(int));
        if (fd == NULL)
                ERR("Unable to malloc file descriptor");
        posix_options_t * o = (posix_options_t*) param->backend_options;
        if (o->direct_io == TRUE){
          set_o_direct_flag(&fd_oflag);
        }

        if(param->dryRun)
          return 0;

#ifdef HAVE_LUSTRE_LUSTREAPI
        if (param->lustre_set_striping) {
                /* In the single-shared-file case, task 0 has to creat the
                   file with the Lustre striping options before any other processes
                   open the file */
                if (!param->filePerProc && rank != 0) {
                    printf("not setting striping\n");
                        MPI_CHECK(MPI_Barrier(testComm), "barrier error");
                        fd_oflag |= O_RDWR;
                        *fd = open64(testFileName, fd_oflag, mode);
                        if (*fd < 0)
                                ERRF("open64(\"%s\", %d, %#o) failed",
                                        testFileName, fd_oflag, mode);
                }
                else {
                    /* lustre stripe size, start ost, stripe count across all OSTs */
                    fd_oflag |= O_RDWR;
                    if (!param->lustre_osts)
                    {
                        *fd = llapi_file_open(
                                testFileName,
                                fd_oflag | O_CREAT | O_EXCL,
                                mode,
                                param->lustre_stripe_size,
                                param->lustre_start_ost,
                                param->lustre_stripe_count,
                                LOV_PATTERN_NONE);
                    }
                    /* explicit OST list */
                    else
                    {
                        *fd = lustre_createFilePath(
                                testFileName,
                                fd_oflag,
                                mode,
                                param->lustre_osts,
                                param->lustre_stripe_size,
                                param->lustre_stripe_count);
                    }

                    if (*fd < 0) {
                        if (*fd == EINVAL)
                            fprintf(stdout, "\nUnable to open '%s': invalid stripe specification\n", testFileName);
                        else if (*fd == EEXIST || *fd == EALREADY)
                            fprintf(stdout, "\nUnable to open '%s': striping information already set\n", testFileName);
                        else if (*fd == ENOTTY)
                            fprintf(stdout, "\nUnable to open '%s': file not on a Lustre file system\n", testFileName);
                        else
                            fprintf(stdout, "\nUnable to open '%s': %s\n", testFileName, strerror(errno));
                        MPI_CHECK(MPI_Abort(MPI_COMM_WORLD, -1), "MPI_Abort() error");
                    } 
                    if (!param->filePerProc)
                        MPI_CHECK(MPI_Barrier(testComm), "barrier error");
                }
        } else {
#endif /* HAVE_LUSTRE_LUSTREAPI */
            printf("no striping to set\n");

                fd_oflag |= O_CREAT | O_RDWR;

#ifdef HAVE_BEEGFS_BEEGFS_H
                if (beegfs_isOptionSet(param->beegfs_chunkSize)
                    || beegfs_isOptionSet(param->beegfs_numTargets)) {
                    bool result = beegfs_createFilePath(testFileName,
                                                        mode,
                                                        param->beegfs_numTargets,
                                                        param->beegfs_chunkSize);
                    if (result) {
                       fd_oflag &= ~O_CREAT;
                    } else {
                       EWARN("BeeGFS tuning failed");
                    }
                 }
#endif /* HAVE_BEEGFS_BEEGFS_H */

                *fd = open64(testFileName, fd_oflag, mode);
                if (*fd < 0)
                        ERRF("open64(\"%s\", %d, %#o) failed",
                                testFileName, fd_oflag, mode);

#ifdef HAVE_LUSTRE_LUSTREAPI
        }

        if (param->lustre_ignore_locks) {
                int lustre_ioctl_flags = LL_FILE_IGNORE_LOCK;
                if (ioctl(*fd, LL_IOC_SETFLAGS, &lustre_ioctl_flags) == -1)
                        ERRF("ioctl(%d, LL_IOC_SETFLAGS, ...) failed", *fd);
        }
#endif /* HAVE_LUSTRE_LUSTREAPI */

#ifdef HAVE_GPFS_FCNTL_H
        /* in the single shared file case, immediately release all locks, with
         * the intent that we can avoid some byte range lock revocation:
         * everyone will be writing/reading from individual regions */
        if (param->gpfs_release_token ) {
                gpfs_free_all_locks(*fd);
        }
#endif
        return ((void *)fd);
}

/*
 * Creat a file through mknod interface.
 */
int POSIX_Mknod(char *testFileName)
{
    int ret;

    ret = mknod(testFileName, S_IFREG | S_IRUSR, 0);
    if (ret < 0)
        ERR("mknod failed");

    return ret;
}

/*
 * Open a file through the POSIX interface.
 */
void *POSIX_Open(char *testFileName, IOR_param_t * param)
{
        int fd_oflag = O_BINARY;
        int *fd;

        fd = (int *)malloc(sizeof(int));
        if (fd == NULL)
                ERR("Unable to malloc file descriptor");

        posix_options_t * o = (posix_options_t*) param->backend_options;
        if (o->direct_io == TRUE)
                set_o_direct_flag(&fd_oflag);

        fd_oflag |= O_RDWR;

        if(param->dryRun)
          return 0;

        *fd = open64(testFileName, fd_oflag);
        if (*fd < 0)
                ERRF("open64(\"%s\", %d) failed", testFileName, fd_oflag);

#ifdef HAVE_LUSTRE_LUSTREAPI
        if (param->lustre_ignore_locks) {
                int lustre_ioctl_flags = LL_FILE_IGNORE_LOCK;
                if (verbose >= VERBOSE_1) {
                        fprintf(stdout,
                                "** Disabling lustre range locking **\n");
                }
                if (ioctl(*fd, LL_IOC_SETFLAGS, &lustre_ioctl_flags) == -1)
                        ERRF("ioctl(%d, LL_IOC_SETFLAGS, ...) failed", *fd);
        }
#endif                          /* HAVE_LUSTRE_LUSTREAPI */

#ifdef HAVE_GPFS_FCNTL_H
        if(param->gpfs_release_token) {
                gpfs_free_all_locks(*fd);
        }
#endif
        return ((void *)fd);
}

/*
 * Write or read access to file using the POSIX interface.
 */
static IOR_offset_t POSIX_Xfer(int access, void *file, IOR_size_t * buffer,
                               IOR_offset_t length, IOR_param_t * param)
{
        int xferRetries = 0;
        long long remaining = (long long)length;
        char *ptr = (char *)buffer;
        long long rc;
        int fd;

        if(param->dryRun)
          return length;

        fd = *(int *)file;

#ifdef HAVE_GPFS_FCNTL_H
        if (param->gpfs_hint_access) {
                gpfs_access_start(fd, length, param, access);
        }
#endif


        /* seek to offset */
        if (lseek64(fd, param->offset, SEEK_SET) == -1)
                ERRF("lseek64(%d, %lld, SEEK_SET) failed", fd, param->offset);

        while (remaining > 0) {
                /* write/read file */
                if (access == WRITE) {  /* WRITE */
                        if (verbose >= VERBOSE_4) {
                                fprintf(stdout,
                                        "task %d writing to offset %lld\n",
                                        rank,
                                        param->offset + length - remaining);
                        }
                        rc = write(fd, ptr, remaining);
                        if (rc == -1)
                                ERRF("write(%d, %p, %lld) failed",
                                        fd, (void*)ptr, remaining);
                        if (param->fsyncPerWrite == TRUE)
                                POSIX_Fsync(&fd, param);
                } else {        /* READ or CHECK */
                        if (verbose >= VERBOSE_4) {
                                fprintf(stdout,
                                        "task %d reading from offset %lld\n",
                                        rank,
                                        param->offset + length - remaining);
                        }
                        rc = read(fd, ptr, remaining);
                        if (rc == 0)
                                ERRF_SIMPLE("read(%d, %p, %lld) returned EOF prematurely",
                                        fd, (void*)ptr, remaining);
                        if (rc == -1)
                                ERRF("read(%d, %p, %lld) failed",
                                        fd, (void*)ptr, remaining);
                }
                if (rc < remaining) {
                        fprintf(stdout,
                                "WARNING: Task %d, partial %s, %lld of %lld bytes at offset %lld\n",
                                rank,
                                access == WRITE ? "write()" : "read()",
                                rc, remaining,
                                param->offset + length - remaining);
                        if (param->singleXferAttempt == TRUE)
                                MPI_CHECK(MPI_Abort(MPI_COMM_WORLD, -1),
                                          "barrier error");
                        if (xferRetries > MAX_RETRY)
                                ERR("too many retries -- aborting");
                }
                assert(rc >= 0);
                assert(rc <= remaining);
                remaining -= rc;
                ptr += rc;
                xferRetries++;
        }
#ifdef HAVE_GPFS_FCNTL_H
        if (param->gpfs_hint_access) {
                gpfs_access_end(fd, length, param, access);
        }
#endif
        return (length);
}

/*
 * Perform fsync().
 */
static void POSIX_Fsync(void *fd, IOR_param_t * param)
{
        if (fsync(*(int *)fd) != 0)
                EWARNF("fsync(%d) failed", *(int *)fd);
}


static void POSIX_Sync(IOR_param_t * param)
{
  int ret = system("sync");
  if (ret != 0){
    FAIL("Error executing the sync command, ensure it exists.");
  }
}


/*
 * Close a file through the POSIX interface.
 */
void POSIX_Close(void *fd, IOR_param_t * param)
{
        if(param->dryRun)
          return;
        if (close(*(int *)fd) != 0)
                ERRF("close(%d) failed", *(int *)fd);
        free(fd);
}

/*
 * Delete a file through the POSIX interface.
 */
void POSIX_Delete(char *testFileName, IOR_param_t * param)
{
        if(param->dryRun)
          return;
        if (unlink(testFileName) != 0){
                EWARNF("[RANK %03d]: unlink() of file \"%s\" failed\n",
                       rank, testFileName);
        }
}

/*
 * Use POSIX stat() to return aggregate file size.
 */
IOR_offset_t POSIX_GetFileSize(IOR_param_t * test, MPI_Comm testComm,
                                      char *testFileName)
{
        if(test->dryRun)
          return 0;
        struct stat stat_buf;
        IOR_offset_t aggFileSizeFromStat, tmpMin, tmpMax, tmpSum;

        if (stat(testFileName, &stat_buf) != 0) {
                ERRF("stat(\"%s\", ...) failed", testFileName);
        }
        aggFileSizeFromStat = stat_buf.st_size;

        if (test->filePerProc == TRUE) {
                MPI_CHECK(MPI_Allreduce(&aggFileSizeFromStat, &tmpSum, 1,
                                        MPI_LONG_LONG_INT, MPI_SUM, testComm),
                          "cannot total data moved");
                aggFileSizeFromStat = tmpSum;
        } else {
                MPI_CHECK(MPI_Allreduce(&aggFileSizeFromStat, &tmpMin, 1,
                                        MPI_LONG_LONG_INT, MPI_MIN, testComm),
                          "cannot total data moved");
                MPI_CHECK(MPI_Allreduce(&aggFileSizeFromStat, &tmpMax, 1,
                                        MPI_LONG_LONG_INT, MPI_MAX, testComm),
                          "cannot total data moved");
                if (tmpMin != tmpMax) {
                        if (rank == 0) {
                                WARN("inconsistent file size by different tasks");
                        }
                        /* incorrect, but now consistent across tasks */
                        aggFileSizeFromStat = tmpMin;
                }
        }

        return (aggFileSizeFromStat);
}
