This folder should contain the Windows mimalloc static library used by the
node addon build.

Place the file at:

  deps/mimalloc/lib/mimalloc.lib

You can override the location by setting the environment variable
MIMALLOC_LIB_PATH before building, for example:

  MIMALLOC_LIB_PATH=C:\path\to\mimalloc\lib
