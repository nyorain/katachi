// Copyright (c) 2018-2020 Jan Kelling
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#include <katachi/stroke.hpp>
#include <katachi/path.hpp>
#include <nytl/vecOps.hpp>
#include <nytl/approxVec.hpp>
#include <dlg/dlg.hpp>

// We assume a bottom-left-origin coordinate system in the code.
// But all functions that depend on it should test for the winding
// order anyways and work for both.

namespace ktc {

/// Returns the left normal of a 2 dimensional vector.
template<typename T>
Vec2<T> lnormal(Vec2<T> vec) {
	return {-vec[1], vec[0]};
}

/// Returns the right normal of a 2 dimensional vector.
template<typename T>
Vec2<T> rnormal(Vec2<T> vec) {
	return {vec[1], -vec[0]};
}

void bakeStroke(Span<const Vec2f> points, const StrokeSettings& settings,
		Span<const Vec4u8> color, const VertexHandlerFn& handler) {

	dlg_assert(settings.width > 0.f);
	dlg_assert(handler);

	if(points.size() < 2) {
		return;
	}

	auto iwidth = settings.width * (0.5f + 0.5f * settings.extrude);
	auto owidth = settings.width * (0.5f - 0.5f * settings.extrude);
	iwidth += 0.5 * settings.fringe; // half extrude, half inside
	owidth += 0.5 * settings.fringe; // half extrude, half inside

	// we will in the following assume that the points are ordered
	// counter-clockwise
	if(area(points) < 0.0) {
		std::swap(iwidth, owidth);
		iwidth *= -1;
		owidth *= -1;
	}

	auto p0 = points.back();
	auto p1 = points.front();
	auto p2 = points[1];

	// start cap
	auto start = 0u;
	auto end = points.size() + settings.loop;
	auto capFringe = settings.capFringe * 0.5f;
	if(!settings.loop && capFringe > 0.f) {
		start = 1u;
		end = points.size() - 1;

		auto xextrusion = normalized(p2 - p1);
		auto yextrusion = rnormal(xextrusion);
		auto c = color.size() > 0 ? color[0] : Vec4u8{0, 0, 0, 255};
		handler({
			p1 - capFringe * xextrusion + owidth * yextrusion,
			{0.f, 1.f}, c
		});
		handler({
			p1 - capFringe * xextrusion - iwidth * yextrusion,
			{0.f, -1.f}, c
		});

		handler({
			p1 + capFringe * xextrusion + owidth * yextrusion,
			{1.f, 1.f}, c
		});
		handler({
			p1 + capFringe * xextrusion - iwidth * yextrusion,
			{1.f, -1.f}, c
		});

		p0 = p1;
		p1 = p2;
		p2 = points[2 % points.size()];

	}

	for(auto i = start; i < end; ++i) {
		auto d0 = rnormal(p1 - p0);
		auto d1 = rnormal(p2 - p1);

		if(i == 0 && !settings.loop) {
			d0 = d1;
		} else if(i == points.size() - 1 && !settings.loop) {
			d1 = d0;
		}

		// skip point if same to next or previous one
		// this assures normalized below will not throw (for nullvector)
		if(d0 == approx(Vec{0.f, 0.f}) || d1 == approx(Vec{0.f, 0.f})) {
			dlg_debug("ktc::bakeStroke: doubled point {}", p1);
			p1 = p2;
			p2 = points[i + 2 % points.size()];
			continue;
		}

		auto extrusion = 0.5f * (normalized(d0) + normalized(d1));
		extrusion *= 1.f / dot(extrusion, extrusion);

		handler({
			p1 + owidth * extrusion,
			{1.f, 1.f},
			color.size() > i ? color[i] : Vec4u8{0, 0, 0, 255}
		});
		handler({
			p1 - iwidth * extrusion,
			{1.f, -1.f},
			color.size() > i ? color[i] : Vec4u8{0, 0, 0, 255}
		});

		p0 = points[(i + 0) % points.size()];
		p1 = points[(i + 1) % points.size()];
		p2 = points[(i + 2) % points.size()];
	}

	// end cap
	if(!settings.loop && settings.capFringe > 0.f) {
		auto i = points.size() - 1;
		auto c = color.size() > i ? color[i] : Vec4u8{0, 0, 0, 255};
		auto xextrusion = normalized(p1 - p0);
		auto yextrusion = rnormal(xextrusion);
		handler({
			p1 - capFringe * xextrusion + owidth * yextrusion,
			{1.f, 1.f}, c
		});
		handler({
			p1 - capFringe * xextrusion - iwidth * yextrusion,
			{1.f, -1.f}, c
		});

		handler({
			p1 + capFringe * xextrusion + owidth * yextrusion,
			{0.f, 1.f}, c
		});
		handler({
			p1 + capFringe * xextrusion - iwidth * yextrusion,
			{0.f, -1.f}, c
		});
	}
}

void bakeFillAA(Span<const Vec2f> points, Span<const Vec4u8> color,
		float fringe, const VertexHandlerFn& fill,
		const VertexHandlerFn& stroke) {
	dlg_assert(fringe > 0.f);
	dlg_assert(fill);
	dlg_assert(stroke);

	if(points.size() < 2) {
		return;
	}

	auto loop = points.front() == points.back();
	if(loop) {
		points = points.first(points.size() - 1);
	}

	fringe *= 0.5f;
	if(area(points) < 0.0) {
		fringe *= -1;
	}

	auto p0 = points.back();
	auto p1 = points.front();
	auto p2 = points[1];

	for(auto i = 0u; i < points.size() + loop; ++i) {
		auto d0 = rnormal(p1 - p0);
		auto d1 = rnormal(p2 - p1);

		if(i == 0 && !loop) {
			d0 = d1;
		} else if(i == points.size() - 1 && !loop) {
			d1 = d0;
		}

		// skip point if same to next or previous one
		// this assures normalized below will not throw (for nullvector)
		if(d0 == approx(Vec {0.f, 0.f}) || d1 == approx(Vec {0.f, 0.f})) {
			dlg_debug("ktc::bakeFillAA: doubled point {}", p1);
			p1 = p2;
			p2 = points[i + 2 % points.size()];
			continue;
		}

		// fill
		auto extrusion = 0.5f * (normalized(d0) + normalized(d1));
		extrusion *= 1.f / dot(extrusion, extrusion);

		fill({
			p1 - fringe * extrusion,
			{1.f, 0.f},
			color.size() > i ? color[i] : Vec4u8{0, 0, 0, 255}
		});

		// stroke
		stroke({
			p1 - fringe * extrusion,
			{1.f, 0.f},
			color.size() > i ? color[i] : Vec4u8{0, 0, 0, 255}
		});

		stroke({
			p1 + fringe * extrusion,
			{1.f, 1.f},
			color.size() > i ? color[i] : Vec4u8{0, 0, 0, 255}
		});

		p0 = points[(i + 0) % points.size()];
		p1 = points[(i + 1) % points.size()];
		p2 = points[(i + 2) % points.size()];
	}
}

void bakeStroke(Span<const Vec2f> points, const StrokeSettings& settings,
		const VertexHandlerFn& handler) {
	bakeStroke(points, settings, {}, handler);
}

void bakeColoredStroke(Span<const Vec2f> points, Span<const Vec4u8> color,
		const StrokeSettings& settings, const VertexHandlerFn& handler) {
	dlg_assertl(dlg_level_debug, color.size() == points.size());
	bakeStroke(points, settings, color, handler);
}

void bakeFillAA(Span<const Vec2f> points, float fringe,
		const VertexHandlerFn& fill, const VertexHandlerFn& stroke) {
	bakeFillAA(points, {}, fringe, fill, stroke);
}

void bakeColoredFillAA(Span<const Vec2f> points, Span<const Vec4u8> color,
		float fringe, const VertexHandlerFn& fill,
		const VertexHandlerFn& stroke) {
	dlg_assertl(dlg_level_debug, color.size() == points.size());
	bakeFillAA(points, color, fringe, fill, stroke);
}

float area(Span<const Vec2f> points) {
	float ret = 0.f;
	for(auto i = 2u; i < points.size(); ++i) {
		auto ab = points[i - 1] - points[0];
		auto ac = points[i - 0] - points[0];
		ret += cross(ab, ac);
	}

	return 0.5f * ret;
}

float enforceWinding(Span<Vec2f> points, bool clockwise) {
	auto a = area(points);
	if((a > 0.f) != clockwise) {
		std::reverse(points.begin(), points.end());
	}

	return a;
}


template<typename T>
void triangleFanIndices(Span<T> ret, unsigned count) {
	for(auto i = 2u; i < count; ++i) {
		ret[3 * i + 0] = 0;
		ret[3 * i + 1] = i - 1;
		ret[3 * i + 2] = i - 0;
	}
}

template<typename T>
void triangleStripIndices(Span<T> ret, unsigned count) {
	for(auto i = 2u; i < count; ++i) {
		ret[3 * i + 0] = i - 2;
		ret[3 * i + 1] = i - 1;
		ret[3 * i + 2] = i - 0;
	}
}

template<typename T>
std::vector<T> triangleFanIndices(unsigned count) {
	if(count < 3) {
		return {};
	}

	std::vector<T> ret;
	ret.resize((count - 2) * 3);
	triangleFanIndices<T>(ret, count);
	return ret;
}

template<typename T>
std::vector<T> triangleStripIndices(unsigned count) {
	if(count < 3) {
		return {};
	}

	std::vector<T> ret;
	ret.resize((count - 2) * 3);
	triangleStripIndices<T>(ret, count);
	return ret;
}

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

template void triangleFanIndices<u8>(Span<u8>, unsigned count);
template void triangleFanIndices<u16>(Span<u16>, unsigned count);
template void triangleFanIndices<u32>(Span<u32>, unsigned count);
template void triangleFanIndices<u64>(Span<u64>, unsigned count);
template std::vector<u8> triangleFanIndices<u8>(unsigned count);
template std::vector<u16> triangleFanIndices<u16>(unsigned count);
template std::vector<u32> triangleFanIndices<u32>(unsigned count);
template std::vector<u64> triangleFanIndices<u64>(unsigned count);

template void triangleStripIndices<u8>(Span<u8>, unsigned count);
template void triangleStripIndices<u16>(Span<u16>, unsigned count);
template void triangleStripIndices<u32>(Span<u32>, unsigned count);
template void triangleStripIndices<u64>(Span<u64>, unsigned count);
template std::vector<u8> triangleStripIndices<u8>(unsigned count);
template std::vector<u16> triangleStripIndices<u16>(unsigned count);
template std::vector<u32> triangleStripIndices<u32>(unsigned count);
template std::vector<u64> triangleStripIndices<u64>(unsigned count);

CombinedFill bakeCombinedFillAA(Span<const Vec2f> points,
		Span<const Vec4u8> color, float fringe) {
	dlg_assert(fringe > 0.f);

	if(points.size() < 2) {
		return {};
	}

	auto loop = true; // points.front() == points.back(); // TODO
	if(loop && points.front() == points.back()) {
		points = points.first(points.size() - 1);
	}

	fringe *= 0.5f;
	if(area(points) < 0.0) {
		fringe *= -1;
	}

	auto p0 = points.back();
	auto p1 = points.front();
	auto p2 = points[1];

	CombinedFill ret;
	for(auto i = 0u; i < points.size() + loop; ++i) {
		auto d0 = rnormal(p1 - p0);
		auto d1 = rnormal(p2 - p1);

		if(i == 0 && !loop) {
			d0 = d1;
		} else if(i == points.size() - 1 && !loop) {
			d1 = d0;
		}

		// skip point if same to next or previous one
		// this assures normalized below will not throw (for nullvector)
		if(d0 == approx(Vec {0.f, 0.f}) || d1 == approx(Vec {0.f, 0.f})) {
			dlg_debug("ktc::bakeFillAA: doubled point {}", p1);
			p1 = p2;
			p2 = points[i + 2 % points.size()];
			continue;
		}

		// fill
		auto extrusion = 0.5f * (normalized(d0) + normalized(d1));
		extrusion *= 1.f / dot(extrusion, extrusion);

		ret.vertices.push_back({
			p1 - fringe * extrusion,
			{1.f, 0.f},
			color.size() > i ? color[i] : Vec4u8{0, 0, 0, 255}
		});

		if(i >= 2) {
			// triangle fan
			ret.indices.push_back(0); // first fill vertex
			ret.indices.push_back(2 * i - 2); // previous fill vertex
			ret.indices.push_back(2 * i); // current fill vertex
		}

		// stroke
		ret.vertices.push_back({
			p1 + fringe * extrusion,
			{1.f, 1.f},
			color.size() > i ? color[i] : Vec4u8{0, 0, 0, 255}
		});

		if(i >= 1) {
			// triangle strip, we need 2 triangles for one stroke segment
			ret.indices.push_back(2 * i - 2); // previous fill vertex
			ret.indices.push_back(2 * i - 1); // previous stroke vertex
			ret.indices.push_back(2 * i + 0); // current fill vertex

			ret.indices.push_back(2 * i - 1); // previous stroke vertex
			ret.indices.push_back(2 * i + 1); // current stroke vertex
			ret.indices.push_back(2 * i + 0); // current fill vertex
		}

		p0 = points[(i + 0) % points.size()];
		p1 = points[(i + 1) % points.size()];
		p2 = points[(i + 2) % points.size()];
	}

	return ret;
}

} // namespace ktc
