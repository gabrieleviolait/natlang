"""v0.3 end-to-end regression tests for the shared multilingual compiler."""
import json
from pathlib import Path
import tempfile
import unittest
from test_integration import ROOT, NATC, call


class MultilingualAndMathTests(unittest.TestCase):
    def native(self, text, stdin=''):
        with tempfile.TemporaryDirectory() as d:
            src=Path(d)/'test.nat'
            src.write_text(text,encoding='utf-8')
            out=Path(d)/('test.exe' if __import__('os').name=='nt' else 'test')
            compilation=call(NATC,src,'-o',out,'--opt-level','0')
            self.assertEqual(compilation.returncode,0,compilation.stdout+compilation.stderr)
            result=call(out,input_text=stdin)
            return result

    def test_math_arithmetic_functions_and_aliases(self):
        r=self.native((ROOT/'examples/multilingual_math.nat').read_text(encoding='utf-8'))
        self.assertEqual(r.returncode,0,r.stderr)
        self.assertEqual(r.stdout.splitlines(),[
            '4','4','4','20','3.5','1','32','512','-4','9','9','30','30','20',
            '120','3','7','3','0','4','12','2','6','3.14159265358979'])

    def test_italian_english_one_program(self):
        r=self.native((ROOT/'examples/bilingual_program.nat').read_text(encoding='utf-8'))
        self.assertEqual((r.returncode,r.stdout),(0,'Condizione vera\n5\n10\n'))

    def test_es_fr_de_basic_keywords_one_backend(self):
        r=self.native((ROOT/'examples/multilingual_keywords.nat').read_text(encoding='utf-8'))
        self.assertEqual((r.returncode,r.stdout),(0,'hola\nbonjour\nhallo\n'))

    def test_it_en_shared_ir_structure(self):
        with tempfile.TemporaryDirectory() as d:
            en=Path(d)/'english.nat';it=Path(d)/'italian.nat'
            en.write_text('Set x to 3\nIf x > 2\nShow x\nEnd\n')
            it.write_text('Imposta x a 3\nSe x è maggiore di 2\nMostra x\nFine\n',encoding='utf-8')
            a=call(NATC,en,'--emit-ir');b=call(NATC,it,'--emit-ir')
            self.assertEqual(a.returncode,0,a.stderr)
            self.assertEqual(b.returncode,0,b.stderr)
            lhs=json.loads(a.stdout);rhs=json.loads(b.stdout)
            self.assertEqual([n['kind'] for n in lhs],[n['kind'] for n in rhs])
            self.assertEqual([n['kind'] for n in lhs[1]['body']],[n['kind'] for n in rhs[1]['body']])
            self.assertEqual(lhs[0]['a'],rhs[0]['a'])

    def test_math_domains(self):
        for expression,error in [('sqrt(-1)','Square root of negative'),
                                 ('1 / 0','Division by zero'),
                                 ('log(0)','Logarithm requires'),
                                 ('fattoriale(-2)','Factorial requires')]:
            with self.subTest(expression=expression):
                r=self.native('Show '+expression)
                self.assertNotEqual(r.returncode,0)
                self.assertIn(error,r.stderr)

    def test_precedence(self):
        r=self.native('Show 2 ^ 3 ^ 2\nShow (-2)^2\nShow -2^2\nShow clamp(15,0,10)\nShow percent(12.5,200)\n')
        self.assertEqual((r.returncode,r.stdout),(0,'512\n4\n-4\n10\n25\n'))

    def test_expression_validation(self):
        with tempfile.TemporaryDirectory() as d:
            src=Path(d)/'bad.nat'
            for line in ('Show nowhere(5)','Show sqrt(1,2)','Show percent(10)','Show 3 ^'):
                src.write_text(line)
                result=call(NATC,src,'--check')
                self.assertNotEqual(result.returncode,0,line)

    def test_question_and_italian_boolean(self):
        r=self.native('Imposta x a 4\nQuanto fa 2 più 2?\nSe x è uguale a 4 e non falso\nMostra "vero"\nFine\n')
        self.assertEqual((r.returncode,r.stdout),(0,'4\nvero\n'))


if __name__=='__main__':
    unittest.main()
