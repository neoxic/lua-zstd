Main module table
=================

```Lua
local zstd = require 'zstd'
```


Functions
---------

### zstd.compress(data, [level])
Compresses `data` as a single frame and returns the result. On error, returns `nil` and the error message. Optional `level` can be used to override the default compression level.

### zstd.decompress(data)
Decompresses `data` and returns the result. On error, returns `nil` and the error message.

### zstd.isFrame(data)
Checks if `data` starts with a valid frame identifier and returns a boolean result.

### zstd.getFrameContentSize(data)
Returns the size of _decompressed_ content in `data`. On error, returns `nil` and the error message.


Constructors
------------

### zstd.CCtx()
Returns an instance of [Compression Context].

### zstd.CCtxParams()
Returns an instance of [Compression Context Parameters].

### zstd.CDict(data, cctxparams)
Returns an instance of [Compression Dictionary].

### zstd.DCtx()
Returns an instance of [Decompression Context].

### zstd.DDict(data)
Returns an instance of [Decompression Dictionary].


[Compression Context]: cctx.md
[Compression Context Parameters]: cctxparams.md
[Compression Dictionary]: cdict.md
[Decompression Context]: dctx.md
[Decompression Dictionary]: ddict.md
