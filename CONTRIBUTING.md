# Contributing

Thank you for contributing. Follow these guidelines to ensure high code quality and a smooth review process.

Code style

- Use the provided clang-format configuration. Keep changes focused and well-scoped.
- Follow the existing pattern for module layout, thread handling and error reporting.

Testing

- Add unit tests for new logic and run the test harness: make test
- Verify the project builds cleanly with the compiler flags in the Makefile.

Pull requests

- Open a topic branch, keep commits atomic and write clear commit messages.
- Include tests and documentation for new features.
- PRs will be reviewed for correctness, performance and security implications.

Security issues

- For vulnerability reports, use the repository's security/contact channel. Do not disclose details publicly until a fix is available.

Release process

- Maintain a changelog entry for user-visible changes.
- Tag releases following semantic versioning.
