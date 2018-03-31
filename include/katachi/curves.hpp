// Copyright (c) 2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#include <katachi/fwd.hpp>
#include <nytl/vec.hpp>
#include <vector>

namespace ktc {

/// All information needed to represent a quadratic bezier curve.
struct QuadBezier {
	Vec2f start;
	Vec2f control;
	Vec2f end;
};

/// All information needed to represent a cubic bezier curve.
struct CubicBezier {
	Vec2f start;
	Vec2f control1;
	Vec2f control2;
	Vec2f end;
};

/// All information needed to draw an arc when its center is known.
struct CenterArc {
	Vec2f center;
	Vec2f radius;
	float start;
	float end;
};

/// All information needed to draw an arc when start- and end- points are known.
/// Note that this representation allows invalid arc (e.g. when radius is
/// to small to allow any circle).
struct EndArc {
	Vec2f from;
	Vec2f to;
	Vec2f radius;
	bool largeArc;
	bool clockwise;
};

void flatten(const CubicBezier&, std::vector<Vec2f>&,
	unsigned maxLevel = 8, float minDist = 0.001f);
void flatten(const QuadBezier&, std::vector<Vec2f>&,
	unsigned maxLevel = 10, float minDist = 0.001f);
void flatten(const CenterArc&, std::vector<Vec2f>&, unsigned steps);

CubicBezier quadToCubic(const QuadBezier&);
CenterArc endToCenter(const EndArc&);
EndArc centerToEnd(const CenterArc&);

} // namespace ktc
