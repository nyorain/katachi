// Copyright (c) 2018-2020 Jan Kelling
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#pragma once

#include <katachi/fwd.hpp>
#include <katachi/path.hpp>
#include <nytl/stringParam.hpp>
#include <nytl/vec.hpp>
#include <optional>
#include <stdexcept>

namespace ktc {

enum class SvgErrorType {
	subpathMove, // subpath contains a move
	invalidCommand, // unknown command char
	invalidNumber, // failed to read a number
	incomplete, // string too short, incomplete command
};

struct SvgError {
	SvgErrorType type;
	unsigned pos;
};

class SvgException : public std::invalid_argument {
public:
	SvgException(const SvgError&);
	SvgError error;
};

/// Returns a textual description of the given svg path parsing error.
std::string description(const SvgError&);

/// Parses the given svg subpath string. The overload with optional error
/// reference simply returns the error and an empty subpath on error,
/// the other overload throws.
/// Only the first command is allowed to be a move command and will override
/// the passed start parameter.
Subpath parseSvgSubpath(StringParam svgSubpath, Vec2f start = {});
Subpath parseSvgSubpath(StringParam svgSubpath, std::optional<SvgError>&,
	nytl::Vec2f start = {});

/// Parses the given svg subpath string. The overload with optional error
/// reference simply returns the error and an empty subpath on error,
/// the other overload throws.
Path parseSvgPath(StringParam svgPath, Vec2f start = {});
Path parseSvgPath(StringParam svgPath, std::optional<SvgError>&,
	Vec2f start = {});

} // namespace ktc
