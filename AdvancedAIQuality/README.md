# AdvancedAIQuality

Educational C++ example of documentation analysis and generation, implemented
with deterministic local mocks. No AI provider, repository access, file writing,
background worker or real publishing is involved when running the example.

## Main classes

- DocumentationQualityFramework::AIDocumentationAnalyzer builds a report from
  a fixed document containing Overview and Usage. Required labels are searched
  as case-sensitive substrings. Metrics and confidence are illustrative values.
- DocumentationQualityFramework::ContinuousDocumentationPipeline processes
  an in-memory FIFO queue and prints simulated monitoring, generation and
  publishing events. Monitoring performs only one scan per start.

The internal SemanticAnalysisEngine and CodeValidationEngine are small stand-ins
that only check for nonempty text. They do not perform real semantic validation.
The analyzer destructor is defined in the implementation file so these types
are complete when their unique_ptr owners are destroyed.

## Example

main.cpp configures three required sections, analyzes a fixture and reports
two-thirds completeness with a suggestion to add Examples. It then initializes
a pipeline, schedules one task, processes it and simulates publication.
The sample paths are labels and do not need to exist.

## Mock limitations

Analyzer weightings are reserved. Audience alignment selects an illustrative
score; disabled audience analysis is represented by zero. Cross-reference checks
only detect duplicate path labels. Code examples are checked for nonempty text.
The pipeline ignores scheduling time, priority, watched patterns, polling interval,
provider endpoint and generation parameters. Call lifecycle and queue processing
methods from one controlling thread. Explicit task IDs are preserved without
uniqueness validation; automatically generated IDs use a per-instance counter.

## Build and run

Open Mod4AIDoc.sln in Visual Studio, select AdvancedAIQuality as the startup
project, select Debug / x64 and build and run the example with MSVC.

## Generate documentation

Following ../GuideDoxy.md, run from this project directory:

```powershell
doxygen Doxyfile
```

Open docs/html/index.html. Doxygen and Graphviz must be available on PATH.
The configuration enables HTML, Markdown as the main page, private members,
source browsing, class and collaboration diagrams, include graphs and call/caller
graphs. Doxygen warnings are written to docs/doxygen-warnings.log.
The guide's obsolete CLASS_DIAGRAMS option is replaced by CLASS_GRAPH and HAVE_DOT,
as in the EnterpriseAI example. This Doxygen generation writes real HTML files;
it is separate from the application's simulated generation pipeline.
