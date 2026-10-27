#!/usr/bin/env python3
"""Build isolated source mutants and require the actual AnnoTest suite to reject them.

Run after configuring/building SerialXML with BUILD_TESTS=ON. The baseline build
supplies its compiler and fetched dependency sources; working files are never
mutated. Logs and the result manifest live under build/mutation-checks by default.
This is a small, explicit mutation sample, not a general mutation-score tool.
"""

import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--jobs", type=int, default=2)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    baseline = args.build_dir.resolve()
    output = (args.output_dir or root / "build/mutation-checks").resolve()
    if root not in output.parents or "build" not in output.relative_to(root).parts:
        parser.error("--output-dir must be inside the repository's build directory")
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    output.mkdir(parents=True, exist_ok=True)
    (output / "results.json").unlink(missing_ok=True)
    cache = (baseline / "CMakeCache.txt").read_text()

    def cached(name):
        match = re.search(r"^" + re.escape(name) + r":[^=]*=(.*)$", cache, re.M)
        return match[1] if match else None

    compiler = cached("CMAKE_CXX_COMPILER")
    if not compiler:
        parser.error("baseline build has no CMAKE_CXX_COMPILER")
    source = output / "source"
    build = output / "build"
    source.mkdir(exist_ok=True)
    tracked = subprocess.check_output(["git", "ls-files", "-z"], cwd=root).split(b"\0")
    # Include newly authored source tests too, without copying unrelated
    # untracked build trees from the checkout.
    tracked += subprocess.check_output(
        ["git", "ls-files", "--others", "--exclude-standard", "-z", "--",
         "tests", "include", "cmake", "examples"], cwd=root).split(b"\0")
    for entry in tracked:
        if not entry:
            continue
        relative = Path(entry.decode())
        original = root / relative
        if original.is_file():
            destination = source / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(original, destination)
    module = source / "include/serial_xml.cxx"
    pristine = module.read_text()
    mutations = [
        ("numeric_trailing_text", "FromStringTests.*",
         "error != std::errc{} || end != text.data() + text.size() || text.empty()",
         "error != std::errc{} || text.empty()"),
        ("depth_off_by_one", "FromXmlTests.accepts_exact_depth_limit_and_rejects_the_next_level",
         "if (depth >= 256)", "if (depth > 256)"),
        ("capacity_off_by_one", "FromXmlTests.fixed_capacity_and_array_boundaries",
         "if (count > T::capacity())", "if (count >= T::capacity())"),
        ("missing_tail_quote_escape", "EscapingTests.escapes_adjacent_quotes_in_report_properties",
         "if (c == '<' || c == '>' || c == '&' || c == '\"' || c == '\\'')",
         "if (c == '<' || c == '>' || c == '&' || c == '\\'')"),
    ]
    for name, _, before, _ in mutations:
        if pristine.count(before) != 1:
            parser.error(f"{name}: mutation anchor must match exactly once")

    def run(command, log):
        with (output / log).open("w") as stream:
            return subprocess.run(command, cwd=root, stdout=stream, stderr=subprocess.STDOUT,
                                  timeout=1800).returncode

    configure = ["cmake", "-S", str(source), "-B", str(build), "-G", "Ninja",
                 f"-DCMAKE_CXX_COMPILER={compiler}", "-DCMAKE_BUILD_TYPE=Debug",
                 "-DBUILD_TESTS=ON", "-DBUILD_EXAMPLES=OFF", "-DBUILD_BENCHMARKS=OFF"]
    for dependency in ("annotest", "structural_tuple"):
        override = cached(f"FETCHCONTENT_SOURCE_DIR_{dependency.upper()}")
        dependency_source = Path(override) if override else baseline / f"_deps/{dependency}-src"
        if dependency_source.is_dir():
            configure.append(f"-DFETCHCONTENT_SOURCE_DIR_{dependency.upper()}={dependency_source}")
    if run(configure, "configure.log"):
        raise RuntimeError("mutation configure failed; see configure.log")
    compile_command = ["cmake", "--build", str(build), "--target", "serial_xml_tests",
                       "--parallel", str(args.jobs)]
    executable = build / "tests/serial_xml_tests"
    if run(compile_command, "baseline-build.log"):
        raise RuntimeError("baseline build failed; see baseline-build.log")
    if run([str(executable), "--brief"], "baseline-tests.log"):
        raise RuntimeError("baseline tests failed; mutations require a passing baseline")
    results = []
    try:
        for name, selector, before, after in mutations:
            print(f"Checking {name}...", flush=True)
            module.write_text(pristine.replace(before, after))
            if run(compile_command, f"{name}-build.log"):
                raise RuntimeError(f"{name}: compile failure is not a killed mutant")
            report = output / f"{name}.json"
            report.unlink(missing_ok=True)
            status = run([str(executable), "--filter", selector, "--brief",
                          "--json-output", str(report)], f"{name}-tests.log")
            data = json.loads(report.read_text())
            # Require a recorded assertion/expectation failure, not just a crash,
            # runner error, skipped selection, or an unrelated uncaught exception.
            failures = [failure for suite in data["suites"] for case in suite["tests"]
                        for failure in case["failures"]]
            killed = status == 1 and data["tests"] > 0 and any(
                failure["type"] in ("assertion", "expectation") for failure in failures)
            results.append({"mutation": name, "killed": killed, "tests": data["tests"],
                            "failed": data["failed"]})
            if not killed:
                raise RuntimeError(f"{name}: survived or failed without an assertion")
            print(f"  killed ({data['failed']} failed cases)", flush=True)
    finally:
        module.write_text(pristine)
        (output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    print(f"Killed {len(results)}/{len(mutations)} explicit mutants; logs: {output}")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.TimeoutExpired, ValueError, KeyError) as error:
        print(error, file=sys.stderr)
        sys.exit(1)
