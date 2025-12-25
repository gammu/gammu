Releasing Gammu
===============

1. Ensure that all tests pass on both Linux and Windows.
2. Update changelog and fill in release date in :file:`ChangeLog`.
3. Update man pages using ``make update-man``.
4. Run ``./admin/make-release`` to verify release build works.
5. (optional) Test created tarballs.
6. Run ``./admin/make-release branch`` to make final release.
7. Push created tag to GitHub.
8. Wait for AppVeyor to produce Windows binaries.
9. Import release to the website.
