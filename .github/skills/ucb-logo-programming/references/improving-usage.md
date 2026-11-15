# Improving Usage and Extensibility

Look for practical ways to improve how Logo is used and extended, not just changes to the language. When relevant, suggest reusable procedures, functional or declarative patterns, examples, documentation, adapters, or extension points that make common work clearer and new features easier to add. Ground suggestions in a concrete task, explain the tradeoffs, and distinguish existing capabilities from proposals. Do not force a language change where an idiom or library solves the problem.

Draw inspiration particularly from Lisp-family languages, reflecting Logo's Lisp heritage, and from Python's readability and practical design. JavaScript is also an influence, especially for browser integration, but is a secondary source of inspiration. Adapt ideas to UCB Logo's semantics and interactive character rather than copying another language's syntax or assuming its features exist.

Consult Lodash for ideas for higher-level functions; its functions are welcome candidates for Logo ports, especially when they make code more readable. Check existing Logo procedures first, then adapt names, inputs, templates, data types, and edge cases to the dialect; document meaningful differences and preserve compatibility.

For a proposed improvement, identify the user-visible friction and a concrete example that is awkward today. Propose the smallest coherent change and describe:

- the current workaround and who benefits;
- proposed syntax and semantics, including errors and edge cases;
- compatibility with existing Logo programs, APIs, and the wx/Web split;
- whether a usage pattern, library procedure, macro, JS adapter, extension point, or primitive is the right layer, including how easily future features can build on it;
- implementation and test impact, plus a small example or prototype.

Prefer additive, backward-compatible changes; breaking compatibility is unlikely to be approved. If a breaking change appears necessary, explicitly identify the affected behavior and migration cost, offer a compatible alternative, and seek approval before implementing it. Make common tasks easier and hard tasks possible without removing Logo's interactive, compositional character. Do not promise a feature before validating feasibility for the target runtimes.
