---
description: "Release the Web Edition to GitHub Pages by pushing a pN tag, then verify the live site bytes."
argument-hint: "Tag to publish (for example p53); leave blank to propose the next one"
agent: "agent"
---
Release the Web Edition to GitHub Pages following [CI and deploy](../skills/ucb-logo-build-deploy/references/ci-and-deploy.md#releasing).

- Tag: ${input:tag:Tag to publish, or blank to propose the next pN}
- Confirm the branch is clean and CI or a local `npm test` passed. Report anything that is not.
- Propose the tag and wait for explicit confirmation before pushing it; pushing publishes the live site.
- After the push, watch the `deploy-web.yaml` run and report its result.
- Verify the live bytes as described in the reference and report which files and version were checked.
- Stop any temporary processes you started and remove temporary directories.
