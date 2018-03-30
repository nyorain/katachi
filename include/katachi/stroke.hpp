// Copyright (c) 2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#include <katachi/fwd.hpp>
#include <nytl/vec.hpp>
#include <vector>

// TODO: this needs some love

namespace ktc {

struct StrokeSettings {
	float width;
};

std::vector<Vec2f> bakeStroke(const Subpath& sub, const StrokeSettings&);
std::vector<Vec2f> bakeStroke(Span<const Vec2f> points, const StrokeSettings&);

void bakeStroke(Span<const Vec2f> points, const StrokeSettings&,
	std::vector<Vec2f>& baked);

void bakeColoredStroke(Span<const Vec2f> points, Span<const Vec4u8> color,
	const StrokeSettings&, std::vector<Vec2f>& outPoints,
	std::vector<Vec4u8>& outColor);

} // namespace ktc
