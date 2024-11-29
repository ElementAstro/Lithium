# Example 1: Convert a binary file to a C header file with default settings
# This example converts my_binary.bin to my_header.h using the default array name, size variable name, and data format (hex).
# By default, the generated header file will include #ifndef protection macros.
$ python3 byte_convertor.py to_header my_binary.bin my_header.h

# Example 2: Convert a binary file to a C header file with compression
# This example converts my_binary.bin to my_header.h and compresses the data during conversion.
$ python3 byte_convertor.py to_header my_binary.bin my_header.h --compress

# Example 3: Convert a binary file to a C header file with custom array and size variable names
# This example converts my_binary.bin to my_header.h using custom array name my_array and size variable name my_size.
$ python3 byte_convertor.py to_header my_binary.bin my_header.h --array_name my_array --size_name my_size

# Example 4: Convert a binary file to a C header file using base64 data format
# This example converts my_binary.bin to my_header.h using base64 data format.
$ python3 byte_convertor.py to_header my_binary.bin my_header.h --format base64

# Example 5: Convert a binary file to a C header file and generate a C++ class wrapper
# This example converts my_binary.bin to my_header.h and generates a C++ class wrapper named DataWrapper.
$ python3 byte_convertor.py to_header my_binary.bin my_header.h --cpp_class DataWrapper

# Example 6: Convert a binary file to a C header file and split into multiple header files
# This example converts large_binary.bin into multiple header files, each with a maximum size of 1024 bytes.
$ python3 byte_convertor.py to_header large_binary.bin --split_size 1024

# Example 7: Convert a binary file to a C header file and verify data integrity
# This example converts my_binary.bin to my_header.h and verifies data integrity after conversion.
$ python3 byte_convertor.py to_header my_binary.bin my_header.h --verify

# Example 8: Convert a C header file back to a binary file
# This example converts my_header.h back to my_binary.bin.
$ python3 byte_convertor.py to_file my_header.h my_binary.bin

# Example 9: Convert a C header file back to a binary file with decompression
# This example converts my_header.h back to my_binary.bin and decompresses the data during conversion.
$ python3 byte_convertor.py to_file my_header.h my_binary.bin --decompress

# Example 10: Convert a C header file back to a binary file and verify data integrity
# This example converts my_header.h back to my_binary.bin and verifies data integrity after conversion.
$ python3 byte_convertor.py to_file my_header.h my_binary.bin --verify