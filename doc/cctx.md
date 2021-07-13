Compression Context
===================

Methods
-------

### cctx:getParameter(name)
Returns the current value of parameter `name` (see below).

### cctx:setParameter(name, value)
Sets a new `value` of parameter `name` (see below). Setting a value that is out of bounds will either clamp it or trigger an error (depending on parameter).

### cctx:setParameters(cctxparams)
Applies a set of parameters from [Compression Context Parameters] `cctxparams`.

### cctx:setPledgedSrcSize(size)
Sets content size to be compressed as a single frame (`size = -1` means that content size is unknown).

### cctx:refCDict(cdict)
References [Compression Dictionary] `cdict` to be used for compression of all next frames in stream `dctx`.

### cctx:compressStream(data, [op])
Consumes `data` as input for stream `cctx` and returns some compressed data (empty string if no output is currently possible). On error, returns `nil` and the error message. Operation `op` (a string) can be one of the following:
- `continue`: consume input, flush output only if necessary for optimal compression ratio (default);
- `flush`: consume input, flush as much output as possible;
- `end`: consume input, flush all output, close the current frame;

### cctx:compressBlock(data, cdict)
Compresses `data` _statelessly_ using [Compression Dictionary] `cdict` and returns a compressed block (empty string if data can't be compressed). On error, returns `nil` and the error message. This call is useful for compressing small chunks of data without metadata overhead and compression history, e.g. UDP datagrams.

### cctx:reset([mode])
Resets context `cctx` according to `mode` (a string) that can be one of the following:
- `session`: session only (default);
- `params`: parameters only;
- `all`: session and parameters;


Parameters
----------

- `compressionLevel`
- `windowLog`
- `hashLog`
- `chainLog`
- `searchLog`
- `minMatch`
- `targetLength`
- `strategy`
- `enableLongDistanceMatching`
- `ldmHashLog`
- `ldmMinMatch`
- `ldmBucketSizeLog`
- `ldmHashRateLog`
- `contentSizeFlag`
- `checksumFlag`
- `dictIDFlag`
- `nbWorkers`
- `jobSize`
- `overlapLog`
- `rsyncable`
- `format`
- `forceMaxWindow`
- `forceAttachDict`
- `literalCompressionMode`
- `targetCBlockSize`
- `srcSizeHint`
- `enableDedicatedDictSearch`
- `stableInBuffer`
- `stableOutBuffer`
- `blockDelimiters`
- `validateSequences`

Refer to http://facebook.github.io/zstd/zstd_manual.html#Chapter5 for more information.


[Compression Context Parameters]: cctxparams.md
[Compression Dictionary]: cdict.md
