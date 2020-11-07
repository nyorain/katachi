// Copyright (c) 2018-2020 Jan Kelling
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#include <katachi/fwd.hpp>

#include <nytl/vec.hpp>
#include <nytl/stringParam.hpp>
#include <nytl/span.hpp>

#include <variant>
#include <vector>
#include <cstddef>

namespace ktc {

/// Command parameters
struct LineParams {};
struct SQBezierParams {};

struct QBezierParams { Vec2f control; };
struct SCBezierParams { Vec2f control2; };

struct CBezierParams {
	Vec2f control1;
	Vec2f control2;
};

struct ArcParams {
	Vec2f radius;
	bool largeArc {};
    bool clockwise {};
};

/// Represents one subpath segment.
/// Can either be a line, a (optionally smooth) cubic/quadratic bezier or
/// an arc, depending which one is active in the params variant.
struct Command {
	Vec2f to;
	std::variant<
		LineParams,
		QBezierParams,
		SQBezierParams,
		CBezierParams,
		SCBezierParams,
		ArcParams> params;
};

/// A continous path consisting of curve commands.
class Subpath {
public:
	Vec2f start {}; /// Starting point
	bool closed {}; /// Whether the subpath is closed
	std::vector<Command> commands {}; /// The commands defining the subpath

public:
	Command& line(Vec2f to);
	Command& arc(Vec2f to, const ArcParams&);
	Command& qBezier(Vec2f to, const QBezierParams&);
	Command& sqBezier(Vec2f to);
	Command& cBezier(Vec2f to, const CBezierParams&);
	Command& scBezier(Vec2f to, const SCBezierParams&);
};

/// Collection of continous subpaths forming a Path that may contains jumps.
class Path {
public:
	std::vector<Subpath> subpaths;
	Subpath& move(Vec2f to);
};

/// Transform ArcParams into a CenterArc description (curves.hpp).
/// The CenterArc description can be used to flatten the arc into points.
CenterArc parseArc(Vec2f from, ArcParams&, Vec2f to);

/// Defines various aspects (mainly precision) of the path flattening
/// process.
struct FlattenSettings {
	/// steps for baking an arc segment:
	/// clamp(arcLengthFac * radius * dangle, minSteps, maxSteps)
	float arcLengthFac = 0.2f; // roughly 1 segment per 5 pixels
	unsigned minArcSteps = 4u;
	unsigned maxArcSteps = 256u;

	/// The maxLevel params passed to the flatten functions for bezier curves
	unsigned maxQBezLevel = 8u;
	float minQBezDist = 0.001f;

	unsigned maxCBezLevel = 10u;
	float minCBezDist = 0.001f;
};

/// Flattens the given subpath into a point array.
/// Note that if the subpath is closed, will append its first point
/// as additional end point.
std::vector<Vec2f> flatten(const Subpath&, const FlattenSettings& = {});

} // namespace vgv
