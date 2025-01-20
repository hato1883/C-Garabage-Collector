# Code Review Report

## How our code reviews worked

We as a team did not have a formal checklist or specific procedure for doing code reviews. However, we did make use of CI checks in GitHub to check formatting, that all tests passed, there were no memory leaks and that the code compiled and ran without crashes.

The checks were incredibly useful as we could easily recognize any issues or incorrect formatting and commit fixes even before any team member had to review the code. Then a team member could begin reviewing the code, checking for faulty logic, noting any improvements to readability, documentation or general code quality.

Since we did not have a clear guideline for how to do code reviews, there were some differences in how pull requests were reviewed by different coders. However, we feel that we for the most part used pull requests in the same way.

The colourful diffs on github worked well for seeing what had changed. Sometimes, though, it made reading code more difficult, when a function was split into many parts by removed lines that were actually unrelated to the function.

## The result of our code reviews

At the end of the project we had a total of around 80 Pull Requests. On average we would guess pull requests were open for 3 business days, however a few large ones were open for up to 1.5 week and some were closed much quicker. How long they were open also depended heavily on how high priority the issue it solved had.

Pull requests were often pushed back for changes, we would guess that at least half of all PR:s needed changes before they could be merged.

## Comparison with code reviews earlier in the course

The biggest difference between the code review we have done during the project and the code reviews we did earlier in the course, is that now code reviews were done continually during development. This was very different compared to the code reviews we did earlier, where we looked at the code after an assignment was finished.

Another difference was that we had a clear check list of what to look for earlier in the course. This is of course something we could have used during the project, but it was not something we thought much about.

## Thoughts on code reviews

We found that there were multiple positive consequences of doing code reviews. An example is that we were able to find bugs early on, before they could make it to the merged code where they are harder to find. It also allowed us to fix stylistic issues and make sure the merged code was of good quality.

Requiring code reviews did introduce some issues as well. Sometimes reviews delayed the project because needed features could not be merged instantly. Because PR's had to wait for reviews, another PR could be merged before the review finished, which caused the first PR to need a merge from main, which then prompted a new review and caused further delay.

For the most part, the code reviews served a valuable purpose, as we caught a lot of issues and bugs when reviewing pull requests. There was one scenario where we felt that code reviews were more unnecessary work. We had a separate branch for our demo code, where we edited an assignment to use our garbage collector. When this branch was merged to the main branch, there was around 1500 lines of code, as well as tests, added. It did not feel very valuable to look through it all, since we had also used pull requests for the branch with the demo code.

Though we have not had any major issues with our code reviews, there is one thing we might have changed if we started over today. We would want to use a code-review checklist to make it easier to know what kind of things to look for in the code and make it consistent how reviews were conducted.