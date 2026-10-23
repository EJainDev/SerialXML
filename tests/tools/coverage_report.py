#!/usr/bin/env python3
"""Merge gcov's line counters for the module and its test-instantiated templates.

Requires a matching gcov executable and a completed SERIAL_XML_TEST_COVERAGE
build/test run. Reports measured runtime coverage, not compile-time reflection
coverage or exhaustive input coverage. Branch totals include compiler-generated
and exception edges, and count each function instantiation separately.
"""
import argparse
import gzip
import json
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", required=True, type=Path)
    parser.add_argument("--gcov", default="gcov")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    build = args.build_dir.resolve()
    notes = [build / "CMakeFiles/serial_xml.dir/include/serial_xml.cxx.gcno",
             build / "tests/CMakeFiles/serial_xml_tests.dir/test_serial_xml.cpp.gcno"]
    counters = {}
    branches = []
    with tempfile.TemporaryDirectory(prefix="serial-xml-gcov-") as temporary:
        for note in notes:
            if not note.is_file() or not note.with_suffix(".gcda").is_file():
                parser.error(f"Missing coverage notes/data for {note}; build and run the tests first")
            subprocess.run([args.gcov, "--json-format", "--branch-probabilities", "--branch-counts",
                            str(note)], cwd=temporary, check=True, stdout=subprocess.DEVNULL)
            for path in Path(temporary).glob("*.gcov.json.gz"):
                data = json.loads(gzip.decompress(path.read_bytes()))
                for entry in data["files"]:
                    if Path(entry["file"]).resolve() != root / "include/serial_xml.cxx":
                        continue
                    for line in entry["lines"]:
                        number = line["line_number"]
                        counters[number] = counters.get(number, 0) + line["count"]
                        branches.extend(line.get("branches", []))
                path.unlink()
    if not counters:
        parser.error("gcov did not report the library source")
    negative = sorted(line for line, count in counters.items() if count < 0)
    if negative:
        parser.error(f"gcov inferred negative counters at {negative}; rebuild with atomic profile updates and remove old .gcda files")
    hit = sum(value > 0 for value in counters.values())
    report = {"source": str(root / "include/serial_xml.cxx"),
              "measured_lines": len(counters), "executed_lines": hit,
              "line_percent": round(100 * hit / len(counters), 2),
              "uncovered_lines": sorted(line for line, count in counters.items() if count == 0),
              "raw_instantiated_branches": len(branches),
              "executed_raw_instantiated_branches": sum(edge["count"] > 0 for edge in branches),
              "limitations": "Runtime lines only; template instances merged by line. Raw branches include compiler-generated and exception edges. Not exhaustive input or compile-time coverage."}
    output = build / "coverage-summary.json"
    output.write_text(json.dumps(report, indent=2) + "\n")
    print(f"Runtime source lines: {hit}/{len(counters)} ({report['line_percent']}%)")
    print(f"Uncovered lines: {report['uncovered_lines']}")
    print(f"Report: {output}")


if __name__ == "__main__":
    main()
