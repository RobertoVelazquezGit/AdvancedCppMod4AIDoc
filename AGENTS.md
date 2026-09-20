\# AGENTS.md



\## Repository Overview



This repository contains software projects primarily developed in C++ using Visual Studio Community and the MSVC compiler.



The projects may cover different technical topics. Do not assume a specific domain or architecture unless it is explicitly stated by the user or clearly established by the existing code.



\---



\## General Rules



\### 1. Do Not Modify Without Explicit Approval



Do NOT modify, create, delete, move, or rename any file without explicit approval from the user.



Before making any change:



1\. Analyze the request or problem.

2\. Explain the situation clearly.

3\. Propose the intended change.

4\. Wait for explicit user approval.

5\. Only then apply the change.



By default, operate in analysis and consultation mode.



\---



\### 2. Analyze Before Changing



When asked for help:



\- Analyze the existing code first.

\- Explain what you found.

\- Identify relevant files and code locations.

\- Explain possible issues or improvements.

\- Propose changes before applying them.



Do not automatically modify code just because a problem or improvement has been identified.



\---



\### 3. Respect the Existing Architecture



Do not assume that the existing architecture is wrong.



Before proposing significant changes:



\- Understand the current design.

\- Identify its purpose.

\- Explain the advantages and disadvantages of the current approach.

\- Explain the impact of any proposed change.



Do not perform architectural changes unless explicitly requested.



\---



\### 4. Preserve the Visual Studio Environment



This repository uses:



\- Visual Studio Community

\- MSVC

\- Visual Studio solutions (`.sln`)

\- Visual Studio project files (`.vcxproj`)



Unless explicitly requested by the user:



\- Do not migrate the project to CMake.

\- Do not introduce another build system.

\- Do not migrate the project to another IDE.

\- Do not change the existing compiler/toolchain.

\- Do not modify Visual Studio solution or project configuration files.



\---



\### 5. Avoid Unnecessary Changes



Do not perform unrelated changes such as:



\- Large refactorings.

\- Repository-wide formatting changes.

\- Renaming symbols for stylistic reasons.

\- Reorganizing directories without a clear need.

\- Changing coding style without being asked.

\- Modifying unrelated files.



Only modify what is directly related to the user's request.



\---



\### 6. Compilation and Diagnostics



When analyzing build problems:



\- Identify the relevant file.

\- Identify the relevant code location or line when possible.

\- Explain the cause of the problem.

\- Explain the consequences.

\- Propose a solution before applying it.



Do not hide, suppress, or disable warnings or compiler diagnostics unless explicitly requested.



\---



\### 7. Git



Respect the existing Git repository and workflow.



Before making changes:



\- Be aware of the current Git status.

\- Clearly identify the files that would be modified.

\- Explain why each modification is necessary.



Do not discard, overwrite, reset, or revert user changes without explicit approval.



Never perform destructive Git operations without explicit user approval.



\---



\### 8. Communication



The user may communicate with the agent in Spanish.



Respond in the language used by the user unless the user explicitly requests another language.



Technical names, API names, compiler messages, and code identifiers should remain unchanged.

Write code comments and Doxygen documentation in English, even when the conversation is in Spanish, unless the user explicitly requests otherwise.



When there is uncertainty about the user's intention, ask before making changes.



\---



\## Preferred Workflow



Unless the user explicitly requests otherwise, follow this workflow:



1\. Analyze.

2\. Explain.

3\. Propose.

4\. Wait for explicit approval.

5\. Modify.

6\. Compile or verify.

7\. Report the changes and results.



Do not skip the approval step without explicit authorization from the user.



\---



\## Final Rule



The user's explicit instructions always take priority.



When in doubt about whether a file should be modified, do not modify it. Ask the user first.

