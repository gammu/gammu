Releasing Gammu
===============

1. Ensure that all tests pass on both Linux and Windows.
2. Update changelog and fill in release date in :file:`ChangeLog`.
3. Run ``./admin/make-release`` to verify release build works.
4. (optional) Test created tarballs.
5. Run ``./admin/make-release branch`` to make final release.
6. Push created tag to GitHub.
7. Wait for AppVeyor to produce Windows binaries.
8. Import release to the website.
