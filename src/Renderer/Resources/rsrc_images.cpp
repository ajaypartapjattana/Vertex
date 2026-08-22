#include "rsrc_images.h"

rsrc_images::rsrc_images(const rhi::device& device, const rhi::allocator& allocator, sgb::vltWrite<rhi::pmvImage> imageWrite)
	:
	m_allocator(allocator),
	_wrt(imageWrite)
{

}

uint32_t rsrc_images::createImage(const createInfo& info) {
	m_allocator.format(info.format);
	m_allocator.extent(info.extent);
	m_allocator.usage(info.usage);

	m_allocator.memory(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	auto _ctor = [&](rhi::pmvImage* i) {
		m_allocator.publish(*i);
	};

	return _wrt.construct(_ctor);

}

