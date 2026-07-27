# Contributing to SerialXML

Thank you for your interest in contributing to SerialXML! All contributions are greatly appreciated. This document outlines the guidelines for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Reporting Bugs](#reporting-bugs)
  - [Compile Errors](#compile-errors)
  - [Runtime Bugs](#runtime-bugs)
- [Feature Requests](#feature-requests)
- [Local Setup Instructions](#local-setup-instructions)
- [Testing Guidelines](#testing-guidelines)
- [Style & Commit Rules](#style--commit-rules)
- [Pull Request Guidelines](#pull-request-guidelines)
- [Review Process](#review-process)

## Code of Conduct

See [Code of Conduct](CODE_OF_CONDUCT.md).

## Reporting Bugs

Before submitting a bug report, ensure the issue hasn't been reported already. Provide clear, concise details to help maintainers reproduce and fix the issue.

### Compile Errors

To report compile errors, specify:
1. The exact error message or symptom.
2. The compiler compilation log.
3. A minimal, self-contained reproducible example.

### Runtime Bugs

To report runtime bugs, specify:
1. What happened.
2. What should have happened.
3. A minimal reproducing example.
4. Why it could have happened (optional).

## Feature Requests

Feature requests drive the growth of this library. All requests are welcome, even if you cannot implement them yourself. If you propose a feature, please test the implementation via a PR. Include:
1. Feature description.
2. Intent and use case for the feature.
3. Example usage (optional).

## Local Setup Instructions

1. Clone the repository.
2. Install GCC 16.1 and CMake 4.3 or higher.
3. Configure the project using the `debug-gcc-16` preset: `cmake --preset debug-gcc-16`
4. Build the project using the `build-debug` preset: `cmake --build --preset build-debug`

## Testing Guidelines

- Use Google Test as the testing framework.
- Run the full test suite before submitting changes: `ctest --preset build-debug`
- All tests must pass. Do not break existing functionality.
- Add new tests for new features.
- Add regression tests for bug fixes.
- Verify that changes compile and run correctly across supported platforms.

## Style & Commit Rules

- Format code using `clang-format`. Run `clang-format -i <file>` before committing.
- Follow general guidelines using `clang-tidy`. If you disable a check (globally or locally), explain the rationale in your PR.
- Use conventional commit messages: `<type>(<scope>): <description>`
  - `feat`: New feature
  - `fix`: Bug fix
  - `docs`: Documentation changes
  - `style`: Code style changes (formatting, semicolons, etc.)
  - `refactor`: Code refactoring
  - `test`: Adding or modifying tests
  - `chore`: Maintenance tasks

## Pull Request Guidelines

- Create a separate branch for each feature or fix.
- Keep PRs focused. One PR per feature/fix.
- Update documentation if API or behavior changes.
- Link related issues in the PR description.
- Ensure all tests pass locally before requesting a review.

## Review Process

1. **Local Verification:** Ensure all tests pass and code is formatted correctly.
2. **Maintainer Review:** Maintainers will evaluate functionality, code quality, and alignment with project goals.
3. **Feedback Loop:** Address review comments promptly. Push updates to the same branch.
4. **Merge:** Once approved, a maintainer will squash and merge the PR.
5. **Release:** Changes will be included in the next scheduled release or hotfix.

Thank you for helping improve SerialXML!