// Copyright (c) 2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#include <katachi/stroke.hpp>
#include <katachi/path.hpp>
#include <nytl/vecOps.hpp>
#include <nytl/approxVec.hpp>
#include <dlg/dlg.hpp>

namespace ktc {
namespace {

void bake(Span<const Vec2f> points, const StrokeSettings& settings,
		Span<const Vec4u8> color, const VertexHandlerFn& handler) {

	using namespace nytl::rho;
	dlg_assert(settings.width > 0.f);
	dlg_assert(handler);

	auto width = settings.width * 0.5f;
	if(points.size() < 2) {
		return;
	}

	auto loop = points.front() == points.back();
	if(loop) {
		points = points.slice(0, points.size() - 1);
	}

	auto p0 = points.back();
	auto p1 = points.front();
	auto p2 = points[1];

	for(auto i = 0u; i < points.size() + loop; ++i) {
		auto d0 = lnormal(p1 - p0);
		auto d1 = lnormal(p2 - p1);

		if(i == 0 && !loop) {
			d0 = d1;
		} else if(i == points.size() - 1 && !loop) {
			d1 = d0;
		}

		// skip point if same to next or previous one
		// this assures normalized below will not throw (for nullvector)
		if(d0 == approx(Vec {0.f, 0.f}) || d1 == approx(Vec {0.f, 0.f})) {
			dlg_debug("bakeStroke: doubled point {}", p1);
			p1 = p2;
			p2 = points[i + 2 % points.size()];
			continue;
		}

		auto extrusion = 0.5f * (normalized(d0) + normalized(d1));
		handler({
			p1 + width * extrusion,
			{0.f, 1.f},
			color.size() > i ? color[i] : Vec4u8 {0, 0, 0, 255}
		});
		handler({
			p1 - width * extrusion,
			{0.f, -1.f},
			color.size() > i ? color[i] : Vec4u8 {0, 0, 0, 255}
		});

		p0 = points[(i + 0) % points.size()];
		p1 = points[(i + 1) % points.size()];
		p2 = points[(i + 2) % points.size()];
	}
}

} // anon namespace

void bakeStroke(Span<const Vec2f> points, const StrokeSettings& settings,
		const VertexHandlerFn& handler) {
	bake(points, settings, {}, handler);
}

void bakeColoredStroke(Span<const Vec2f> points, Span<const Vec4u8> color,
		const StrokeSettings& settings, const VertexHandlerFn& handler) {
	dlg_assertl(dlg_level_debug, color.size() == points.size());
	bake(points, settings, color, handler);
}

} // namespace ktc
