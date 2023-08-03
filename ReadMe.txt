Dirlist - Customizable directory listings with md5, crc32, sha1 and more

======================================================================

Dirlist is a utility I wrote for the purpose of creating easily
customizable directory listings that would allow easy differencing based on
dates, times, hashes, or other attributes of files.

======================================================================

Usage:    dirlist [options] [directory [format] [additional wildcards]]

Example:  dirlist C:\WINDOWS\*.dll "%d %14.14s %p"

Options:  -s  Recursive listing
          -f  Calculate full directory sizes
          -t  Use complete time (hh:mm:ss.ms)
          -h  Use human-readable file sizes (KB, MB, GB)
          -r  Reverse any sort order specified
          -l  Use relative pathnames (changes %p to work as %l)
          -e  Print unsorted output to stderr
          -aX Limit by type X, where X is any of [DHSRCEOPT], which signify:
              Dir, Hidden, Sys, Readonly, Comp., Encrypt, Offline, Sparse, Temp

          -p  size   Display a partial-file hash every "size" megabytes
          -D  path   Similar to -d but puts the file in \"path\"
          -L  path   Similar to -l but uses \"path\" as the relative root
          -F  list   use \"list\" as the list of files to process

Format:   As in "printf" but with the following format variables:
          %c  CRC-32 value                %m  MD-5 value
          %d  File modification date/time %D  File creation date/time
          %f  Long file name              %F  Short file name
          %s  File size (no commas)       %S  File size (with commas)
          %n  Directory name              %a  File attributes
          %P  Absolute path and filename  %L  Relative-root-path and filename
          %p  same as %P except with -l   %l  Relative-sub-path and filename
          %w  Length of the filename      %h  SHA-1 value

Sorting:  Use a '$' in place of a '%' to indicate sorting on that variable

Default formats:  The following options use predefined format strings:
          --crc         "%c %l"               --sortcrc     "%c $f" /a-d
          --sort        "%14.14s %p"          --sortsize    "$14.14s %p"
          --dirsize     "%14.14s %p" /ad /f   --sortdir     "$14.14s %p" /ad /f
          --md5         "%m %l"               --sha1        "%h %l"

		  
======================================================================

"Dirlist" is copyright 2005-2023 by Eric VanHeest (edv_ws@vanheest.org). 
Feel free to modify this program to suit your needs.

