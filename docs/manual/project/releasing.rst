Releasing Gammu
===============

1. Ensure that all tests pass on both Linux and Windows.
2. Update the version in :file:`CMakeLists.txt`, :file:`gammu.spec`, and
   :file:`description-pak`. Update :file:`ChangeLog` with the release date and
   changes since the previous release.
3. Update man pages using ``make update-man``.
4. Configure and verify the source package with
   ``cmake -S . -B _build -DCMAKE_BUILD_TYPE=Release`` and
   ``cmake --build _build --target package_source``.
5. (Optional) Test the created source packages.
6. Commit the release changes, create a tag matching the version, and push the
   commit and tag to GitHub. The release workflow builds the source packages
   and Windows installers and publishes the GitHub release from the tag.
7. Import the release to the website.
