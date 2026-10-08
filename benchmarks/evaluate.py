#!/usr/bin/env python3
"""Compare local GGUF or deterministic frontend to canonical NatLang AST targets.

Requires natc executable; real LLM mode requires running llama-server.
Nothing in this benchmark pretends to measure a GGUF if no model is active.
"""
import argparse
import collections
import json
import os
from pathlib import Path
import statistics
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]


def ast_signature(nodes):
    """Drop only diagnostic line numbers; use a strict structural oracle."""
    if isinstance(nodes, dict):
        return {k: ast_signature(v) for k, v in nodes.items() if k != "line"}
    if isinstance(nodes, list):
        return [ast_signature(x) for x in nodes]
    return nodes


def run_natc(natc, src, options, timeout, temp_dir):
    path = temp_dir / 'program.nat'
    out = temp_dir / 'program.ir.json'
    path.write_text(src, encoding='utf-8')
    started = time.perf_counter()
    try:
        p = subprocess.run([str(natc), str(path), '--emit-ir', str(out), *options],
                           capture_output=True, text=True, encoding='utf-8', timeout=timeout)
        elapsed = 1000 * (time.perf_counter() - started)
        if p.returncode != 0:
            return None, elapsed, p.stderr.strip()[-400:]
        if not out.exists():
            return None, elapsed, 'missing IR artifact'
        return ast_signature(json.loads(out.read_text(encoding='utf-8'))), elapsed, ''
    except (subprocess.TimeoutExpired, OSError, ValueError) as exc:
        return None, 1000 * (time.perf_counter()-started), str(exc)[-400:]
    finally:
        out.unlink(missing_ok=True)


def main():
    p=argparse.ArgumentParser(description='NatLang deterministic or LOCAL GGUF evaluation; no fabricated model accuracy')
    p.add_argument('--natc', default=str(ROOT/'build'/'natc'))
    p.add_argument('--dataset', default=str(ROOT/'benchmarks'/'cases.jsonl'))
    p.add_argument('--mode', choices=['deterministic','fallback','gguf'], default='deterministic')
    p.add_argument('--url', default='http://127.0.0.1:8080/v1/chat/completions')
    p.add_argument('--profile', choices=['qwen3','generic'], default='qwen3')
    p.add_argument('--max-cases', type=int, default=0)
    p.add_argument('--language', choices=['it','en','es','fr','de','all'], default='all')
    p.add_argument('--timeout', type=int, default=150)
    p.add_argument('--report', default='')
    p.add_argument('--model-label', default='not-specified')
    args=p.parse_args()
    natc=Path(args.natc).resolve()
    if not natc.is_file(): p.error('natc not found: '+str(natc))
    if args.max_cases < 0: p.error('--max-cases cannot be negative')
    options=[]
    if args.mode != 'deterministic':
        options=['--llm-all' if args.mode=='gguf' else '--llm','--llm-url',args.url,'--llm-profile',args.profile]
    cases=[json.loads(line) for line in Path(args.dataset).read_text(encoding='utf-8').splitlines() if line.strip()]
    if args.language!='all':cases=[c for c in cases if c['language']==args.language]
    if args.max_cases: cases=cases[:args.max_cases]
    results=[]
    with tempfile.TemporaryDirectory() as td:
        temp_dir=Path(td)
        for c in cases:
            reference=None
            if not c.get('reject'):
                reference,_,err=run_natc(natc,c['canonical'],[],args.timeout,temp_dir)
                if reference is None: raise RuntimeError(f'Invalid gold reference {c["id"]}: {err}')
            actual,ms,err=run_natc(natc,c['input'],options,args.timeout,temp_dir)
            recognized=actual is not None
            strict=actual==reference if not c.get('reject') else not recognized
            results.append({'id':c['id'],'language':c['language'],'category':c['category'],
                            'expected_rejection':bool(c.get('reject')),'accepted':recognized,
                            'strict_match':strict,'latency_ms':round(ms,2),'error':err})
            print(('PASS' if strict else 'FAIL')+f' {c["id"]:16} {ms:8.1f}ms'+(' '+err[:100] if err else ''),flush=True)
    n=len(results)
    by_language={}
    for lang in sorted({r['language'] for r in results}):
        selected=[r for r in results if r['language']==lang]
        by_language[lang]={'total':len(selected),'strict_matches':sum(r['strict_match'] for r in selected),
                           'accepted':sum(r['accepted'] for r in selected)}
    latencies=sorted(r['latency_ms'] for r in results)
    report={'mode':args.mode,'model_label':args.model_label if args.mode!='deterministic' else 'none',
            'profile':args.profile if args.mode!='deterministic' else 'none',
            'dataset':str(Path(args.dataset).resolve()),'cases':n,
            'strict_matches':sum(r['strict_match'] for r in results),
            'acceptances':sum(r['accepted'] for r in results),
            'mean_latency_ms':round(statistics.mean(latencies),2) if latencies else None,
            'median_latency_ms':round(statistics.median(latencies),2) if latencies else None,
            'p95_latency_ms':latencies[min(len(latencies)-1,int(0.95*(len(latencies)-1)))] if latencies else None,
            'by_language':by_language,'results':results,
            'notes':'Strict IR agreement, not full semantic equivalence. GGUF numbers only measure the given running server; synthetic/mock responses are not model evaluations.'}
    print(json.dumps({k:v for k,v in report.items() if k!='results'},ensure_ascii=False,indent=2))
    if args.report:
        Path(args.report).write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    return 0 if n else 1


if __name__=='__main__':sys.exit(main())
