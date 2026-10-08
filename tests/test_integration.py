"""End-to-end tests: compile .nat to actual native executables and run them."""
import http.server
import json
import os
from pathlib import Path
import subprocess
import tempfile
import threading
import unittest

ROOT = Path(__file__).resolve().parents[1]
NATC = Path(os.environ.get("NATC", ROOT / "build" / ("natc.exe" if os.name == "nt" else "natc")))


def call(*args, input_text=None):
    return subprocess.run([str(a) for a in args], input=input_text, text=True, capture_output=True, timeout=90)


class CompilationTests(unittest.TestCase):
    def native(self, source, stdin=""):
        with tempfile.TemporaryDirectory() as d:
            src = Path(d) / "program.nat"
            exe = Path(d) / ("program.exe" if os.name == "nt" else "program")
            src.write_text(source, encoding="utf-8")
            compilation = call(NATC, src, "-o", exe)
            self.assertEqual(compilation.returncode, 0, compilation.stdout + compilation.stderr)
            result = call(exe, input_text=stdin)
            return result

    def test_hello(self):
        r = self.native('Show "Hello"\nSet x to 5\nShow x + 2\n')
        self.assertEqual((r.returncode, r.stdout), (0, "Hello\n7\n"))

    def test_countdown(self):
        r = self.native((ROOT / "examples/countdown.nat").read_text())
        self.assertEqual((r.returncode, r.stdout), (0, "5\n4\n3\n2\n1\nDone!\n"))

    def test_functions(self):
        r = self.native((ROOT / "examples/functions.nat").read_text())
        self.assertEqual((r.returncode, r.stdout), (0, "5! = 120\n"))

    def test_lists(self):
        r = self.native((ROOT / "examples/lists.nat").read_text())
        self.assertEqual((r.returncode, r.stdout), (0, "[3, 7, 2]\nsum=12\nlength=3\n"))

    def test_statistics(self):
        r = self.native((ROOT / "examples/statistics.nat").read_text(), "3\n9\n5\n2\n6\n")
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertIn("Maximum: 9", r.stdout)
        self.assertIn("Average: 5", r.stdout)
        self.assertIn("not above 50", r.stdout)

    def test_input_repeat(self):
        r = self.native((ROOT / "examples/interactive.nat").read_text(), "2\n10\n")
        self.assertEqual((r.returncode, r.stdout), (0, "Not yet.\n5\n"))

    def test_compare_numbers_function(self):
        r = self.native((ROOT / "examples/compare_numbers.nat").read_text(), "7\n13\n")
        self.assertEqual((r.returncode, r.stdout), (0, "Largest: 13\n"))

    def test_boolean_math(self):
        r = self.native('Set x to 2 + 3 * 4\nIf x == 14 and not (x == 5)\nShow "ok"\nOtherwise\nShow "wrong"\nEnd\n')
        self.assertEqual((r.returncode, r.stdout), (0, "ok\n"))

    def test_repeat_and_break(self):
        r = self.native('Set x to 0\nRepeat 10 times\nIncrease x by 1\nIf x >= 3\nBreak\nEnd\nEnd\nShow x\n')
        self.assertEqual((r.returncode, r.stdout), (0, "3\n"))

    def test_errors(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/"bad.nat"
            for code, expected in [("Show unknowable", "unknown variable"),
                                   ("If true\nShow 1", "Missing END"),
                                   ("Break", "outside loop"),
                                   ("Show unknown(2)", "unknown function")]:
                p.write_text(code)
                r=call(NATC,p,"--check")
                self.assertNotEqual(r.returncode,0)
                self.assertIn(expected,r.stderr)

    def test_emit_source(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/"x.nat";out=Path(d)/"x.cpp"
            p.write_text('Show "abc"')
            r=call(NATC,p,"--emit-cpp",out)
            self.assertEqual(r.returncode,0,r.stderr)
            self.assertIn("int main()",out.read_text())
            self.assertIn("namespace nat",out.read_text())

    def test_files_write_and_read(self):
        with tempfile.TemporaryDirectory() as d:
            src=Path(d)/'files.nat'
            exe=Path(d)/('files.exe' if os.name=='nt' else 'files')
            src.write_text('Save "Hello file" to file "out.txt"\nLoad file "out.txt" into content\nShow content\n')
            r=call(NATC,src,'-o',exe)
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            outcome=subprocess.run([str(exe)],cwd=d,text=True,capture_output=True,timeout=10)
            self.assertEqual((outcome.returncode,outcome.stdout),(0,'Hello file\n'))
            self.assertEqual((Path(d)/'out.txt').read_text(),'Hello file')

    def test_uninitialized_variable_is_runtime_error(self):
        r=self.native('Show x\nSet x to 3\n')
        self.assertNotEqual(r.returncode,0)
        self.assertIn('Variable used before assignment: x',r.stderr)

    def test_genuine_native_artifact(self):
        with tempfile.TemporaryDirectory() as d:
            src=Path(d)/'x.nat'
            out=Path(d)/('native.exe' if os.name=='nt' else 'native')
            src.write_text('Show 42')
            r=call(NATC,src,'-o',out)
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            self.assertTrue(out.is_file())
            with open(out,'rb') as f: prefix=f.read(4)
            self.assertIn(prefix[:2],(b'MZ',b'\x7fE',b'\xcf\xfa',b'\xca\xfe'))

    def test_llm_adapter_mock(self):
        # Mock the llama-server protocol; tests adapter parsing, not model quality.
        class Handler(http.server.BaseHTTPRequestHandler):
            def do_POST(self):
                body=json.loads(self.rfile.read(int(self.headers.get('Content-Length',0))))
                self.server.testcase.assertEqual(body['response_format']['type'],'json_schema')
                content=json.dumps({'program': 'Set x to 7\nShow x + 3'})
                result=json.dumps({'choices':[{'message':{'role':'assistant','content':content}}]}).encode()
                self.send_response(200);self.send_header('Content-Type','application/json');self.send_header('Content-Length',str(len(result)));self.end_headers();self.wfile.write(result)
            def log_message(self,*args): pass
        server=http.server.HTTPServer(('127.0.0.1',0),Handler)
        server.testcase=self
        if server.server_port < 1000:
            server.server_close(); self.skipTest('Invalid test server port')
        t=threading.Thread(target=server.serve_forever,daemon=True)
        t.start()
        try:
            with tempfile.TemporaryDirectory() as d:
                src=Path(d)/'text.nat';exe=Path(d)/'app'
                if os.name=='nt':exe=exe.with_suffix('.exe')
                src.write_text('please compute something')
                r=call(NATC,src,'--llm','--llm-url',f'http://127.0.0.1:{server.server_port}/v1/chat/completions','-o',exe)
                self.assertEqual(r.returncode,0,r.stdout+r.stderr)
                output=call(exe)
                self.assertEqual((output.returncode,output.stdout),(0,'10\n'))
        finally:
            server.shutdown();server.server_close();t.join(2)

if __name__=='__main__': unittest.main()
