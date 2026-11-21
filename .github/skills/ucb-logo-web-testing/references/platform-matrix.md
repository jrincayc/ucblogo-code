# Platform Matrix

The default iteration order, broader browser-test priority, and Safari policy live in [project instructions](../../../copilot-instructions.md#browser-test-priority); this file holds the rest of the matrix.

- **Expand validation before pushing, opening a PR, or reporting final results.** Run the relevant broader web tests in that priority order and the relevant Linux wx checks; debug platform differences as necessary. Checkpoint commits do not require this validation. Earlier targeted platform or sanitizer checks are appropriate when a specific failure or shared C/GC/build change makes them discriminating, not as a blanket per-edit requirement.
- The maintainer has Linux wx and an iPad for real mobile Safari testing, but no Mac or local Windows. Use the iPad for affected Safari, touch, keyboard, storage, and layout workflows. Desktop Safari remains supported but cannot be tested locally without a Mac; record this coverage gap rather than blocking ordinary iteration or claiming compatibility from another browser.

| Target | Broader Validation Goal |
| --- | --- |
| wx on Linux | Native build, relevant Logo regressions, GUI startup/input and affected behavior. |
| wx on macOS and Windows | Prefer native builds and relevant tests through CI; GUI checks where runner support permits. |
| Desktop web | Chrome first; Firefox after mobile Chrome/emulation checks. |
| Mobile web | Mobile Chrome or Chrome device emulation second; real iOS/iPadOS Safari last, using a human-in-the-loop session on the maintainer's iPad. Additional mobile Firefox coverage when relevant or requested. |
| Desktop Safari | Supported, but unavailable locally because there is no Mac. Use actual Safari through available macOS CI or an approved hosted service when feasible; otherwise report untested. |

- Aim to automate this matrix on GitHub Actions; use another CI or hosted browser/device service where necessary. This is requested coverage, not a claim that the jobs already exist. Prove runner, driver, display, and device capabilities before promising automation; get approval for paid services or infrastructure provisioning.
- Prefer existing harnesses and reusable focused tests. Separate native-platform and web-browser/build-profile jobs, keep relevant checks fast, and use scheduled or release coverage for expensive device matrices where appropriate. Include prod behavior/service-worker tests and compatible debug/ASAN checks without contaminating dev sessions.
- Distinguish native compilation from wx GUI execution, headless tests from event-loop coverage, Chromium from branded Chrome, and automated WebKit from shipping Safari. Responsive/device emulation is useful but does not establish real mobile keyboard, storage, or lifecycle behavior. iPad Safari does not establish desktop Safari or iPhone coverage.
- Record actual browser/engine, OS/device, profile, and versions. Do not infer mobile engine diversity from browser branding. Report required, optional, manual, skipped, and unavailable checks explicitly.
- Synchronize CI on readiness and observable results, propagate test failures through exit status, and retain compiler/sanitizer output, browser console errors, test reports, and useful screenshots as artifacts. Keep service credentials in CI secrets. When automation is unavailable, retain a reproducible manual workflow and make the coverage gap visible.
