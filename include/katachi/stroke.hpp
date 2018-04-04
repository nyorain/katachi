// Copyright (c) 2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#include <katachi/fwd.hpp>
#include <nytl/vec.hpp>
#include <vector>
#include <functional>

namespace ktc {

/// Defines how the given outline points are transform to stroke
/// points.
struct StrokeSettings {
	float width; /// Width of the stroke (point normal length).
};

/// Vertex of a stroke operation.
/// The aa value can be used for antialiased strokes.
/// Its y value if 1.f for vertices on the left and -1.f for vertices
/// on the right. Knowing the stroke width one can easily compute
/// a stroke mask (e.g. in a fragment shader).
struct Vertex {
	Vec2f position;
	Vec2f aa;
	Vec4u8 color;
};

/// Handles (e.g. pushes into buffer) a generated stroke vertex.
/// Can discard any information it does not need.
using VertexHandlerFn = std::function<void(const Vertex&)>;

/// Generates the vertices to stroke the given points.
/// The vertices will be ordered triangle-strip like.
/// The color member of all generated vertices will be {0, 0, 0, 255}
void bakeStroke(Span<const Vec2f> points, const StrokeSettings&,
	const VertexHandlerFn&);

/// Like bakeStroke but applies the given colors in order to the vertices.
/// If the color span is not as long as the points span, extra color
/// values will be discarded or the default color value {0, 0, 0, 255}
/// used for the remaining points.
void bakeColoredStroke(Span<const Vec2f> points, Span<const Vec4u8> color,
	const StrokeSettings&, const VertexHandlerFn&);

} // namespace ktc
