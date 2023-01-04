from lib.utility import log
from lib.lwrdo.helper.run_ssh import run_ssh
from lib.service.wait import wait

logger = log.default_logger


@log.class_logger
class DiskCreate():
    pass


# command tuple list (functionName, hasParams, command)
methods = \
    [('create',
        'diskCreate -f %s')
     , ('zzzzzzz',
        'diskCreate zzzzzzz %s')

     ]


def make_function(fcmd):
    @log.service_class_function_named('lib.service')
    def disk_create(ssh,
                parameters,
                silent=False,
                ignoreError=False,
                estimatedTimeToComplete=0):
        command = fcmd % parameters
        logger.verbose('diskCreate: %s' % command)
        return run_ssh(ssh,
                       command,
                       silent=silent,
                       ignoreError=ignoreError,
                       estimatedTimeToComplete=estimatedTimeToComplete)
    return disk_create


print('Creating %d esx host methods for class:DiskCreate' % len(methods))
for method in methods:
    print('VimCmd.%s() -> %s' % (method[0], method[1]))
    setattr(DiskCreate,
            method[0],
            staticmethod(make_function(method[1])))

"""
disk creation tool.
Usage: diskCreate OPTIONS diskName
 Options:
   -a <string>     : adapter type (ide, buslogic, lsilogic, lsisas or pvsci) [ide]
   -A <str or id>  : disk allocation type (mono or 2 GB plain disk only)
   -b <chunk size> : specify chunk size for bitmap cloning
   -B <fileName>   : clone from disk using a bitmap of allocated sectors
   -c              : clean up newly created files
   -C <fileName>   : clone from disk
   -d              : create a digest
   -D <device>     : device (for fullDevice disks)
   -e <extentSize> : extentSize for a split flat/sparse hosted/esx format
   -E data         : encrypt data (default) (also use -K at least once)
   -E descriptor   : encrypt descriptor (insecure -- do not use)
   -E compress:<alg> : create compressed (default algorithm: 'deflate')
   -f <fileName>   : create child link from this disk
   -F              : don't create digest disk when creating a child (use with -f)
   -g <number>     : grain size in sectors [128]
                   : <0 => use value from DiskLib_CreateChildCreateParam()
   -G <algo>       : digest SHA algorithm (SHA-1=1, SHA-256=2)
   -h              : this message
   -H              : clone only the bottommost link (with -C only)
   -i <policyFile> : policy specification for the disk read from a file
   -I <policy>     : policy specification for the disk
   -j <size>       : digest journal size (8MB=default or 1MB multiples in power of 2)
   -k              : in-place convert existing disk to specified format
   -K pw           : use password read from console as key
   -K pub:<file>   : use public key from file as key
   -L              : turn on collision detection for digest disks
   -l              : Create a native linked clone (with -f only).
   -M <cipher>     : cipher to use for encryption
   -n              : use native snapshotting (with -f only). only available on esx
   -N              : avoid using native cloning (with -C only).
   -o <ctk>        : change tracking options
   -O <str>        : disk backing object type (file, vsan, pmem, vvol, or upit) [file]
   -p              : print partition info and exit
   -P              : turn on partition offset for digest disks
   -q              : quiet
   -Q              : super quiet
   -r 0|1          : recompute digest disk (1=full, 0=partial)
   -R <str>        : Storage container ID for object storage
   -s <capacity>   : size in sectors (or KB, MB, GB or TB) [1008 sectors]
   -S              : shuffle order of grains in new disk (with -C only)
   -t <str or id>  : disk type or id [monoSparse, 0]
   -T              : Do a dry run for the native clone (with -n or -C only)
   -u              : Do not swizzle while taking a native snapshot (with -f only)
   -v level        : verbosity level
   -w <name=value> : disk backing object type specific parameters
   -z <size>       : digest block size (4KB=default or 4KB multiples in power of 2)
   --plugin <path> : specify path of DiskLib plugin
   --importUnmanaged <uuid>: Given an object uuid, create a descriptor for the unmanaged object
   --vdfm <f1,fn>  : attach listed VDFM filters to new base disk
   --iofilters <f1,fn>  : attach listed IO filters to new base disk
 Allocation types:
  0 zeroPreAlloc   : zeroed pre allocated disk
  1 onDemandAlloc  : allocate on demand
  2 vmfsNonzeroPreAlloc: pre allocated disk not zeroed (unsupported)
  3 vmfsScrubPreAlloc  : pre allocated disk, zeroed at first write (vmfs only)
 Disk types:
  0 monoSparse     : single sparse file with embedded descriptor file
  1 twoGBSparse    : descriptor file with 1 or more <= 2 GB sparse extent files
  2 monoFlat       : descriptor file and single flat file
  3 twoGBFlat      : descriptor file with 1 or more <= 2 GB flat extent files
  4 legacySparse   : WS3.x, GSX2.x disk
  5 dynamicAllocGT : WS3.x, GSX2.x disk, but don't preallocate grain tables
  6 fullDevice     : device backed disks
  7 partitionedDevice: device backed disks using partitions
  8 vmfs           : vmfs file.
  9 vmfssparse     : sparse vmfs file.  only available on esx
  10 vmfsrdm       : a vmfs raw disk map (aka symlink).  only available on esx
  11 vmfsrdmpassthru: a vmfs raw disk map (aka symlink) in passthrough mode.
                      only available on esx
  12 vmfsraw       : an esx raw disk. only available on esx
  14 streamOptimized: compressed monoSparse optimized for streaming
  15 pvfs          : PVFS disk and database
  16 seSparse      : VMFS space efficient virtual disk. only available on esx

"""
