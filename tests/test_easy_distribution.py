"""Dependency-free static checks for the Windows Easy distribution sources.

These tests do not substitute for compiling the Win32 GUI or running the
GitHub Actions packaging workflow on actual Windows.
"""
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class EasyDistribution(unittest.TestCase):
    def test_windows_native_gui_target(self):
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("if(WIN32)", cmake)
        self.assertIn("add_executable(natlang-studio WIN32", cmake)
        self.assertIn("studio/main.cpp", cmake)

    def test_studio_compiler_and_ai_controls(self):
        app = (ROOT / "studio/main.cpp").read_text(encoding="utf-8")
        for feature in ("--check", "clang++.exe", "--llm-preview", "--opt-level", "start_gguf.ps1", "TerminateProcess", "CreateProcessW", "CreatePipe", "WriteFile", "SetEnvironmentVariableW"):
            with self.subTest(feature=feature): self.assertIn(feature, app)

    def test_package_has_a_portable_compiler_and_model_choice(self):
        flow = (ROOT / ".github/workflows/easy-windows.yml").read_text(encoding="utf-8")
        for feature in ("llvm-mingw", "llama-server.exe", "natlang-studio.exe", "include_model", "NatLang-Studio-Easy-Setup.exe", "actions/upload-artifact@v4"):
            with self.subTest(feature=feature): self.assertIn(feature, flow)

    def test_release_build_and_portable_smoke(self):
        flow = (ROOT / ".github/workflows/easy-windows.yml").read_text(encoding="utf-8")
        for feature in ("Verify portable compilation WITHOUT Visual Studio", "Get-FileHash", "gh release upload", "gh release create", "--clobber", "if-no-files-found: error", "NATLANG_VERSION: '0.5.1'"):
            with self.subTest(feature=feature): self.assertIn(feature, flow)

    def test_installer_and_docs_exist(self):
        iss = (ROOT / "dist/NatLang.iss").read_text(encoding="utf-8")
        self.assertIn("PrivilegesRequired=lowest", iss)
        self.assertIn("recursesubdirs", iss)
        self.assertIn("natlang-studio.exe", iss)
        for name in ("dist/EASY_START.md", "docs/THIRD_PARTY.md", "LICENSE", "scripts/start_gguf.ps1"):
            self.assertTrue((ROOT / name).is_file(), name)

if __name__ == '__main__': unittest.main()
