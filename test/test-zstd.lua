local zstd = require 'zstd'

local function randstr(n)
	local t = {}
	for i = 1, math.random(1, n) do
		t[i] = string.char(math.random(0, 255))
	end
	return table.concat(t)
end

math.randomseed(os.time())

--------------------------------------
-- Simple compression/decompression --
--------------------------------------

for i = 1, 10 do
	local d = randstr(100000)
	local c = assert(zstd.compress(d))
	assert(zstd.isFrame(d) == false)
	assert(zstd.isFrame(c) == true)
	local n, e = zstd.getFrameContentSize(d)
	assert(n == nil and e == 'invalid frame data')
	assert(zstd.getFrameContentSize(c) == #d)
	assert(zstd.decompress(c) == d)
end

-----------------------------------------
-- Streaming compression/decompression --
-----------------------------------------

local cctxparams = zstd.CCtxParams()
local cctx = zstd.CCtx()
local dctx = zstd.DCtx()

for i = 1, 10 do
	local level = math.random(1, 10)
	local wlog = math.random(10, 20)
	if math.random() < 0.5 then -- Use CCtxParams
		cctxparams:set('compressionLevel', level)
		cctxparams:set('windowLog', wlog)
		cctx:setParameters(cctxparams)
	else
		cctx:setParameter('compressionLevel', level)
		cctx:setParameter('windowLog', wlog)
	end
	dctx:setParameter('windowLogMax', wlog)

	local t1 = {}
	local t2 = {}
	local t3 = {}
	for i = 1, 100 do
		local s1 = randstr(1000)
		local s2 = assert(cctx:compressStream(s1, math.random() < 0.5 and 'flush' or 'continue'))
		t1[#t1 + 1] = s1
		if s2 ~= '' then
			t2[#t2 + 1] = s2
			t3[#t3 + 1] = assert(dctx:decompressStream(s2))
		end
	end
	local s2 = assert(cctx:compressStream('', 'end')) -- Finalize frame
	local s3, e = assert(dctx:decompressStream(s2))
	assert(e == 'end')
	t2[#t2 + 1] = s2
	t3[#t3 + 1] = s3
	local s1 = table.concat(t1)
	local s2 = table.concat(t2)
	local s3 = table.concat(t3)
	assert(s1 == s3)
	assert(zstd.isFrame(s1) == false)
	assert(zstd.isFrame(s2) == true)
	local s, e = zstd.getFrameContentSize(s2)
	assert(s == nil and e == 'unknown content size') -- Content size is unknown unless 'setPledgedSrcSize' is called

	cctx:reset('all')
	dctx:reset('all')
end
