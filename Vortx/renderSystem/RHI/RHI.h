#pragma once

class RHiBuffer {
public:
	virtual ~RHiBuffer() = default;
};

class RHiPipeline {
public:
	virtual ~RHiPipeline() = default;
};

class RHiCommandBuffer {
public:
	virtual void begin() = 0;
	virtual void end() = 0;

};

class RHI {
public:
	virtual RHiBuffer* createBuffer() = 0;
	virtual RHiPipeline* createPipeline() = 0;
	virtual RHiCommandBuffer* createCommandBuffer() = 0;
};