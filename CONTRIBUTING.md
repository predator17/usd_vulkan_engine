# Contributing

Thank you for helping improve USD Vulkan Engine.

## Before changing code

- Read the **Current boundaries** section in `README.md`. The project is a rendering study, not a complete USD viewer.
- Open an issue before a large architectural change, especially scene loading, animation, materials or native Windows support.
- Keep the WSL2 Rocky Linux 10 setup working unless a change explicitly targets another environment.

## Build and verify

Follow `README.md` to download dependencies and build OpenUSD, then run:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
LD_LIBRARY_PATH=./OpenUSD/install/lib:$LD_LIBRARY_PATH ctest --test-dir build --output-on-failure
```

Run the application and check the spinning-cube demo when your change affects rendering, camera, UI or application behavior. Include the environment, compiler, GPU and Vulkan driver used for manual verification in the pull request description.

## Code expectations

- Use the existing `usd_vk` namespace and follow the current module boundaries.
- Keep Vulkan resource ownership and destruction explicit and correct.
- Do not hide new failure modes behind placeholder behavior. If a feature is partial, document what works and what remains.
- Add or update Catch2 tests for core data structures, USD integration and rendering helpers when applicable.
- Keep generated dependencies, build outputs, credentials and local machine configuration out of commits.

## Documentation expectations

Update the README and relevant files under `doc/` when behavior, build steps or project boundaries change. Do not describe a feature as complete while its parser, platform support or tests are still partial.

## License

By contributing, you agree that your contributions are licensed under the MIT License used by this repository.
