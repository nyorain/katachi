#include <bugged.hpp>
#include <katachi/svg.hpp>
#include <nytl/approxVec.hpp>
#include <dlg/dlg.hpp>

using namespace nytl;

// Forwarding utility to escape ',' in macros
template<typename T>
decltype(auto) id(T&& val) {
	return std::forward<T>(val);
}

TEST(subpath) {
	auto subpath = ktc::parseSvgSubpath("M 100.0 100 L 200 200, 300 10");

	EXPECT(subpath.start, id(Vec {100.f, 100.f}));
	EXPECT(subpath.closed, false);
	EXPECT(subpath.commands.size(), 2u);
	EXPECT(subpath.commands[0].to, id(Vec {200.f, 200.f}));
	EXPECT(subpath.commands[0].params.index(), 0u);
	EXPECT(subpath.commands[1].to, id(Vec {300.f, 10.f}));
	EXPECT(subpath.commands[1].params.index(), 0u);
}

TEST(qbezier) {
	auto subpath = ktc::parseSvgSubpath("Q 1e2 0 200 200 t 100,100", {10, 10});

	auto& cmds = subpath.commands;
	EXPECT(subpath.start, id(Vec {10.f, 10.f}));
	EXPECT(subpath.closed, false);
	EXPECT(cmds.size(), 2u);
	EXPECT(cmds[0].to, id(Vec {200.f, 200.f}));
	EXPECT(cmds[1].to, id(Vec {300.f, 300.f}));
	EXPECT(cmds[1].params.index(), 2u);

	auto& b1 = std::get<ktc::QBezierParams>(cmds[0].params);
	EXPECT(b1.control, id(Vec {100.f, 0.f}));
}

TEST(arc) {
	// example exactly from the svg spec page
	auto arcpath = "M300,200 h-150 a150,150 0 1,0 150,-150 z";
	auto subpath = ktc::parseSvgSubpath(arcpath);

	EXPECT(subpath.start, id(Vec {300.f, 200.f}));
	EXPECT(subpath.closed, true);
	EXPECT(subpath.commands.size(), 2u);
	EXPECT(subpath.commands[0].to, id(Vec{150.f, 200.f}));
	EXPECT(subpath.commands[0].params.index(), 0u);
	EXPECT(subpath.commands[1].to, id(Vec {300.f, 50.f}));

	auto& arc = std::get<ktc::ArcParams>(subpath.commands[1].params);
	EXPECT(arc.radius, id(Vec {150.f, 150.f}));
	EXPECT(arc.largeArc, true);
	EXPECT(arc.clockwise, false);
}

TEST(paths) {
	auto pathstring = "M 10,10 L 20,20 M 30,3e1 h 10 z l10 10";
	auto paths = ktc::parseSvgPath(pathstring);

	EXPECT(paths.subpaths.size(), 3u);

	auto& s1 = paths.subpaths[0];
	EXPECT(s1.start, id(Vec {10.f, 10.f}));
	EXPECT(s1.closed, false);
	EXPECT(s1.commands.size(), 1u);
	EXPECT(s1.commands[0].to, id(Vec {20.f, 20.f}));
	EXPECT(s1.commands[0].params.index(), 0u);

	auto& s2 = paths.subpaths[1];
	EXPECT(s2.start, id(Vec {30.f, 30.f}));
	EXPECT(s2.closed, true);
	EXPECT(s2.commands.size(), 1u);
	EXPECT(s2.commands[0].to, id(Vec {40.f, 30.f}));
	EXPECT(s2.commands[0].params.index(), 0u);

	auto& s3 = paths.subpaths[2];
	EXPECT(s3.start, id(Vec {40.f, 30.f}));
	EXPECT(s3.closed, false);
	EXPECT(s3.commands.size(), 1u);
	EXPECT(s3.commands[0].to, id(Vec {50.f, 40.f}));
	EXPECT(s3.commands[0].params.index(), 0u);
}

TEST(errors) {
	auto str1 = "M10,10Zh10";
	ERROR(ktc::parseSvgSubpath(str1), ktc::SvgException);

	std::optional<ktc::SvgError> error;
	EXPECT(ktc::parseSvgSubpath(str1, error).commands.empty(), true);
	EXPECT(error.has_value(), true);
	EXPECT(error.value().pos, 7u);
	EXPECT(error.value().type, ktc::SvgErrorType::subpathMove);

	auto str2 = "R";
	EXPECT(ktc::parseSvgSubpath(str2, error).commands.empty(), true);
	EXPECT(error.has_value(), true);
	EXPECT(error.value().pos, 0u);
	EXPECT(error.value().type, ktc::SvgErrorType::invalidCommand);

	auto str3 = "L10Z";
	EXPECT(ktc::parseSvgSubpath(str3, error).commands.empty(), true);
	EXPECT(error.has_value(), true);
	EXPECT(error.value().pos, 3u);
	EXPECT(error.value().type, ktc::SvgErrorType::invalidNumber);

	auto str4 = "L 10";
	EXPECT(ktc::parseSvgSubpath(str4, error).commands.empty(), true);
	EXPECT(error.has_value(), true);
	EXPECT(error.value().pos, 4u);
	EXPECT(error.value().type, ktc::SvgErrorType::incomplete);

	auto str5 = "";
	EXPECT(ktc::parseSvgSubpath(str5, error).commands.empty(), true);
	EXPECT(error.has_value(), false);
}
