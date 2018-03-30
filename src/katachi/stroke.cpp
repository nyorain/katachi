// Copyright (c) 2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#include <katachi/stroke.hpp>
#include <katachi/path.hpp>
#include <nytl/vecOps.hpp>
#include <nytl/approxVec.hpp>
#include <dlg/dlg.hpp>

namespace ktc {

using namespace nytl::rho;

// stroke api
std::vector<Vec2f> bakeStroke(Span<const Vec2f> p, const StrokeSettings& s) {
	std::vector<Vec2f> ret;
	bakeStroke(p, s, ret);
	return ret;
}

std::vector<Vec2f> bakeStroke(const Subpath& sub, const StrokeSettings& s) {
	auto points = flatten(sub);
	return bakeStroke(points, s);
}

void bakeStroke(Span<const Vec2f> points, const StrokeSettings& settings,
		std::vector<Vec2f>& ret) {

	dlg_assert(settings.width > 0.f);

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

		ret.push_back(p1 + width * extrusion);
		ret.push_back(p1 - width * extrusion);

		p0 = points[(i + 0) % points.size()];
		p1 = points[(i + 1) % points.size()];
		p2 = points[(i + 2) % points.size()];
	}
}

void bakeColoredStroke(Span<const Vec2f> points, Span<const Vec4u8> color,
		const StrokeSettings& settings, std::vector<Vec2f>& outPoints,
		std::vector<Vec4u8>& outColor) {

	dlg_assert(color.size() == points.size());
	dlg_assert(settings.width > 0.f);

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
			p1 = p2;
			p2 = points[i + 2 % points.size()];
			continue;
		}

		auto extrusion = 0.5f * (normalized(d0) + normalized(d1));

		outPoints.push_back(p1 + width * extrusion);
		outPoints.push_back(p1 - width * extrusion);

		outColor.push_back(color[i]);
		outColor.push_back(color[i]);

		p0 = points[i];
		p1 = points[i + 1 % points.size()];
		p2 = points[i + 2 % points.size()];
	}
}

} // namespace ktc
