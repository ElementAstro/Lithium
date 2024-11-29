# Example 1: Configure libclang with a custom path
# This example configures libclang using a custom path to the libclang library.
$ python libclang_finder.py configure --path /custom/path/to/libclang.so

# Example 2: Configure libclang and clear the cache
# This example configures libclang and clears any cached libclang path.
$ python libclang_finder.py configure --clear-cache

# Example 3: Configure libclang with additional search patterns
# This example configures libclang and includes additional glob patterns to search for the libclang library.
$ python libclang_finder.py configure --search-patterns "/opt/llvm/lib/libclang.so*" "/another/path/libclang.so*"

# Example 4: List available libclang versions
# This example lists all available libclang versions on the system.
$ python libclang_finder.py list