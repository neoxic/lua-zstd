local zstd = require 'zstd'

local e = {'abc', 'de', 'fghi', 'jklm', 'no', 'p', 'qrstu', 'vx', 'yz', '1234', '567', '89', '0'}
local function randstr(n)
	local l = 0
	local t = {}
	for i = 1, math.random(1, n) do
		local s = e[math.random(#e)]
		l = l + #s
		if l > n then
			break
		end
		t[i] = s
	end
	return table.concat(t)
end

local f = assert(io.open(os.getenv('SOURCE_DIR') .. '/test/test.dict', 'r'))
local dict = f:read(999999)
f:close()

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
		if math.random() < 0.5 then -- Use dictionary
			cctx:refCDict(zstd.CDict(dict, cctxparams))
			dctx:refDDict(zstd.DDict(dict))
		end
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

-----------------------------------------
-- Stateless compression/decompression --
-----------------------------------------

local cctxparams = zstd.CCtxParams()
local cctx = zstd.CCtx()
local dctx = zstd.DCtx()

for i = 1, 10 do
	local level = math.random(1, 10)
	local wlog = math.random(10, 13) -- Window can't be larger than the dictionary

	cctxparams:set('compressionLevel', level)
	cctxparams:set('windowLog', wlog)
	dctx:setParameter('windowLogMax', wlog)

	local cdict = zstd.CDict(dict, cctxparams)
	local ddict = zstd.DDict(dict)

	local m, n = 0, 0
	for i = 1, 100 do
		local s1 = randstr(2 ^ wlog)
		local s2 = assert(cctx:compressBlock(s1, cdict))
		if math.random() < 0.5 and s2 ~= '' then
			local s3 = assert(dctx:decompressBlock(s2, ddict))
			assert(s1 == s3)
			m = m + 1
		else
			n = n + 1
		end
	end
	assert(m > 0 and n > 0)

	cctx:reset('all')
	dctx:reset('all')
end
