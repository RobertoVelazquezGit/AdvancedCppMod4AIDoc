# MultiModalDoc

Educational C++ example of documentation generation and prompt management using
in-memory mocks. No AI services are contacted and no source or template files
are read.

## Main classes

- MultiModalDocumentation::DocumentationOrchestrator registers mock providers,
  generates placeholder documentation, appends audience labels, and applies
  quality thresholds.
- MultiModalDocumentation::IntelligentPromptManager creates fixed templates,
  builds contextual prompts, appends feedback, and stores template scores.

The helper classes are implemented in `MultiModalDoc.cpp`. Public interfaces,
request structures, and result structures are declared in `MultiModalDoc.h`.
`main.cpp` demonstrates both main classes with one Markdown request and a prompt
for a simple addition function.

## Mock behavior and limitations

- Generation runs immediately and returns a ready future. Calling `get()`
  retrieves the result or rethrows a captured generation error.
- Batch generation is sequential and stops at the first failure.
- Every output format is a labeled plain-text placeholder, including PDF and HTML.
- Documentation quality and audience scores are fixed at 80 out of 100.
- Failed quality thresholds produce warnings. Blocking clears generated content
  when enabled through `enableQualityGating`.
- Provider configuration must be nonempty but is not parsed or stored.
- Loading a nonempty template path creates six fixed templates in memory and
  resets their scores and feedback; it never opens the path.
- Prompt optimization only appends context and feedback. Its quality score is
  zero for empty output and 80 otherwise; confidence is zero or 0.8.
- Custom generation parameters, template substitutions, and contextual hints
  are reserved and unused by the mock. Instances are not thread-safe.

## Build and run

The project uses Visual Studio and the MSVC v143 toolset. The example has been
compiled and run with MSVC in C++17 mode. Build the `MultiModalDoc` project and
run it to print mock documentation, its quality score, and a generated prompt.

## Generate documentation

Requirements: Doxygen and Graphviz (`dot`) available on `PATH`.
Run from the `MultiModalDoc` directory:

```powershell
doxygen Doxyfile
```

Open `docs/html/index.html`. Diagnostics are saved to
`docs/doxygen-warnings.log`.

The configuration follows the solution's `GuideDoxy.md`: recursive C++ and
Markdown scanning, this README as the main page, private-member documentation,
source browsing, HTML output, and Graphviz class, collaboration, include,
call, and caller graphs wherever Doxygen resolves relationships. Generated
documentation and build directories are excluded from input. LaTeX output is
disabled.

Modern Doxygen versions use `CLASS_GRAPH` with `HAVE_DOT` in place of the guide's
obsolete `CLASS_DIAGRAMS` option.
