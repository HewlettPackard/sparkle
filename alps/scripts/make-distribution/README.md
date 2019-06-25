## Copyright notice files

The script works by attaching the contents of copyright.*.txt to each source file 
based on the file type. The default copyright notice is Apache. 

* copyright.c.txt: applies to C, C++ source and header files
* copyright.md.txt: applies to markdown source files
* copyright.sh.txt: applies to shell source files, including python scripts 

## Creating a source tree to be distributed and adding a copyright notice

To clean up and prepare a source tree for distribution and add a copyright 
notice:

```
$ make-distribution.sh <PROJECT_DIR> <DISTRIBUTION_DIR>
```

Note: This script will copy the source tree pointed by PROJECT_DIR to 
DISTRIBUTION_DIR, clean up the copied tree, and add a copyright notice. 

## Adding copyright notice to a to-be-distributed source tree

To add a copyright notice to a source tree that is already 
cleaned up and prepared for distribution:

```
$ add_copyright.sh <DISTRIBUTION_DIR>
```

Note: This script assumes that the source tree pointed by DISTRIBUTION_DIR is 
already cleaned up and ready for distribution except the missing copyright notice.


