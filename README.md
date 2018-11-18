# FAT32-File-System
FAT32 File System assignment for Operating Systems class

open <filename>  
close   
info  
stat <filename> or <directory name>  
get <filename>  
cd <directory>  
ls  
read <filename> <position> <number of bytes>  
volume  
  
  
# Implemented Commands

## open  <image name>
After calling open, check to see if FP is NULL. If NULL, print "Error: File system image not found." If non-null, print "Error: File system image already open."  

## close  
"After calling close, check to see if FP is NULL. If NULL, print "Error: File system not open." Any command after close, except for open, prints "Error: File system image must be opened first."  


## info  
Print out values for:  
BPB_BytesPerSec  
BPB_SecPerClus  
BPB_RsvdSecCnt  
BPB_NumFATS  
BPB_FATSz32  


## stat  <filename>
Print attributes and starting cluster number of the file or directory name. If it's a directory name, size is 0. If the file or directory does not exist, print "Error: File not found."  


## get  <filename>
Retrieves file from the FAT32 image and places it in your cwd (current working directory). If it does not exist, print "Error: File not found."  


## cd  <folder>
Use chdir(input)  


## ls  
List directory contents. Supports "." and ".."  


## read  <filename>
'Reads from the given file at the position, in bytes, specified by the position parameter and output the number bytes specified.' Byte is in size 1, use fseek and have the position be used as the offset. Number of bytes is the count to read.  


## volume  
Print the volume name of the file system image. If it exists, it will be in the reserved section. If it does not exist, print "Error: volume name not found."
