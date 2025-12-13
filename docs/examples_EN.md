# 🎨 Command Preview Examples

This document showcases the actual usage and output examples of various Linux Command Pro commands.

## 📁 pls - File Listing

```bash
$ pls
📁 Directory Contents (5 files/directories)
====================
📁 📁 Documents
📄 📄 README.md
⚡ ⚡ main.c
💻 💻 script.py
📦 📦 archive.zip
```

## 📄 pcat - File Display

```bash
$ pcat -n main.c
File: main.c
====================
   1 | #include <stdio.h>
   2 | int main() {
   3 |     printf("Hello World\n");
   4 |     return 0;
   5 | }
```

## 🔍 pfind - File Search

```bash
$ pfind . -name "*.c"
Search Results (3 matches)
====================
⚡ ./src/main.c
⚡ ./src/utils.c
⚡ ./test/test.c
```

## 🔎 pgrep - Text Search

```bash
$ pgrep -n "printf" main.c
Search Results (1 match)
====================
   3:     printf("Hello World\n");
```

## ✂️ psplit - File Split

```bash
$ psplit -l 1000 -v bigfile.txt parts
Starting line-based file split...
File: bigfile.txt
Total lines: 5000
Lines per chunk: 1000
Split progress [██████████████████████████████████████████████████] 100% (5000/5000)
Split complete!
Generated 5 files
```

## 🔗 pjoin - File Merge

```bash
$ pjoin -o combined.txt -m header part.*
Starting file merge (with headers)...
Output file: combined.txt
File count: 3
=== part.001 ===
   1 | First line content
   2 | Second line content
==================================================

=== part.002 ===
   1 | Third line content
   2 | Fourth line content
==================================================
Merge complete!
```

## 🔍 pdiff - File Comparison

```bash
$ pdiff -s old.txt new.txt
File Comparison: old.txt vs new.txt
====================================
   1 |    1 |  First line content
   2 |    2 |  Second line content
   3 |  -   | -Third line content
   4 |    3 | +New third line content
   4 |    4 |  Fourth line content

Difference Statistics:
Added: 1 line
Deleted: 1 line
Modified: 0 lines
Same: 3 lines
Total changes: 2 lines
```

## 📦 pzip - Compression

```bash
$ pzip -c -v archive.zip file1.txt file2.txt
Starting file compression...
Compressed file: archive.zip
File count: 2
Compression level: 6
Compressing: file1.txt (1.2KB -> 0.8KB)
Compressing: file2.txt (2.5KB -> 1.9KB)
Compression complete!
Original size: 3.7KB
Compressed size: 2.7KB
Compression ratio: 73%
```

## 📋 pcp - File Copy

```bash
$ pcp -r -v source/ dest/
Starting file copy...
Destination: dest/
Source file count: 3
Creating directory: dest/source
Copying file: source/file1.txt -> dest/source/file1.txt (1.2KB)
Copying file: source/file2.txt -> dest/source/file2.txt (2.5KB)
Copying file: source/subdir/file3.txt -> dest/source/subdir/file3.txt (0.8KB)

Copy Statistics:
Files copied: 3
Directories created: 1
Data copied: 4.5KB
Copy complete!
```

## 🚚 pmv - File Move

```bash
$ pmv -i -v old.txt new.txt
Move old.txt to new.txt? (y/n): y
Moving file: old.txt -> new.txt

Move Statistics:
Files moved: 1
Directories moved: 0
Data moved: 1.2KB
Move complete!
```

## 📁 ptar - Archive Tool

```bash
$ ptar -cvf archive.tar file1.txt file2.txt
Creating archive: archive.tar
Adding: file1.txt
Adding: file2.txt
[Success] Archive creation complete

$ ptar -tf archive.tar
Archive Contents: archive.tar
====================================
rwxrwxrwx      50 2025-09-21 23:07 file1.txt
rwxrwxrwx      53 2025-09-21 23:08 file2.txt

Total: 2 entries

$ ptar -xf archive.tar
Extracting archive: archive.tar
Extracting: file1.txt
Extracting: file2.txt
Extraction complete: 2 files
```

> 📖 For more detailed usage instructions, see [Usage Tutorial](usage.md)

