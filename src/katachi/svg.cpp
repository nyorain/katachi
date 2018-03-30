// Copyright (c) 2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#include <katachi/svg.hpp>
#include <katachi/path.hpp>
#include <dlg/dlg.hpp>

namespace ktc {
namespace {

Path parseSvgPath(StringParam svgSubpath,
		std::optional<SvgError>& error, Vec2f start, Subpath* sub) {

	// TODO: error checks etc
	//  - check for nonnegative numbers (e.g. radius), flags
	//  - handle arc axis rotation

	//  NOTE: probably better as clean recursive descent parser

	error.reset();

	Path ret;
	Subpath* current = sub;
	if(!current) {
		ret.subpaths.push_back({start});
		current = &ret.subpaths.back();
	} else {
		sub->start = start;
	}

	auto begin = svgSubpath.c_str();
	auto it = begin;
	dlg_assert(it);

	auto skipSpace = [&] {
		while(isspace(*it)) {
			++it;
		}
	};

	auto skipComma = [&] {
		if(*it == ',') {
			++it;
		}
	};

	auto readFloat = [&]{
		if(!(*it)) {
			error = {SvgErrorType::incomplete, unsigned(it - begin)};
			return 0.f;
		}

		if(error) {
			return 0.f;
		}

		char* e;
		skipSpace();
		auto r = strtof(it, &e);
		if(it == e) {
			error = {SvgErrorType::invalidNumber, unsigned(it - begin)};
			return 0.f;
		}

		it = e;
		return r;
	};

	auto readCoords = [&]{
		Vec2f ret;
		if(error) {
			return ret;
		}

		ret.x = readFloat();
		skipSpace();
		skipComma();
		skipSpace();
		ret.y = readFloat();
		return ret;
	};

	auto repeat = [&](const auto& parse) {
		auto cmd = parse();
		if(error) {
			return false;
		}

		auto itb = it;
		while(!error) {
			current->commands.push_back(cmd);
			itb = it;
			skipSpace();
			skipComma();
			skipSpace();
			cmd = parse();
		}

		error = {};
		it = itb;
		return true;
	};

	bool first = true;
	while(*it) {
		auto last = current->start;
		if(!current->commands.empty()) {
			last = current->commands.back().to;
		}

		skipSpace();
		auto c = *it;
		if(current->closed) {
			if(sub) {
				error = {SvgErrorType::subpathMove, unsigned(it - begin)};
				return {};
			} else if(c != 'M' && c != 'm') {
				ret.subpaths.emplace_back();
				ret.subpaths.back().start = last;
				current = &ret.subpaths.back();
			}
		}

		++it;
		switch(c) {
			case 'M': case 'm': {
				if(first) {
					current->start = readCoords();
				} else {
					if(sub) {
						auto id = unsigned(it - begin);
						error = {SvgErrorType::subpathMove, id};
					} else {
						ret.subpaths.push_back({readCoords()});
						current = &ret.subpaths.back();
					}
				}
				break;
			} case 'L': case 'l': {
				repeat([&] {
					auto cmd = Command{readCoords(), LineParams {}};
					if(c == 'l') {
						cmd.to += last;
					}
					return cmd;
				});
				break;
			} case 'H': case 'h': {
				repeat([&] {
					auto cmd = Command{{}, LineParams {}};
					cmd.to.x = readFloat();
					cmd.to.y = last.y;
					if(c == 'h') {
						cmd.to.x += last.x;
					}
					return cmd;
				});
				break;
			} case 'V': case 'v': {
				repeat([&] {
					auto cmd = Command{{}, LineParams {}};
					cmd.to.x = last.x;
					cmd.to.y = readFloat();
					if(c == 'v') {
						cmd.to.y += last.y;
					}
					return cmd;
				});
				break;
			} case 'C': case 'c': {
				repeat([&] {
					auto cmd = Command{{}, CBezierParams{}};
					auto& bezier = std::get<CBezierParams>(cmd.params);
					bezier.control1 = readCoords();
					skipSpace(); skipComma();
					bezier.control2 = readCoords();
					skipSpace(); skipComma();
					cmd.to = readCoords();
					if(c == 'c') {
						bezier.control1 += last;
						bezier.control2 += last;
						cmd.to += last;
					}
					return cmd;
				});
				break;
			} case 'S': case 's': {
				repeat([&] {
					auto cmd = Command{{}, SCBezierParams{}};
					auto& bezier = std::get<SCBezierParams>(cmd.params);
					bezier.control2 = readCoords();
					skipSpace(); skipComma();
					cmd.to = readCoords();
					if(c == 's') {
						bezier.control2 += last;
						cmd.to += last;
					}
					return cmd;
				});
				break;
			} case 'Q': case 'q': {
				repeat([&] {
					auto cmd = Command{{}, QBezierParams{}};
					auto& bezier = std::get<QBezierParams>(cmd.params);
					bezier.control = readCoords();
					skipSpace(); skipComma();
					cmd.to = readCoords();
					if(c == 'q') {
						bezier.control += last;
						cmd.to += last;
					}
					return cmd;
				});
				break;
			} case 'T': case 't': {
				repeat([&] {
					auto cmd = Command{{}, SQBezierParams{}};
					cmd.to = readCoords();
					if(c == 't') {
						cmd.to += last;
					}
					return cmd;
				});
				break;
			} case 'A': case 'a': {
				repeat([&] {
					auto cmd = Command{{}, ArcParams {}};
					auto& arc = std::get<ArcParams>(cmd.params);
					arc.radius = readCoords();

					skipSpace(); skipComma();
					readFloat(); // TODO: axis rotation

					skipSpace(); skipComma();
					arc.largeArc = readFloat();

					skipSpace(); skipComma();
					arc.clockwise = readFloat();

					skipSpace(); skipComma();
					cmd.to = readCoords();
					if(c == 'a') {
						cmd.to += last;
					}
					return cmd;
				});
				break;
			} case 'Z': case 'z': {
				current->closed = true;
				break;
			} default: {
				auto pos = unsigned(it - begin) - 1;
				error = {SvgErrorType::invalidCommand, pos};
				return {};
			}
		}

		first = false;
		if(error) {
			return {};
		}
	}

	return ret;
}

} // anon namespace

SvgException::SvgException(const SvgError& err) :
	std::invalid_argument(description(err)) {
}

std::string description(const SvgError& err) {
	auto typestr = [](auto type) {
		switch(type) {
			case SvgErrorType::subpathMove:
				return "Move command not allowed for subpath";
			case SvgErrorType::invalidCommand:
				return "Invalid svg path command";
			case SvgErrorType::invalidNumber:
				return "Invalid number parameter";
			default:
				return "<Invalid error type>";
		}
	};

	std::string str = "svg path error at char ";
	str += std::to_string(err.pos);
	str += ": ";
	str += typestr(err.type);
	return str;
}

Subpath parseSvgSubpath(StringParam svgSubpath, Vec2f start) {
	std::optional<SvgError> error;
	auto ret = parseSvgSubpath(svgSubpath, error, start);
	if(error) {
		throw SvgException(error.value());
	}
	return ret;
}

Subpath parseSvgSubpath(StringParam svgSubpath, std::optional<SvgError>& error,
		nytl::Vec2f start) {
	Subpath subpath;
	parseSvgPath(svgSubpath, error, start, &subpath);
	return subpath;
}

Path parseSvgPath(StringParam svgPath, Vec2f start) {
	std::optional<SvgError> error;
	auto ret = parseSvgPath(svgPath, error, start);
	if(error) {
		throw SvgException(error.value());
	}
	return ret;
}

Path parseSvgPath(StringParam svgPath, std::optional<SvgError>& error,
		Vec2f start) {
	return parseSvgPath(svgPath, error, start, nullptr);
}

} // namespace ktc
