"""Offline adapter integration using a fake server, NOT a GGUF accuracy claim."""
import http.server
import json
import tempfile
import threading
from pathlib import Path
import unittest
from test_integration import NATC, call


class FakeLLM(http.server.BaseHTTPRequestHandler):
    def do_POST(self):
        req=json.loads(self.rfile.read(int(self.headers.get('Content-Length',0))))
        self.server.calls.append(req)
        program=self.server.replies[min(len(self.server.calls)-1, len(self.server.replies)-1)]
        content=json.dumps({'program':program},ensure_ascii=False)
        response=json.dumps({'id':'chatcmpl-test','choices':[{'index':0,'message':{'role':'assistant','content':content}}]}).encode('utf-8')
        self.send_response(200)
        self.send_header('Content-Type','application/json')
        self.send_header('Content-Length',str(len(response)))
        self.end_headers()
        self.wfile.write(response)
    def log_message(self,*args):pass


class ModelAdapterTests(unittest.TestCase):
    def start_server(self,replies):
        server=http.server.HTTPServer(('127.0.0.1',0),FakeLLM)
        server.replies=replies
        server.calls=[]
        worker=threading.Thread(target=server.serve_forever,daemon=True)
        worker.start()
        self.addCleanup(worker.join,2)
        self.addCleanup(server.server_close)
        self.addCleanup(server.shutdown)
        return server,f'http://127.0.0.1:{server.server_port}/v1/chat/completions'

    def test_preview_multilingual_and_structured_response(self):
        server,url=self.start_server(['Set numero to 8\nShow numero * 2'])
        with tempfile.TemporaryDirectory() as td:
            src=Path(td)/'request.nat'
            src.write_text('Dammi il doppio di otto',encoding='utf-8')
            r=call(NATC,src,'--llm-preview','--llm-profile','qwen3','--llm-url',url)
            self.assertEqual(r.returncode,0,r.stderr)
            self.assertIn('Set numero to 8',r.stdout)
            self.assertIn('"kind":"assign"',r.stdout)
            self.assertIn('"kind":"print"',r.stdout)
            self.assertEqual(len(server.calls),1)
            req=server.calls[0]
            self.assertEqual(req['response_format']['type'],'json_schema')
            self.assertIn('Examples of exact translations',req['messages'][0]['content'])
            self.assertEqual(req['model'],'local-model')
            self.assertFalse((Path(td)/'request.generated.cpp').exists())

    def test_failed_translation_is_retried_then_validated(self):
        server,url=self.start_server(['Show unknown_word','Show 3 + 4'])
        with tempfile.TemporaryDirectory() as td:
            src=Path(td)/'test.nat';src.write_text('Find three plus four')
            r=call(NATC,src,'--llm-all','--llm-url',url,'--llm-attempts','2','--check')
            self.assertEqual(r.returncode,0,r.stderr)
            self.assertEqual(len(server.calls),2)
            self.assertIn('unknown variable',server.calls[1]['messages'][1]['content'])

    def test_no_silent_accept_after_two_errors(self):
        server,url=self.start_server(['Show invalid_unknown','Show also_invalid_unknown'])
        with tempfile.TemporaryDirectory() as td:
            src=Path(td)/'test.nat';src.write_text('Make something impossible')
            r=call(NATC,src,'--llm-all','--llm-url',url,'--check')
            self.assertNotEqual(r.returncode,0)
            self.assertIn('rejected after 2 attempt',r.stderr)
            self.assertEqual(len(server.calls),2)

    def test_deterministic_mode_never_calls_model(self):
        server,url=self.start_server(['Show 999'])
        with tempfile.TemporaryDirectory() as td:
            src=Path(td)/'test.nat';src.write_text('Show 2 + 2')
            r=call(NATC,src,'--llm','--llm-url',url,'--emit-ir')
            self.assertEqual(r.returncode,0,r.stderr)
            self.assertIn('"kind":"print"',r.stdout)
            self.assertEqual(server.calls,[])

    def test_one_attempt_and_unsupported_are_errors(self):
        server,url=self.start_server(['Build a GUI with a database'])
        with tempfile.TemporaryDirectory() as td:
            src=Path(td)/'test.nat';src.write_text('Build a GUI with a database')
            r=call(NATC,src,'--llm-all','--llm-url',url,'--llm-attempts','1','--check')
            self.assertNotEqual(r.returncode,0)
            self.assertIn('rejected after 1 attempt',r.stderr)
            self.assertEqual(len(server.calls),1)

    def test_remote_endpoint_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            src=Path(td)/'test.nat';src.write_text('make a new variable somehow')
            r=call(NATC,src,'--llm-all','--llm-url','https://api.somewhere.com/v1/chat/completions','--check')
            self.assertNotEqual(r.returncode,0)
            self.assertIn('localhost',r.stderr)


if __name__=='__main__':unittest.main()
