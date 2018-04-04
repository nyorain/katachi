# Katachi

Lightweight library that flattens vector graphics like paths into point arrays.
Also offers various utility for processing curves and shapes with additional
data and functionality (like colored points or antialiasing) for accelerated
rendering.

## Example

This example shows some of the simpler, basic functionality katachi
offers.

```cpp
ktc::Path path;
auto& sub1 = path.move({100.f, 100.f});
sub1.qBezier({100.f, 200.f}, {200.f, 200.f});
sub1.line({200.f, 200.f});
sub1.arc({300.f, 300.f}, {150.f, 150.f, false, false});

auto& sub2 = path.move({500.f, 500.f});
sub2.line({600.f, 500.f});

// Alternatively we can specify paths directly by using the svg
// path syntax
auto subpath = ktc::parseSvgSubpath("M 100 100 Q 200 200, 100 200 L 200 200");

// Flattens a subpath, returns vector<Vec2f>
// The polygon could then e.g. be used for hardware accelerated rendering
auto polygon = flatten(subpath);

// We could now also bake the points needed for stroking the outline
// of a flattened path
auto settings = ktc::StrokeSettings {10.f}; // stroke width
auto stroke1 = ktc::bakeStroke(polygon, settings);
```

Overall, this library cleanly implements the curve/points part of the
svg specification.

## Roadmap

- [ ] handle arc axis rotation
- [ ] unit testing
- [ ] stroke.hpp: doc, api cleanup, additions (StrokeSettings)
	- [x] support for antialiasing data (stroke + fill)
	- [x] more advanced color/whatever data support
	- [ ] lineCap, lineJoin (also with anti aliasing)
	- [ ] better loop handling (loop flag in StrokeSettings?)
