Decompression Context
=====================

Methods
-------

### dctx:getParameter(name)
Returns the current value of parameter `name` (see below).

### dctx:setParameter(name, value)
Sets a new `value` of parameter `name` (see below). Setting a value that is out of bounds will either clamp it or trigger an error (depending on parameter).

### dctx:refDDict(ddict)
References [Decompression Dictionary] `ddict` to be used for decompression of all next frames in stream `dctx`.

### dctx:decompressStream(data)
Consumes `data` as input for stream `dctx` and returns some decompressed data (empty string if no output is currently possible). Additional literal `end` is returned as a second result at the end of each frame. On error, returns `nil` and the error message.

### dctx:decompressBlock(data, ddict)
Decompresses block `data` _statelessly_ using [Decompression Dictionary] `ddict` and returns the result. On error, returns `nil` and the error message.

### dctx:reset([mode])
Resets context `dctx` according to `mode` (a string) that can be one of the following:
- `session`: session only (default);
- `params`: parameters only;
- `all`: session and parameters;


Parameters
----------

- `windowLogMax`
- `format`
- `stableOutBuffer`
- `forceIgnoreChecksum`
- `refMultipleDDicts`

Refer to http://facebook.github.io/zstd/zstd_manual.html#Chapter6 for more information.


[Decompression Dictionary]: ddict.md
