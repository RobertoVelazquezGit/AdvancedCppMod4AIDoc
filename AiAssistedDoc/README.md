# AiAssistedDoc

Educational C++ console game demonstrating AI-assisted source documentation,
reviewed against the implementation and published with Doxygen and Graphviz.
All game state is held in memory.

## Main classes

- @ref Player manages a name, positive score increments and level progression.
- @ref SimpleGame registers players and manages number-guessing rounds.

Both classes and the demonstration entry point are in `AiAssistedDoc.cpp`.
`Doxyfile` configures documentation generation; this README is its main page.

## Actual game behavior

Each round generates a target from 1 to 100 inclusive. An exact guess awards
100 points; incorrect guesses only print directional feedback. A correct guess
does not close the round: the caller must invoke `endRound()`. Repeated exact
guesses can score again while the round remains active.

`calculateScore()` exposes proximity tiers of 100, 50, 25, 10 and 5 points for
absolute differences of 0, 1-5, 6-15, 16-30 and over 30 respectively. The game
calls it only for exact matches, so the lower tiers are not awarded by gameplay.

Ending a round prints each player's statistics before checking a 200-point
threshold. Qualifying players advance exactly one level and reset their score
to zero, discarding excess points. There is no separate level-up notification.

Names are case-sensitive; duplicate registration is rejected, but an empty name
is accepted. Guess ranges are not validated. Starting a new round replaces an
active round and retains players and scores. Random targets use `mt19937` seeded
from `random_device`; this is not cryptographic randomness. Integer arithmetic
must stay within the representable range. The classes are not thread-safe.

## Example

@code{.cpp}
Player alice("Alice");
alice.addScore(200);
alice.checkLevelUp(200); // Level 2 and score 0.

SimpleGame game;
game.addPlayer("Alice");
game.startNewRound();
bool correct = game.makeGuess("Alice", 42);
game.endRound(); // Explicitly close the round regardless of the result.
@endcode

## Build and run

Open `../Mod4AIDoc.sln` in Visual Studio with the C++ workload and the configured
MSVC toolset and Windows SDK. Select `AiAssistedDoc` as the startup project,
then build and run it. No external runtime libraries are needed.

The demonstration registers Alice, Bob and Charlie and runs three rounds using
up to six predetermined guesses per round. Targets and results vary. Press Enter
after each round, including the last, to continue to the final standings.

## Generate documentation

With Doxygen and Graphviz (`dot`) available on PATH, run in `AiAssistedDoc`:

```powershell
doxygen Doxyfile
```

Open `docs/html/index.html`. Generation diagnostics are written to
`docs/doxygen-warnings.log`.

The configuration follows `../GuideDoxy.md`: English triple-slash source
comments, Markdown main page, recursive scanning, documented private members,
source browsing, HTML output and Graphviz class, collaboration, include, call
and caller graphs where relationships can be resolved. LaTeX output is disabled.
`CLASS_GRAPH` and `HAVE_DOT` replace the guide's obsolete `CLASS_DIAGRAMS` option.

After changing comments or the README, run the same command and refresh the HTML.
AI-generated descriptions must be checked against the implementation before
publication; this example documents the actual scoring and round lifecycle.
