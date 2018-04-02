// Copyright (c) 2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#include <katachi/curves.hpp>
#include <nytl/math.hpp>
#include <nytl/vecOps.hpp>
#include <cmath>

namespace ktc {
namespace {

/// 2-dimensional cross product.
/// Is the same as the dot of a with the normal of b.
template<typename T>
constexpr T cross(nytl::Vec<2, T> a, nytl::Vec<2, T> b) {
    return a[0] * b[1] - a[1] * b[0];
}

/// Returns the point on the unit circle for the given angle.
Vec2f unitCirclePoint(float angle) {
	return {std::cos(angle), std::sin(angle)};
}

/// Simple Paul de Casteljau implementation.
/// See antigrain.com/research/adaptive_bezier/
void subdivide(const CubicBezier& bezier, unsigned maxlvl, unsigned lvl,
		std::vector<nytl::Vec2f>& points, float minSubdiv) {

	if (lvl > maxlvl) {
		return;
	}

	auto p1 = bezier.start;
	auto p2 = bezier.control1;
	auto p3 = bezier.control2;
	auto p4 = bezier.end;

	auto p12 = 0.5f * (p1 + p2);
	auto p23 = 0.5f * (p2 + p3);
	auto p34 = 0.5f * (p3 + p4);
	auto p123 = 0.5f * (p12 + p23);

	auto d = p4 - p1;
	auto d2 = std::abs(cross(p2 - p4, d));
	auto d3 = std::abs(cross(p3 - p4, d));

	if((d2 + d3) * (d2 + d3) <= minSubdiv * dot(d, d)) {
		points.push_back(p4);
		return;
	}

	auto p234 = 0.5f * (p23 + p34);
	auto p1234 = 0.5f * (p123 + p234);

	subdivide({p1, p12, p123, p1234}, maxlvl, lvl + 1, points, minSubdiv);
	subdivide({p1234, p234, p34, p4}, maxlvl, lvl + 1, points, minSubdiv);
}

} // anon namespace

// stackoverflow.com/questions/3162645/convert-a-quadratic-bezier-to-a-cubic
CubicBezier quadToCubic(const QuadBezier& b) {
	return {b.start,
		b.start + (2 / 3.f) * (b.control - b.start),
		b.end + (2 / 3.f) * (b.control - b.end),
		b.end};
}

void flatten(const CubicBezier& bezier, std::vector<Vec2f>& p,
		unsigned maxLevel, float minDist) {
	subdivide(bezier, maxLevel, 0, p, minDist);
}

void flatten(const QuadBezier& bezier, std::vector<Vec2f>& p,
		unsigned maxLevel, float minDist) {
	return flatten(quadToCubic(bezier), p, maxLevel, minDist);
}

// Arc implementations from
// www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
void flatten(const CenterArc& arc, std::vector<Vec2f>& points, unsigned steps) {
	using namespace nytl::vec::cw::operators;

	// Currently no x-axis rotation possible
	auto delta = arc.end - arc.start;
	points.reserve(points.size() + steps);
	auto r = Vec {std::abs(arc.radius.x), std::abs(arc.radius.y)};
	for(auto i = 1u; i <= steps; ++i) {
		auto angle = arc.start + i * (delta / steps);
		points.push_back(r * unitCirclePoint(angle) + arc.center);
	}
}

// https://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
CenterArc endToCenter(const EndArc& arc) {
	auto r = Vec2f {std::abs(arc.radius.x), std::abs(arc.radius.y)};

	// step 1 (p = (x', y'))
	auto p = 0.5f * (arc.from - arc.to);
	if(p == Vec2f {0.f, 0.f}) {
		// endpoints identical: emit the arc completly
		return {{}, r, 0.f, 0.f};
	}

	// squared values
	auto rxs = r.x * r.x, rys = r.y * r.y, pys = p.y * p.y, pxs = p.x * p.x;

	// step 1.5: correct radi (see F.6.6)
	auto a = pxs / rxs + pys / rys;
	if(a > 1) {
		r *= std::sqrt(a);
		rxs = r.x * r.x;
		rys = r.y * r.y;
	}

	// step2 (tc = (cx', cy'))
	auto inner = (rxs * rys - rxs * pys - rys * pxs) / (rxs * pys + rys * pxs);
	auto sign = (arc.largeArc != arc.clockwise) ? 1 : -1;
	auto mult = Vec {r.x * p.y / r.y, -r.y * p.x / r.x};
	auto tc = sign * std::sqrt(inner) * mult;

	// step3: center
	auto c = tc;
	c += Vec2f{(arc.from.x + arc.to.x) / 2, (arc.from.y + arc.to.y) / 2};

	// step4: angles
	auto vec1 = Vec {(p.x - tc.x) / r.x, (p.y - tc.y) / r.y};
	auto vec2 = Vec {(-p.x - tc.x) / r.x, (-p.y - tc.y) / r.y};
	auto angle1 = angle(Vec2f{1, 0}, vec1);
	auto delta = float(std::fmod(angle(vec1, vec2), 2 * nytl::constants::pi));

	if(!arc.clockwise && delta > 0) {
		delta -= 2 * nytl::constants::pi;
	} else if(arc.clockwise && delta < 0) {
		delta += 2 * nytl::constants::pi;
	}

	return {c, arc.radius, angle1, angle1 + delta};
}

EndArc centerToEnd(const CenterArc& arc) {
	using namespace nytl::vec::cw::operators;

	auto ret = EndArc { arc.center, arc.center, arc.radius, {}, {}};
	ret.from += arc.radius * Vec2f {std::cos(arc.start), std::sin(arc.start)};
	ret.to += arc.radius * Vec2f {std::cos(arc.end), std::sin(arc.end)};
	ret.largeArc = std::abs(arc.end - arc.start) > nytl::constants::pi;
	ret.clockwise = arc.end - arc.start > 0.f;
	return ret;
}

} // namespace ktc
