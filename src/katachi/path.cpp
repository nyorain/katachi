// Copyright (c) 2018-2020 Jan Kelling
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#include <katachi/path.hpp>
#include <katachi/curves.hpp>
#include <nytl/math.hpp>
#include <nytl/vecOps.hpp>
#include <dlg/dlg.hpp>
#include <cmath>

// https://www.w3.org/TR/SVG11/paths.html#PathElement

namespace ktc {

// Subpath
Command& Subpath::line(Vec2f to) {
	commands.push_back({to, LineParams{}});
	return commands.back();
}

Command& Subpath::arc(Vec2f to, const ArcParams& arc) {
	commands.push_back({to, arc});
	return commands.back();
}

Command& Subpath::qBezier(Vec2f to, const QBezierParams& bezier) {
	commands.push_back({to, bezier});
	return commands.back();
}

Command& Subpath::sqBezier(Vec2f to) {
	commands.push_back({to, SQBezierParams {}});
	return commands.back();
}

Command& Subpath::cBezier(Vec2f to, const CBezierParams& bezier) {
	commands.push_back({to, bezier});
	return commands.back();
}

Command& Subpath::scBezier(Vec2f to, const SCBezierParams& bezier) {
	commands.push_back({to, bezier});
	return commands.back();
}

// Path
Subpath& Path::move(Vec2f to) {
	subpaths.push_back({to});
	return subpaths.back();
}

std::vector<Vec2f> flatten(const Subpath& sub, const FlattenSettings& fs) {
	if(sub.commands.empty()) {
		return {};
	}

	std::vector<Vec2f> points = {sub.start};
	points.reserve(sub.commands.size() * 2);

	auto current = sub.start;
	auto lastControlQ = current;
	auto lastControlC = current;
	auto to = current;

	auto commandBaker = [&](auto&& p){
		using T = std::decay_t<decltype(p)>;

		if constexpr(std::is_same_v<T, LineParams>) {
			points.push_back(to);
			lastControlC = lastControlQ = to;
		} else if constexpr(std::is_same_v<T, QBezierParams>) {
			auto b = QuadBezier {current, p.control, to};
			flatten(b, points, fs.maxQBezLevel, fs.minQBezDist);
			lastControlQ = p.control;
			lastControlC = to;
		} else if constexpr(std::is_same_v<T, SQBezierParams>) {
			lastControlQ = mirror(current, lastControlQ);
			auto b = QuadBezier {current, lastControlQ, to};
			flatten(b, points, fs.maxQBezLevel, fs.minQBezDist);
			lastControlC = to;
		} else if constexpr(std::is_same_v<T, CBezierParams>) {
			auto b = CubicBezier {current, p.control1, p.control2, to};
			flatten(b, points, fs.maxCBezLevel, fs.minCBezDist);
			lastControlQ = to;
			lastControlC = p.control2;
		} else if constexpr(std::is_same_v<T, SCBezierParams>) {
			auto control = mirror(current, lastControlC);
			auto b = CubicBezier {current, control, p.control2, to};
			flatten(b, points, fs.maxCBezLevel, fs.minCBezDist);
			lastControlQ = to;
			lastControlC = p.control2;
		} else if constexpr(std::is_same_v<T, ArcParams>) {
			// if radius is {0.f, 0.f} draw a straight line
			if(p.radius == Vec {0.f, 0.f}) {
				points.push_back(to);
			} else {
				auto arc = endToCenter({current, to, p.radius,
					p.largeArc, p.clockwise});
				auto fac = std::abs(arc.end - arc.start) * length(arc.radius);
				auto steps = std::clamp<unsigned>(fs.arcLengthFac * fac,
					fs.minArcSteps, fs.maxArcSteps);
				flatten(arc, points, steps);
			}
			lastControlC = lastControlQ = to;
		} else {
			dlg_error("bake(Subpath): Invalid variant");
			return;
		}
	};

	for(auto& cmd : sub.commands) {
		to = cmd.to;
		visit(commandBaker, cmd.params);
		current = to;
	}

	if(sub.closed) {
		points.push_back(sub.start);
	}

	return points;
}

} // namespace ktc

